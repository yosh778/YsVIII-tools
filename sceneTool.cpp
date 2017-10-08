// Scene tool

#include "sceneTool.hh"
#include "sceneOpCodes.hh"
#include "sceneOpCodeNames.hh"

#include <string>
#include <iostream>
#include <algorithm>

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


int main(int argc, char *argv[])
{
    if ( argc < 3 ) {
        std::cerr << "Usage : " << argv[0] << " <script>" << std::endl;
        return -1;
    }

    std::string iPath = argv[1];
    std::string oPath = argv[2];

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

inline bool isOpCode( uint16_t code )
{
	return (0x8000 <= code && code <= 0x8181);
}

void process_segment( std::ifstream& fh, SEGMENT_HEADER& segHead )
{
	uint32_t size = segHead.size;
	char *segment = (char*)malloc( size );

	fh.read( segment, size );


	char *pSeg = segment, *pEnd = segment + size;

	while ( pSeg < (pEnd-1) ) {

		OpCode opcode = (OpCode)*((uint16_t*)pSeg);
		// std::cerr << std::endl << "pSeg = " << std::hex << (int)pSeg << std::dec;
		pSeg += sizeof(uint16_t);
		// std::cerr << std::endl << "sizeof(uint16_t) = " << sizeof(uint16_t);
		// std::cerr << std::endl << "pSeg = " << std::hex << (int)pSeg << std::dec;
		// std::cerr << std::endl << "opcode = " << std::hex << (int)opcode << std::dec;

		if ( !isOpCode( opcode ) ) {
			std::cerr << std::endl << "ERROR : bad opcode 0x"
				<< std::hex << (int)opcode << std::dec;
			return;
		}

		std::cout << std::endl << opCodeNames[ opcode - OPCODE_exit ];

		if ( opcode == OPCODE_exit ) {
			std::cout << std::endl;
			return;
		}

		bool is_opcode = false;

		while ( !is_opcode && (pSeg < (pEnd-1)) ) {

			GENERIC_ARG arg = *((GENERIC_ARG*)pSeg);
			DataTag tag = (DataTag)arg.tag;
			is_opcode = isOpCode( tag );
			uint32_t tag_val;

			if ( is_opcode ) {
				// std::cerr << std::endl << "New OPCODE detected";
				break;
			}

			else {
				// std::cerr << "New ARG detected" << std::endl;
				// pSeg += sizeof(uint16_t);

				// tag_val = *((uint32_t*)pSeg);
				// pSeg += sizeof(uint32_t);

				pSeg += sizeof(GENERIC_ARG);

				std::cout << " ";
			}

			// std::cerr << "Switch ARG" << std::endl;

			switch ( tag ) {

			case INT_TAG:
				std::cout << ", int " << arg.iVal;
				break;

			case FLOAT_TAG:
				std::cout << ", float " << arg.fVal;
				break;

			case STRING_TAG: {
				std::cout << ", string ";

				uint32_t len = arg.uVal;
				char data[len+1];

				strncpy(data, pSeg, len);
				data[len] = 0;
				pSeg += len;

				// TODO : find unambiguous string delimiters
				std::cout << data; }
				break;

			case UNK0_TAG:
			default: {
				std::cout << ", unk0 ";

				uint32_t count = arg.uVal;
				char data[count+1];

				strncpy(data, pSeg, count);
				data[count] = 0;
				pSeg += count;

				// TODO : find unambiguous unk0 delimiters
				std::cout << data; }
				break;
			}

		};

	}
}
