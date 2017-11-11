// XAST Patcher by yosh778

#include <string>
#include <iostream>
#include <algorithm>

#include <sstream>
#include <fstream>
#include <cinttypes>
#define RBUF_SIZE 0x1000000

uint32_t read32(std::fstream& input) {
	uint32_t data;
	input.read((char*)&data, (int)sizeof(data));
	return data;
}

uint64_t read64(std::fstream& input) {
	uint64_t data;
	input.read((char*)&data, (int)sizeof(data));
	return data;
}

void write32(std::fstream& output, uint32_t data) {
	output.write((char*)&data, (int)sizeof(data));
}

// Checksum reverse by weaknespase
uint32_t checksum(const char* in, const uint32_t length, int last = 0){
	const char* end = in + length;
	int acc = last;
	while (in < end)
		acc = (acc * 33) ^ (unsigned char) *in++;
	return acc;
}

int main(int argc, char *argv[])
{
	if ( argc < 4 ) {
		std::cerr << "Usage : " << argv[0] << " <inputXai> <inputFile> <filename>" << std::endl;
		return -1;
	}

	std::string iXai = argv[1];
	std::string iFile = argv[2];
	std::string iFileName = argv[3];

	uint32_t fHash = checksum( iFileName.c_str(), iFileName.size() );

	std::fstream file( iXai.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::binary );

	if ( !file.is_open() ) {
		std::cerr << "ERROR : input XAST not found" << std::endl;
		return -2;
	}

	uint32_t data;

	// XAST header
	uint32_t magic = read32(file);

	// Version : 1.01
	uint32_t version = read32(file);
	uint32_t nbEntries = read32(file); // nbEntries
	uint32_t maxEntries = read32(file); // maxEntries
	uint32_t pathsCount = read32(file);
	uint32_t dataOffset = read32(file);
	uint32_t headerChecksum = read32(file);
	uint32_t headersCount = read32(file);
	uint32_t xaiSize = read32(file);

	uint32_t pathsOffset = 0x30 + headersCount;

	file.seekg(0x30);

	uint64_t fileSize;
	uint64_t fileOffset;
	uint32_t pathOffset;
	uint32_t pathHash;
	uint32_t fileChecksum;
	uint32_t nbActualEntries = 0;
	uint32_t i = 0;
	std::stringstream ss;
	bool found = false;

	for (i = 0; i < maxEntries; i++) {

		pathHash = read32(file);
		pathOffset = read32(file);
		fileChecksum = read32(file);
		uint32_t isXai = read32(file);
		found = (pathHash == fHash);


		ss << std::hex << pathHash << " | " << pathOffset
			<< " | " << fileChecksum << " | " << isXai << std::endl;

		if ( isXai > 1 ) {
			std::cerr << "WARNING : unsupported value found for isXai : "
				<< isXai << std::endl;
		}

		fileSize = read64(file);

		ss << fileSize << std::endl;

		if ( uint64_t padding = read64(file) ) {
			std::cerr << "WARNING : unsupported value found for padding : "
				<< padding << std::endl;
			// return -3;
		}

		fileOffset = read64(file);
		uint64_t aSize = read64(file);

		ss << fileOffset << " | " << aSize << std::endl << std::endl;

		if ( found || file.tellg() >= pathsOffset ) {
			// std::cout << "break" << std::endl;
			// std::cout << std::hex << file.tellg() << std::endl;
			break;
		}
	}

	// std::cerr << ss.str();

	if ( !found ) {
		file.close();
		std::cerr << "File " << iFileName << " not found in this XAST" << std::endl;
		return -1;
	}

	const uint32_t headerOffset = (i+1) * 0x30;


	std::ifstream input( iFile.c_str(), std::ios_base::binary );

	if ( !input.is_open() ) {
		file.close();
		std::cerr << "Failed to open " << iFile << std::endl;
		return -2;
	}

	std::cout << "Writing " << iFileName << " at offset 0x" << std::hex << fileOffset << std::dec << std::endl << std::endl;

	file.seekg(fileOffset);

	uint32_t last = 0;
	uint64_t read = 0;
	uint64_t totalRead = 0;
	uint8_t *buf = new uint8_t[RBUF_SIZE];

	while ( input && file ) {
		input.read( (char*)buf, RBUF_SIZE );

		if ( input )
			read = RBUF_SIZE;

		else
			read = input.gcount();

		totalRead += read;

		uint64_t wSize = read;

		if ( totalRead > fileSize ) {
			wSize -= totalRead - fileSize;
			std::cerr << "ERROR : Input file size differs ! Expected size : " << fileSize << std::endl;
		}

		if ( wSize > 0 ) {
			last = checksum( (const char*)buf, wSize, last );
			file.write( (const char*)buf, wSize );
		}

		if ( totalRead >= fileSize )
			break;
	}

	input.close();

	file.seekp(headerOffset + 8);

	write32(file, last); // File checksum

	std::cout << "Header offset        : 0x" << std::hex << headerOffset << std::dec << std::endl;
	std::cout << "Old file checksum    : 0x" << std::hex << fileChecksum << std::dec << std::endl;
	std::cout << "New file checksum    : 0x" << std::hex << last << std::dec << std::endl;

	file.seekp(0x30);

	uint32_t headerSize = 0x30 * maxEntries;

	if ( RBUF_SIZE < headerSize ) {
		delete buf;
		buf = new uint8_t[ headerSize ];
	}

	file.read( (char*)buf, headerSize );
	uint32_t headerCheck = checksum( (const char*)buf, headerSize);

	file.seekp(0x18);
	write32(file, headerCheck);

	std::cout << "Old header checksum  : 0x" << std::hex << headerChecksum << std::dec << std::endl;
	std::cout << "New header checksum  : 0x" << std::hex << headerCheck << std::dec << std::endl;

	std::cout << std::endl << "XAST file patched with success" << std::endl;

	return 0;
}

