// Scene tool

#include "sceneTool.hh"
#include "sceneOpCodes.hh"
#include "sceneOpCodeNames.hh"

#include <string>
#include <iostream>
#include <algorithm>
#include <iomanip>

#include <sstream>
#include <fstream>
#include <cstring>
#include <list>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp> 
#include <boost/iostreams/code_converter.hpp>

#include <boost/locale.hpp>

using namespace boost::locale;
using namespace boost::locale::conv;


void write16(std::fstream& output, uint16_t data) {
	output.write((char*)&data, (int)sizeof(data));
}

void write32(std::fstream& output, uint32_t data) {
	output.write((char*)&data, (int)sizeof(data));
}

void write64(std::fstream& output, uint64_t data) {
	output.write((char*)&data, (int)sizeof(data));
}

uint16_t read16(std::ifstream& input) {
	uint16_t data;
	input.read((char*)&data, (int)sizeof(data));
	return data;
}

uint32_t read32(std::ifstream& input) {
	uint32_t data;
	input.read((char*)&data, (int)sizeof(data));
	return data;
}

uint64_t read64(std::ifstream& input) {
	uint64_t data;
	input.read((char*)&data, (int)sizeof(data));
	return data;
}

// Checksum reverse by weaknespase
uint32_t checksum(const char* in, const uint32_t length, int last = 0){
	const char* end = in + length;
	int acc = last;
	while (in < end)
		acc = (acc * 33) ^ (unsigned char) *in++;
	return acc;
}

bool oShiftJis = false;
bool iShiftJis = false;
bool keepStringSizes = false;

// Apply encoding fixes here
void convertText(std::string& text)
{
	char last = 0;
	char elem = 0;
	bool quoteStarts = true;
	std::string data;

	// Convert from Shift-JIS to UTF-8 if needed
	if ( iShiftJis ) {
		data = to_utf<char>( text, "Shift-JIS" );
		goto end;
	}

	// Cancel UTF-8 to Shift-JIS conversion if unneeded
	if ( !oShiftJis ) {
		return;
	}

	data = from_utf( text, "Shift-JIS" );

	for ( uint32_t i = 0; i < data.size(); i++ ) {

		last = elem;
		elem = data[i];

		// Fix english \" for japanese game
		if ( last == '\\' && elem == '"' ) {

			// data[i-1] = 0x81;

			// if ( quoteStarts ) {
			// 	data[i] = 0x77;
			// }
			// else {
			// 	data[i] = 0x78;
			// }
			data = data.substr(0, i-1) + "\"" + data.substr(i+1);
			quoteStarts = !quoteStarts;
		}

	}

	end:

	if ( !keepStringSizes ) {
		text = data;
		return;
	}

	int oSize = data.size();
	int iSize = text.size();
	int diff = oSize - iSize;

	if ( diff < 0 ) {
		diff = -diff;

		bool soh = false;

		if ( data.size() > 0 && data[data.size()-1] == '\x01' )
			data = data.substr(0, data.size()-1), soh = true;

		for ( int i = 0; i < diff; i++ )
			data += " ";

		if ( soh )
			data += "\x01";
	}
	else if ( diff > 0 ) {

		bool soh = false;

		if ( data.size() > 0 && data[data.size()-1] == '\x01' )
			data = data.substr(0, data.size()-1), soh = true;

		data = data.substr(0, data.size()-diff);

		if ( soh )
			data += "\x01";
	}

	text = data.substr(0, iSize);

	if ( text.size() != iSize ) {
		std::cerr << "ERROR : a text fitting error occurred" << std::endl;
		exit(1);
	}
}



#define write_elem(fh, elem) fh.write((char*)&elem, (int)sizeof(elem))


void write_segment( std::fstream& fh, SEGMENT_HEADER& segHead, std::list<std::string>::iterator& it, std::list<std::string>::iterator& end );

void print_hex(uint8_t *data, uint32_t size)
{
	for (uint32_t i = 0; i < size; ++i) {
		std::cout << " " << std::hex << std::setfill('0') << std::setw(2)
			<< (int)data[i] << std::dec;
	}
}

uint32_t countSegments(std::list<std::string>& lines);

std::map<std::string,uint16_t> opCodeMap;
std::vector<std::pair<uint32_t, std::string>> jmpList;
std::map<std::string, uint32_t> labelMap;

