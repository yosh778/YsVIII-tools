// PLT Unpacker by yosh778

#include <string>
#include <iostream>
#include <algorithm>

#include <sstream>
#include <fstream>
#include <cinttypes>

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

    uint16_t nbEntries = read16(file);
	char fileName[64];

    uint8_t *buf = NULL;
    uint16_t bufSize = 0;
    uint32_t nFoundEntries = 0;

    std::ofstream output( oDir.c_str(), std::ios_base::trunc );

    if ( !output.is_open() )
        return -1;


    for (uint32_t i = 0; i < nbEntries; i++) {

	    file.read(fileName, sizeof(fileName));
	    fileName[63] = 0;

        std::cout << fileName << std::endl;

        uint16_t size = read16(file);

        if ( !size )
            continue;

        nFoundEntries++;

        if ( (size+1) > bufSize ) {
            buf = (uint8_t*)realloc( buf, size+1 );
            bufSize = size+1;
        }

        file.read( (char*)buf, size );
        buf[size] = 0;

        output << fileName << std::endl;
        output << buf << std::endl;
    }

    output.close();

    std::cout <<
    std::endl << "Number of declared entries      : " << nbEntries << std::endl;
    std::cout << "Number of extracted entries     : " << nFoundEntries << std::endl;

    file.close();

    return 0;
}

