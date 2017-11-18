// PLT Packer by yosh778

#include <string>
#include <iostream>
#include <algorithm>

#include <fstream>
#include <unordered_map>
#include <cinttypes>
#include <cstring>


void write32(std::fstream& output, uint32_t data) {
	output.write((char*)&data, (int)sizeof(data));
}

void write16(std::fstream& output, uint16_t data) {
	output.write((char*)&data, (int)sizeof(data));
}

uint64_t write64(std::fstream& output, uint64_t data) {
	output.write((char*)&data, (int)sizeof(data));
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

int main(int argc, char *argv[])
{
	if ( argc < 3 ) {
		std::cerr << "Usage : " << argv[0] << " <input> <outputData>" << std::endl;
		return -1;
	}

	std::string inPath = argv[1];
	std::string output = argv[2];
	uint32_t mimicUnused = 0;

	uint32_t i = 0;
	uint32_t nbEntries = 0;

	std::ifstream input( inPath.c_str() );

	if ( !input.is_open() ) {
		std::cerr << "Failed to open " << inPath << std::endl;
		return -2;
	}

	std::fstream oFile( output.c_str(), std::ios_base::trunc | std::ios_base::out | std::ios_base::binary );

	if ( !oFile.is_open() ) {
		std::cerr << "Failed to open output file '" << output << "'" << std::endl;
		return -2;
	}

	write16(oFile, nbEntries);

	i = 0;
	char nameBuf[64];

	for( std::string line; getline( input, line ); )
	{
		memset(nameBuf, 0, sizeof(nameBuf));
		strncpy(nameBuf, line.c_str(), sizeof(nameBuf)-1);

		oFile.write( nameBuf, sizeof(nameBuf) );

		std::cerr << "Writing " << nameBuf << std::endl;

		getline( input, line );
		write16(oFile, line.size() );
		oFile.write( line.c_str(), line.size() );

		nbEntries++;
	}

	oFile.seekg(0);
	write16(oFile, nbEntries);

	input.close();
	oFile.close();

	std::cerr << nbEntries << " files were included" << std::endl;
	std::cerr << "PLT archive successfully created" << std::endl;

	return 0;
}