int main(int argc, char *argv[])
{
	std::string iPath;
	std::string oPath;

	for ( int i = 1; i < argc; i++ ) {

		std::string arg = argv[i];

		if ( arg == "--enc-shift-jis" ) {
			std::cout << "Reencoding strings as Shift-JIS" << std::endl;
			oShiftJis = true;
		}

		else if ( arg == "--dec-shift-jis" ) {
			std::cout << "Decoding strings as Shift-JIS" << std::endl;
			iShiftJis = true;
		}

		else if ( arg == "--preserve-string-sizes" ) {
			std::cout << "Preserving string sizes" << std::endl;
		// else if ( arg == "--change-string-sizes" ) {
		// 	std::cout << "Allowing string size modification" << std::endl;
		// 	keepStringSizes = false;
			keepStringSizes = true;
		}

		else if ( iPath.size() < 1 ) {
			iPath = arg;
		}

		else if ( oPath.size() < 1 ) {
			oPath = arg;
		}
	}

	if ( iShiftJis && oShiftJis ) {
		iShiftJis = false;
		oShiftJis = false;
	}

	if ( argc < 3 || iPath.size() < 1 || oPath.size() < 1 ) {
		std::cerr << "Usage : " << argv[0]
			<< " <script> <output> (--dec-shift-jis) (--enc-shift-jis) (--change-string-sizes)" << std::endl;
		return -1;
	}


	for ( uint32_t i = 0; i < OPCODE_COUNT; i++ ) {
		opCodeMap[ opCodeNames[i] ] = 0x8000 + i;
	}

	std::ifstream iFile( iPath.c_str(), std::ios_base::binary );

	if ( !iFile.is_open() ) {
		std::cerr << iPath << " not found" << std::endl;
		return -2;
	}

	// Create output directory structure
    boost::system::error_code returnedError;
    boost::filesystem::path path( oPath );
    //std::cout << path.parent_path() << std::endl;

    boost::filesystem::create_directories(
        path.parent_path(), returnedError
    );

	std::fstream oFile( oPath.c_str(), std::ios_base::trunc | std::ios_base::in | std::ios_base::out | std::ios_base::binary );

	if ( !oFile.is_open() ) {
		std::cerr << "Failed to open " << oPath << std::endl;
		return -2;
	}


	FILE_HEADER_V2 headerV2;
	memset(&headerV2, 0, sizeof(headerV2));

	std::list<std::string> lines(0);

	for( std::string line; getline( iFile, line ); )
	{
		lines.push_back(line);
	}

	// TODO : count segments

	iFile.close();

	strcpy( headerV2.magic, "YS7_SCP" );

	headerV2.version = 4;
	headerV2.checksum = 0x69DB279B411DC943; // Fake checksum
	headerV2.segs_count = countSegments(lines);

	write_elem( oFile, headerV2 );

	SEGMENT_HEADER segHeads[ headerV2.segs_count ];
	memset(&segHeads, 0, sizeof(segHeads));

	write_elem( oFile, segHeads );

	std::list<std::string>::iterator it = lines.begin();
	std::list<std::string>::iterator end = lines.end();

	for ( int i = 0; i < headerV2.segs_count; i++ ) {
		write_segment( oFile, segHeads[i], it, end );
	}

	oFile.seekp( 0 );
	write_elem( oFile, headerV2 );

	oFile.seekp( sizeof(headerV2) );
	write_elem( oFile, segHeads );

	oFile.close();

	return 0;
}

inline bool isJapOpCode( uint16_t code )
{
	return (0x8000 <= code && code <= 0x8181);
}

inline bool isOpCode( uint16_t code )
{
	return (0x8000 <= code && code <= 0x8182);
}

std::string nextLine(std::list<std::string>::iterator& it, std::list<std::string>::iterator end)
{
	std::string line;

	for (; it != end; ++it) {

		line = *it;
		boost::trim(line);

		if ( line.size() > 0 )
			return line;
	}
}

#define SEGMENT_DELIMITER ".segment"
#define CODE_DELIMITER ".code"

void goToCode(std::list<std::string>::iterator& it, std::list<std::string>::iterator end)
{
	std::string line;

	for (; it != end; ++it) {

		line = *it;
		boost::trim(line);

		if ( line == CODE_DELIMITER )
			return;
	}
}

void nextEmptyLine(std::list<std::string>::iterator& it, std::list<std::string>::iterator end)
{
	std::string line;

	for (; it != end; ++it) {

		line = *it;
		boost::trim(line);

		if ( line.size() == 0 )
			return;
	}
}

uint32_t countSegments(std::list<std::string>& lines)
{
	std::string line;
	uint32_t count = 0;
	std::list<std::string>::iterator it, end = lines.end();

	for ( it = lines.begin(); it != end; ) {

		// Get next segment
		line = nextLine(it, end);

		// Go to segment code
		nextEmptyLine(it, end);

		if ( it != end )
			count++, it++;

		// Go to first code line
		nextLine(it, end);

		// Go to last code line
		nextEmptyLine(it, end);
	}

	return count;
}

bool isInteger(std::string line)
{
	return line.substr(0, 2) == "0x";
}

