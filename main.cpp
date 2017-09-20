// XAST Unpacker by yosh778

#include <string>
#include <iostream>
#include <algorithm>

#include <fstream>
#include <cinttypes>
#include <boost/filesystem.hpp>

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
	if ( argc < 2 )
		return -1;

	std::ifstream file(argv[1], std::ios_base::binary);

	if ( !file.is_open() )
		return -2;

	uint32_t data;

	// XAST header
	uint32_t magic = read32(file);

	// Version : 1.01
	uint32_t version = read32(file);
	uint32_t nheaders = read32(file);
	uint32_t nfiles = read32(file);
	uint32_t pathsCount = read32(file);
	uint32_t dataOffset = read32(file);
	uint32_t unk0 = read32(file);
	uint32_t headersCount = read32(file);
	uint32_t fileSize = read32(file);

	uint32_t pathsOffset = 0x30 + headersCount;

	file.seekg(0x30);

	uint64_t *fileSizes = new uint64_t[nfiles];
	uint64_t *fileOffsets = new uint64_t[nfiles];
	uint32_t *pathOffsets = new uint32_t[nfiles];

	for (uint64_t i = 0; i < nheaders; i++) {

		// Skip unknown data (checksum ?)
		uint32_t unk1 = read32(file);
		pathOffsets[i] = read32(file);
		uint64_t unk2 = read64(file);

		fileSizes[i] = read64(file);
		file.seekg(8, std::	ios_base::cur);

		fileOffsets[i] = read64(file);
		file.seekg(8, std::	ios_base::cur);
	}

	char **fileNames = new char*[nfiles];

	file.seekg(pathsOffset);

	char *pathsData = new char[pathsCount];
	file.read(pathsData, pathsCount);

	uint32_t n = 0;
	uint32_t i = 0;
	char *next = pathsData;
	char *end = pathsData + pathsCount;

	// while ( next < end ) {

	// 	fileNames[n++] = next;

	// 	// Prints file paths
	// 	//std::cout << next << std::endl;

	// 	while ( pathsData[i++] && pathsData < end );

	// 	// Skip null terminating byte
	// 	next = pathsData + ++i;
	// }

	uint8_t *buf = NULL;

	for ( i = 0; i < nfiles; i++ ) {

		uint32_t offset = fileOffsets[i];
		uint32_t size = fileSizes[i];

		if ( !offset || !size )
			continue;

		file.seekg( offset );

		buf = (uint8_t*)realloc( buf, size );
		file.read( (char*)buf, size );

		std::string opath( pathsData + (pathOffsets[i] - pathsOffset) );
		std::cout << opath << std::endl;


		boost::filesystem::path rootPath ( "./basePath/extraPath/" );
		boost::system::error_code returnedError;

		boost::filesystem::path path( opath );
		std::cout << path.parent_path() << std::endl;

		boost::filesystem::create_directories(
			path.parent_path(), returnedError
		);


		std::ofstream output( opath.c_str(), std::ios_base::binary );

		if ( !output.is_open() )
			continue;

		output.write( (char*)buf, size );
		output.close();
	}


	return 0;
}

