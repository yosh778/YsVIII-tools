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

#define read_elem(fh, elem) fh.read((char*)&elem, (int)sizeof(elem))


void process_segment( std::ifstream& fh, SEGMENT_HEADER& segHead );

void print_hex(uint8_t *data, uint32_t size)
{
    for (uint32_t i = 0; i < size; ++i) {
        std::cout << " " << std::hex << std::setfill('0') << std::setw(2)
    		<< (int)data[i] << std::dec;
    }
}


int main(int argc, char *argv[])
{
    if ( argc < 2 ) {
        std::cerr << "Usage : " << argv[0] << " <script>" << std::endl;
        return -1;
    }

    std::string iPath = argv[1];

    std::ifstream iFile( iPath.c_str(), std::ios_base::binary );

    if ( !iFile.is_open() )
        return -2;


    FILE_HEADER_V2 headerV2;
    read_elem( iFile, headerV2 );

    SEGMENT_HEADER segHeads[ headerV2.segs_count ];
    read_elem( iFile, segHeads );

    for ( int i = 0; i < headerV2.segs_count; i++ ) {
        process_segment( iFile, segHeads[i] );
    }

    iFile.close();

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

void process_segment( std::ifstream& fh, SEGMENT_HEADER& segHead )
{
	fh.seekg( segHead.offset );

	uint32_t size = segHead.size;
	char *segment = (char*)malloc( size );

	fh.read( segment, size );

	std::cout << std::endl;
	std::cout << "0x" << std::hex << (int)segHead.offset << std::dec << std::endl;
	std::cout << segHead.name << std::endl;
	std::cout << "Size : 0x" << std::hex << (int)segHead.size << std::dec << std::endl;

	char *pSeg = segment, *pEnd = segment + size;

	while ( pSeg < (pEnd-1) ) {

		OpCode opcode = (OpCode)*((uint16_t*)pSeg);

		if ( !isOpCode( opcode ) ) {
			std::cerr << std::endl << "ERROR : bad opcode 0x"
				<< std::hex << (int)opcode
				<< " at offset 0x" << (pSeg - segment + segHead.offset)
				<< std::dec;
			// return;
		}

		else if ( !isJapOpCode( opcode ) ) {
			std::cerr << std::endl << "WARNING : unsupported english opcode 0x"
				<< std::hex << (int)opcode
				<< " at offset 0x" << (pSeg - segment + segHead.offset)
				<< std::dec;
			// return;
		}

		std::cout << std::endl << "0x" << std::hex << std::setfill('0') << std::setw(4)
			<< (int)(pSeg - segment) << ": "
			<< std::dec;

		uint32_t opcode_idx = opcode - OPCODE_exit;
		std::string opcode_name;

		if ( opcode_idx < sizeof(opCodeNames)/sizeof(char*) ) {
			opcode_name = opCodeNames[ opcode_idx ];
			std::cout << opcode_name;
		}
		else {
			std::cout << std::hex << "0x" << opcode << std::dec;
		}

		// std::cerr << std::endl << "pSeg = " << std::hex << (int)pSeg << std::dec;
		pSeg += sizeof(uint16_t);
		// std::cerr << std::endl << "sizeof(uint16_t) = " << sizeof(uint16_t);
		// std::cerr << std::endl << "pSeg = " << std::hex << (int)pSeg << std::dec;
		// std::cerr << std::endl << "opcode = " << std::hex << (int)opcode << std::dec;

		// if ( opcode == OPCODE_exit ) {
		// 	std::cout << std::endl;
		// 	return;
		// }

		bool is_opcode = false;
		uint16_t nargs = 0;

		while ( !is_opcode && (pSeg < (pEnd-1)) ) {

			const char *pArg = pSeg;
			GENERIC_ARG arg = *((GENERIC_ARG*)pSeg);
			DataTag tag = (DataTag)arg.tag;
			is_opcode = isOpCode( tag );
			uint32_t tag_val;

			if ( is_opcode || ++nargs >= 16 ) {
				// std::cerr << std::endl << "New OPCODE detected";
				break;
			}

			else {
				// std::cerr << "New ARG detected" << std::endl;
				// pSeg += sizeof(uint16_t);

				// tag_val = *((uint32_t*)pSeg);
				// pSeg += sizeof(uint32_t);

				pSeg += sizeof(GENERIC_ARG);

				// std::cout << " ";
			}

			// std::cerr << "Switch ARG" << std::endl;

			switch ( tag ) {

			case INT_TAG:
				std::cout << ", #" << arg.iVal;
				break;

			case FLOAT_TAG:
				std::cout << ", f" << arg.fVal;
				break;

			case STRING_TAG: {
				std::cout << ", s\"";
				// std::cout << ", "<<arg.uVal<<"\"";

				uint32_t len = arg.uVal;
				char data[len+1];

				strncpy(data, pSeg, len);
				data[len] = 0;
				pSeg += len;

				// TODO : find unambiguous string delimiters
				std::cout << data << "\""; }
				break;

			case POPUP_TAG: {
				std::cout << ", popup (" << arg.uVal << ":";

				POPUP_ARG popup = *(POPUP_ARG*)pArg;
				uint32_t nLines = popup.nLines;
				uint32_t len = popup.len;
				pSeg += sizeof(uint32_t);

				uint32_t args[ nLines ];

				for (uint32_t i = 0; i < nLines; i++) {
					args[i] = *((uint32_t*)pSeg);
					pSeg += sizeof(uint32_t);

					std::cout << args[i] << ";";
				}

				std::cout << " \"";

				char data[len+1];

				strncpy(data, pSeg, len);
				data[len] = 0;
				pSeg += len;

				// TODO : find unambiguous string delimiters
				std::cout << data << "\")"; }
				break;

			case UNK0_TAG: {
				std::cout << ", o";

				uint32_t count = arg.uVal;

				// if ( count != 8 ) {
				// 	std::cerr << std::endl << "WARNING : 0x82E0 ARG size = " << count;
				// }

				if ( count > (pEnd - pSeg) ) {
					std::cerr << std::endl << "FATAL ERROR, segment aborted";
					std::cout << std::endl << "FATAL ERROR, segment aborted";
					return;
				}

				print_hex((uint8_t*)pSeg, count);

				pSeg += count; }
				break;


			default:
				std::cerr << std::endl
					<< "ERROR : Unknown argument 0x" << std::hex << (int)tag
					<< " at offset 0x" << (pArg - segment + segHead.offset)
					<< std::dec << ", stopping argument parsing";

				is_opcode = true;
				pSeg = (char*)pArg;
				break;

				// std::cout << ", " << std::hex << "0x" << (int)tag << " : 0x" << arg.uVal << std::dec;
				// break;
			}
		}
	}

	std::cout << std::endl;
}