bool parseNextArg( std::string& arg, std::string& args )
{
	uint32_t beg = args.size();
	uint32_t size = 0;

	// Look for arg beginning
	for ( uint32_t i = 0; i < args.size(); i++ ) {

		char elem = args[i];

		if ( elem != ' ' && elem != ',' ) {
			beg = i;
			break;
		}
	}

	if ( beg >= args.size() )
		return false;

	bool inString = false;
	char beforeLast = 0;
	char last = 0;
	char elem = args[beg];

	// Look for arg end, ignore string contents
	for ( uint32_t i = beg; i < args.size(); i++ ) {

		elem = args[i];

		if ( elem == '"' ) {

			if ( !inString ) {

				if ( last == ' ' || last == ',' || !last
					 || last == 's' || last == 'p' ) {

					inString = true;
				}
			}
			else if ( last != '\\' ) {

				for ( uint32_t j = i+1; j < args.size(); j++ ) {

					char next = args[j];

					if ( next == ',' ) {
						inString = false;
						break;
					}

					else if ( next != ' ' ) {
						break;
					}
				}

			}
		}

		if ( !inString && elem == ',' ) {
			break;
		}

		size++;

		beforeLast = last;
		last = elem;
	}

	// if ( inString ) {
	// 	return false;
	// }

	bool ret = beg < args.size() && size > 0;

	arg = args.substr( beg, size );
	args = args.substr( beg + size );

	return ret;
}

bool parsePopup( std::vector<uint32_t>& args, std::string& data )
{
	uint32_t start = 0;

	for ( uint32_t i = 0; i < data.size(); i++ ) {
		char elem = data[i];

		if ( elem == '\x01' ) {
			args.push_back( start );
			start = i+1;
		}
	}

	return true;
}

bool parseHex( std::vector<uint8_t>& values, std::string data )
{
	char buf[3];
	// int hex = 0;
	int iBuf = 0;

	for (uint32_t i = 0; i < data.size(); ++i) {

		char elem = data[i];

		if ( elem != ' ' ) {

			if ( (elem < '0' && elem > '9')
				|| (elem < 'a' && elem > 'f')
				|| (elem < 'A' && elem > 'F') ) {
				return false;
			}

			buf[iBuf++] = elem;

			if ( iBuf >= 2 ) {
				buf[3] = 0;

				try {
					int value = std::stoul(buf, nullptr, 16);
					values.push_back( value );
				}
				catch (...) {
					std::cerr << "ERROR : failed to parse '" << data << "' as hex" << std::endl;
				}
				// std::cerr << "Added " << value << std::endl;

				iBuf = 0;
			}

			// hex *= 0x10;
			// hex |= std::stoul(s, nullptr, 16);
		}
		// std::cout << " " << elem << std::dec;
	}

	return true;
}

bool hasJump(uint32_t opcode)
{
	bool hasJump = false;

	switch (opcode) {
	case OPCODE_if:
	case OPCODE_elseIf:
	case OPCODE_else:
	case OPCODE_break:
	case OPCODE_while:
	case OPCODE_case:
	case OPCODE_default:
	case OPCODE_Execute:
	case OPCODE_8085:
		hasJump = true;
		break;

	default:
		NULL;
	};

	return hasJump;
}

uint32_t lastJump = 0;
std::string labelName;

