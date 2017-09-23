// XAST Unpacker by yosh778

#include <string>
#include <iostream>
#include <algorithm>

#include <sstream>
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
    if ( argc < 3 ) {
        std::cout << "Usage : " << argv[0] << " <input> <output>" << std::endl;
        return -1;
    }

    std::string iDir = argv[1];
    std::string oDir = argv[2];

    std::ifstream file( iDir.c_str(), std::ios_base::binary );

    if ( !file.is_open() )
        return -2;

    uint32_t data;

    // Version : 1.01
    uint32_t nbEntries = read32(file); // nbEntries
    nbEntries = read32(file); // nbEntries
    nbEntries = read32(file); // nbEntries
    nbEntries = read32(file); // nbEntries

    uint64_t *fileSizes = new uint64_t[nbEntries];
    uint64_t *fileOffsets = new uint64_t[nbEntries];
    uint32_t i = 0;

    char **fileNames = new char*[nbEntries];

    for (i = 0; i < nbEntries; i++) {

	    char *fileName = new char[0x10];
	    file.read(fileName, 0x10);
	    fileNames[i] = fileName;
        std::cout << fileName << std::endl;

        fileOffsets[i] = read32(file);
        fileSizes[i] = read32(file);

        uint32_t padding = read64(file);
    }


    uint8_t *buf = NULL;
    uint32_t bufSize = 0;
    uint32_t nFoundEntries = 0;

    for ( i = 0; i < nbEntries; i++ ) {

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

        std::string base = fileNames[i];
        std::string opath( oDir + "/" + base );
        std::cout << base << std::endl;


        boost::system::error_code returnedError;
        boost::filesystem::path path( opath );
        //std::cout << path.parent_path() << std::endl;

        boost::filesystem::create_directories(
            path.parent_path(), returnedError
        );


        std::ofstream output( opath.c_str(), std::ios_base::binary );

        if ( !output.is_open() )
            continue;

        output.write( (char*)buf, size );
        output.close();
    }

    std::cout <<
    std::endl << "Number of declared entries      : " << nbEntries << std::endl;
    std::cout << "Number of extracted entries     : " << nFoundEntries << std::endl;

    file.close();

    return 0;
}

