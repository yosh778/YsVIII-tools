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
	uint32_t nbEntries = read32(file); // nbEntries
	uint32_t maxEntries = read32(file); // maxEntries
	uint32_t pathsCount = read32(file);
	uint32_t dataOffset = read32(file);
	uint32_t unk0 = read32(file);
	uint32_t headersCount = read32(file);
	uint32_t fileSize = read32(file);

	uint32_t pathsOffset = 0x30 + headersCount;

	file.seekg(0x30);

	uint64_t *fileSizes = new uint64_t[maxEntries];
	uint64_t *fileOffsets = new uint64_t[maxEntries];
	uint32_t *pathOffsets = new uint32_t[maxEntries];
	uint32_t *unk1 = new uint32_t[maxEntries];
	uint64_t *fileChecksums = new uint64_t[maxEntries];
	uint32_t nbActualEntries = 0;
	uint32_t i = 0;

	for (i = 0; i < maxEntries; i++) {

		// Skip unknown data (checksum ?)
		unk1[i] = read32(file);
		pathOffsets[i] = read32(file);
		fileChecksums[i] = read64(file);

		fileSizes[i] = read64(file);
		file.seekg(8, std::	ios_base::cur);

		fileOffsets[i] = read64(file);
		file.seekg(8, std::	ios_base::cur);

		if ( file.tellg() >= pathsOffset ) {
			// std::cout << "break!" << std::endl;
			// std::cout << std::hex << file.tellg() << std::endl;
			break;
		}
	}

	nbActualEntries = i + 1;

	// std::cout << "Number of declared entries : " << maxEntries << std::endl;
	// std::cout << "Number of actual entries   : " << nbActualEntries << std::endl;
	// std::cout << "Number of used entries     : " << nbEntries << std::endl;

	char **fileNames = new char*[maxEntries];

	file.seekg(pathsOffset);

	char *pathsData = new char[pathsCount];
	file.read(pathsData, pathsCount);

	uint32_t n = 0;
	char *next = pathsData;
	char *end = pathsData + pathsCount;

	i = 0;

	while ( next < end ) {

		fileNames[n++] = next;

		// Prints file paths
		// std::cout << next << std::endl;

		while ( pathsData[i++] && pathsData < end );

		next = pathsData + i;
	}

	// std::cout << "Number of filepaths        : " << n << std::endl << std::endl;
	// std::cout << "Start                      : " << std::hex << pathsOffset << std::endl << std::endl;
	// std::cout << "Offset                     : " << std::hex << pathsOffset + i << std::dec << std::endl << std::endl;

	uint8_t *buf = NULL;
	uint32_t bufSize = 0;
	uint32_t nFoundEntries = 0;

	for ( i = 0; i < maxEntries; i++ ) {

		uint32_t offset = fileOffsets[i];
		uint32_t size = fileSizes[i];

		if ( !offset || !size )
			continue;

		nFoundEntries++;

		file.seekg( offset );

		if ( size > bufSize ) {
			buf = (uint8_t*)realloc( buf, size );
			bufSize = size;
		}
		file.read( (char*)buf, size );

		std::string opath( pathsData + (pathOffsets[i] - pathsOffset) );
		std::cout << opath << std::endl;


		boost::filesystem::path rootPath ( "./basePath/extraPath/" );
		boost::system::error_code returnedError;

		boost::filesystem::path path( opath );
		//std::cout << path.parent_path() << std::endl;

		boost::filesystem::create_directories(
			path.parent_path(), returnedError
		);

		// if ( opath == "text/b008.sab" ) {
		// 	std::cout << "text/b008.sab detected" << std::endl;
		// 	std::cout << std::hex << unk1[i] << std::endl;
		// 	std::cout << std::hex << fileChecksums[i] << std::endl;
		// }

		// if ( opath == "text/b008bit.sab" ) {
		// 	std::cout << "text/b008bit.sab detected" << std::endl;
		// 	std::cout << std::hex << unk1[i] << std::endl;
		// 	std::cout << std::hex << fileChecksums[i] << std::endl;
		// }

		// if ( opath == "shader/clear_f.ssf" ) {
		// 	std::cout << "shader/clear_f.ssf detected" << std::endl;
		// 	std::cout << "i = " << i << std::endl;
		// 	std::cout << "offset = " << std::hex << offset << std::endl;
		// 	std::cout << "size = " << std::hex << size << std::endl;
		// 	std::cout << std::hex << unk1[i] << std::endl;
		// 	std::cout << std::hex << fileChecksums[i] << std::endl;
		// }

		std::ofstream output( opath.c_str(), std::ios_base::binary );

		if ( !output.is_open() )
			continue;

		output.write( (char*)buf, size );
		output.close();
	}

	std::cout <<
	std::endl << "Number of declared entries      : " << maxEntries << std::endl;
	std::cout << "Number of actual entries        : " << nbActualEntries << std::endl;
	std::cout << "Number of used entries          : " << nbEntries << std::endl;
	std::cout << "Number of filepaths             : " << n << std::endl;
	std::cout << "Number of extracted entries     : " << nFoundEntries << std::endl;

	return 0;
}