void write_arg( std::fstream& fh, std::string arg, uint16_t opcode )
{
	boost::trim(arg);

	if ( arg.size() < 1 )
		return;

	char first = arg[0];
	std::string content = arg;
		// std::cout << "Detected arg " << content << std::endl;

	switch ( first ) {

	case 's': {
		// std::cout << "Detected s'string' arg " << content << std::endl;
		uint32_t beg = 1;

		for ( uint32_t i = beg; i < content.size(); i++ ) {
			
			char elem = content[i];

			if ( elem == '"' ) {
				beg = i;
				break;
			}
		}

		content = content.substr( beg );
	}

	case '"': {
		// std::cout << "Detected 'string' arg " << content << std::endl;
		uint32_t end = 1;
		uint32_t i = content.size()-1;

		while ( i > 0 ) {
			
			char elem = content[i];

			if ( elem == '"' ) {
				end = i;
				break;
			}

			i--;
		}

		content = content.substr( 1, end-1 );

		// if (   opcode == OPCODE_MenuAdd
		// 	|| opcode == OPCODE_YesNoMenu
		// 	|| opcode == OPCODE_GetItemMessageExPlus
		// 	|| opcode == OPCODE_Message
		// 	// TalkMes
		// 	 ) {

			convertText(content);
		// }

		// std::cout << "Writing string " << content << std::endl;

		write16( fh, STRING_TAG );
		write32( fh, content.size() );
		fh.write( content.c_str(), content.size() );
		break;
	}

	case '#': {
		int32_t value = std::stoi( content.substr( 1 ) );

		write16( fh, INT_TAG );
		write32( fh, value );

		if ( hasJump( opcode ) ) {

			int labelPos = content.find("/");

			if ( labelPos != std::string::npos ) {
				labelName = content.substr(labelPos+1);
				boost::trim(labelName);
				lastJump = value;
			}
		}

		break;
	}

	case '.': {
		// float value = std::stof( content.substr( 1 ) );
		uint32_t value = std::stoul( content.substr(1), nullptr, 16 );

		write16( fh, FLOAT_TAG );
		write32( fh, *(uint32_t*)&value ); // float writing trick
		break;
	}

	case 'o': {
		std::vector<uint8_t> values(0);
		parseHex( values, content.substr(1) );

		write16( fh, UNK0_TAG );
		write32( fh, values.size() );
		fh.write( (char*)values.data(), values.size() );
		break;
	}

	case 'p': {
		std::string text;
		std::vector<uint32_t> args(0);

		content = content.substr(1);

		size_t strPos = content.find("\"");

		if ( strPos == std::string::npos ) {
			std::cerr << "ERROR : invalid popup argument (no string found)" << std::endl;
			exit(1);
		}

		content = content.substr( strPos+1 );

		size_t strEnd = content.rfind('\"');

		if ( strEnd == std::string::npos ) {
			std::cerr << "ERROR : failed to parse popup '" << content << "' of size : " << content.size() << std::endl;
			exit(2);
		}

		text = content.substr( 0, strEnd );

		convertText(text);

		// parsePopup( args, text, content.substr(5) );
		parsePopup( args, text );

		write16( fh, POPUP_TAG );
		write32( fh, args.size() );
		write32( fh, text.size() );

		// for ( uint32_t i = 0; i < args.size(); i++ ) {
		// 	write32( fh, args[i] );
		// }

		fh.write( (char*)args.data(), args.size() * sizeof(uint32_t) );
		fh.write( text.c_str(), text.size() );
		break;
	}

	default:
		std::cerr << "ERROR : unrecognized parameter '" << content << "'" << std::endl;
	}
}

void write_segment(
		std::fstream& fh, SEGMENT_HEADER& segHead,
		std::list<std::string>::iterator& it,
		std::list<std::string>::iterator& end
	)
{
	segHead.offset = fh.tellp();

	std::string line = nextLine(it, end);

	if ( isInteger(line) ) {
		std::cout << "Offset : " << line << std::endl;
		line = nextLine(++it, end);
	}

	strncpy( segHead.name, line.c_str(), line.size() );
	std::cout << "Segment name : " << line << std::endl;

	nextEmptyLine(it, end);

	bool done = false;
	uint16_t opcode = 0;

	while ( ++it != end ) {

		line = *it;

		std::string codeLine = line;
		std::size_t first;
		std::size_t found;

		boost::trim(line);

		if ( line.size() > 1 && line[0] == '#' ) {
			continue;
		}

		do {
			first = codeLine.find(",");
			found = codeLine.find(":");

			if ( found != std::string::npos && found < first ) {
				std::string label = codeLine.substr(0, found);
				boost::trim(label);

				if ( label.substr(0, 2) != "0x" ) {
					labelMap[ label ] = fh.tellp();
				}

				codeLine = codeLine.substr(found + 1);
			}
			else {
				break;
			}

		} while ( true );

		boost::trim(codeLine);

		if ( codeLine.size() < 1 ) {
			++it;
			break;
		}

		std::vector<std::string> elems;
		boost::split(elems, codeLine, boost::is_any_of(","));

		std::string& opCodeName = elems[0];
		opcode = opCodeMap[ opCodeName ];
		// std::cout << "OPCODE : " << opCodeName << std::endl;

		write16( fh, opcode );

		std::string arg;
		std::string args = codeLine.substr( opCodeName.size() );

		while ( parseNextArg( arg, args ) ) {

			// std::cout << "Next arg : " << arg << std::endl;
			write_arg( fh, arg, opcode );
		}

		if ( hasJump(opcode) ) {
			jmpList.push_back( std::make_pair( (int)fh.tellp(), labelName ) );
		}
	}

	segHead.size = (int)fh.tellp() - (int)segHead.offset;
	const uint32_t pos = fh.tellp();

	for ( auto &jmp : jmpList ) {

		fh.seekp( jmp.first - 4 );
		write32( fh, labelMap[ jmp.second ] - jmp.first );
		// std::cerr << "setting jmp @offset 0x" << std::hex << (jmp.first-4 - segHead.offset)
		// 	<< " to @0x" << (int)(labelMap[ jmp.second ] - segHead.offset) << std::dec
		// 	<< std::endl;
			// << " ie #" << (int)(labelMap[ jmp.second ] - segHead.offset) << std::endl;
	}

	fh.seekp(pos);

	jmpList.clear();
	labelMap.clear();
}

