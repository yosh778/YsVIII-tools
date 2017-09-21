// XAST Packer by yosh778

#include <string>
#include <iostream>
#include <algorithm>

#include <fstream>
#include <cinttypes>
#include <boost/filesystem.hpp>

#define ALIGN_16(n) ((n / 0x10 + (n % 0x10 ? 1 : 0)) * 0x10)
#define RBUF_SIZE 0x1000000

using namespace boost::filesystem;

void write32(std::ofstream& output, uint32_t data) {
    output.write((char*)&data, (int)sizeof(data));
}

uint64_t write64(std::ofstream& output, uint64_t data) {
    output.write((char*)&data, (int)sizeof(data));
}

int main(int argc, char *argv[])
{
    if ( argc < 3 ) {
        std::cout << "Usage : " << argv[0] << " <dataPath> <output>" << std::endl;
        return -1;
    }

    std::cout << "WARNING : this release cannot compute valid checksums" << std::endl;

    std::string inPath = argv[1];
    std::string output = argv[2];

    // Check input path existence
    if ( !exists( inPath ) ) {
      std::cout << "ERROR : Input directory " << inPath << " does not exist" << std::endl;
      return EXIT_FAILURE;
    }

    typedef struct FileData_ {
        std::string fullpath;
        std::string filename;
        uint64_t size;
    } FileData;

    std::vector<FileData> files;

    recursive_directory_iterator it( inPath ), end;
    FileData fileData;
    uint32_t pathsCount = 0;

    // Compress input directory recursively
    for ( ; it != end; ++it ) {
      bool is_dir = is_directory( *it );

      std::string fullpath = it->path().string();
      std::string filename = fullpath.substr( inPath.size() );

      if ( filename.front() == '/' ) {
        filename.erase(0, 1);
      }

      // Skip any non-existing file
      if ( !exists( fullpath ) )
        continue;

      if ( !is_dir ) {
        pathsCount += filename.size() + 1;
        fileData.filename = filename;
        fileData.fullpath = fullpath;
        fileData.size = file_size( fullpath );

        files.push_back( fileData );
      }
    }

    uint32_t nbEntries = files.size();

    std::ofstream oFile( output.c_str(), std::ios_base::binary );

    if ( !oFile.is_open() )
        return -2;

    uint32_t maxEntries = nbEntries;
    uint32_t alignedPathsCount = ALIGN_16(pathsCount);

    uint32_t data;
    uint64_t pathsOffset = 0x30 * (1 + maxEntries);
    uint32_t dataOffset = pathsOffset + alignedPathsCount;

    // // XAST header
    write32(oFile, 0x54534158);
    write32(oFile, 0x01010000); // Version : 1.01
    write32(oFile, nbEntries);
    write32(oFile, nbEntries); // maxEntries

    write32(oFile, pathsCount);
    write32(oFile, dataOffset);

    uint32_t unk0 = 0xA02A4B6A; // File checksum ?
    write32(oFile, unk0);

    uint32_t headersCount = pathsOffset - 0x30;
    write32(oFile, headersCount);

    uint64_t fileSize = -1;
    write64(oFile, fileSize);
    write64(oFile, 0);


    uint64_t nextPathOffset = pathsOffset;
    uint64_t nextFileOffset = dataOffset;

    for ( FileData file : files ) {

        // Entry checksum ?
        uint32_t unk1 = 0x8113D6D3;
        write32(oFile, unk1);

        write32(oFile, nextPathOffset);
        nextPathOffset += file.filename.size() + 1;

        uint32_t checksum = 0xC75EB5E1;
        write32(oFile, checksum); // File checksum ?
        write32(oFile, 0); // Padding

        write64(oFile, file.size);
        write64(oFile, 0); // Padding

        const uint64_t aSize = ALIGN_16(file.size);
        write64(oFile, nextFileOffset);
        write64(oFile, aSize); // Aligned file size

        nextFileOffset += aSize;
    }

    for ( FileData file : files ) {
        oFile.write( file.filename.c_str(), file.filename.size() + 1 );
    }

    uint8_t *buf = new uint8_t[RBUF_SIZE];

    if ( oFile.tellp() % 0x10 ) {
        memset( buf, 0, 0x10 );
        oFile.write( (char*)buf, 0x10 - (oFile.tellp() % 0x10) );
    }

    if ( oFile.tellp() != dataOffset ) {
        std::cout << "Error while writing header" << std::endl;
        return -2;
    }

    uint32_t n = 1;

    for ( FileData file : files ) {

        std::ifstream input( file.fullpath.c_str(), std::ios_base::binary );

        if ( !input.is_open() )
            continue;

        uint64_t read = 0;
        uint64_t totalRead = 0;

        while ( input && oFile ) {
            input.read( (char*)buf, RBUF_SIZE );

            if ( input )
                read = RBUF_SIZE;

            else
                read = input.gcount();

            oFile.write( (char*)buf, read );
            totalRead += read;
        }

        input.close();

        if ( totalRead != file.size ) {
            std::cout << "Bad file size" << std::endl;
            return -3;
        }

        // Align for next file data if needed
        if ( n < files.size() && oFile.tellp() % 0x10 ) {
            memset( buf, 0, 0x10 );
            oFile.write( (char*)buf, 0x10 - (oFile.tellp() % 0x10) );
        }

        n++;
    }

    fileSize = oFile.tellp();

    oFile.seekp(0x20);
    write64(oFile, fileSize);

    oFile.close();

    std::cout << "XAST archive created" << std::endl;

    return 0;
}

