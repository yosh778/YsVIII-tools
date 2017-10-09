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

int main(int argc, char *argv[])
{
	if ( argc < 3 ) {
		std::cerr << "Usage : " << argv[0] << " <script> <output>" << std::endl;
		return -1;
	}

	for ( uint32_t i = 0; i < OPCODE_COUNT; i++ ) {
		opCodeMap[ opCodeNames[i] ] = 0x8000 + i;
	}

	std::string iPath = argv[1];
	std::string oPath = argv[2];

	std::ifstream iFile( iPath.c_str(), std::ios_base::binary );

	if ( !iFile.is_open() ) {
		std::cerr << iPath << " not found" << std::endl;
		return -2;
	}

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
	uint32_t size = 1;

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
	char beforeLast;
	char last;
	char elem = args[beg];

	// Look for arg end, ignore string contents
	for ( uint32_t i = beg+1; i < args.size(); i++ ) {

		beforeLast = last;
		last = elem;
		elem = args[i];

		if ( elem == '"' ) {

			if ( !inString ) {

				if ( last == ' ' || last == ',' || last == 's' ) {
					inString = true;
				}
			}
			else if ( last != '\\' && (last != 0x77 || beforeLast != 0x81) ) {

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
	}

	// if ( inString ) {
	// 	return false;
	// }

	bool ret = beg < args.size() && size > 0;

	arg = args.substr( beg, size );
	args = args.substr( beg + size );

	return ret;
}

bool parsePopup( std::vector<uint32_t>& args, std::string& text, std::string data )
{
	int posOpenBracket = data.find("(");
	int posColon = data.find(":");

	if ( posOpenBracket == std::string::npos )
		return false;

	int argPos = (posColon == std::string::npos) ? posColon : posColon;
	argPos++;

	int strPos = data.find("\"");

	if ( strPos == std::string::npos )
		return false;

	std::vector<std::string> elems;
	std::string content = data.substr( argPos, strPos-1-argPos );
	boost::split( elems, content, boost::is_any_of(";") );

	for ( uint32_t i = 0; i < elems.size(); i++ ) {
		
		std::string& elem = elems[i];
		boost::trim(elem);

		if ( elem.size() < 1 ) {
			continue;
		}

		// std::cout << "Converting to int : " << elem << std::endl;
		int value = std::stoi(elem);
		// std::cout << "Result : " << value << std::endl;
		args.push_back( value );
	}

	text = data.substr( strPos+1 );

	int strEnd = text.rfind("\"");

	if ( strEnd == std::string::npos || strEnd == strPos )
		return false;

	text = text.substr( 0, strEnd );

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
				int value = std::stoul(buf, nullptr, 16);
				values.push_back( value );

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

void write_arg( std::fstream& fh, std::string arg )
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
		parsePopup( args, text, content.substr(5) );

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

	while ( ++it != end ) {

		line = *it;

		std::string codeLine = line;
		std::size_t found = line.find(":");

		if ( found != std::string::npos )
			codeLine = line.substr(found + 1);

		boost::trim(codeLine);

		if ( codeLine.size() < 1 ) {
			break;
		}

		std::vector<std::string> elems;
		boost::split(elems, codeLine, boost::is_any_of(","));

		std::string& opCodeName = elems[0];
		uint16_t opcode = opCodeMap[ opCodeName ];
		// std::cout << "OPCODE : " << opCodeName << std::endl;

		write16( fh, opcode );

		std::string arg;
		std::string args = codeLine.substr( opCodeName.size() );

		while ( parseNextArg( arg, args ) ) {

			// std::cout << "Next arg : " << arg << std::endl;
			write_arg( fh, arg );
		}

	}

	segHead.size = (int)fh.tellp() - (int)segHead.offset;
}

