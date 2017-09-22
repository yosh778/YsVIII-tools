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

void write32(std::fstream& output, uint32_t data) {
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

// Checksum reverse by weaknespase
uint32_t checksum(const char* in, const size_t length, int last = 0){
    const char* end = in + length;
    int acc = last;
    while (in < end)
        acc = (acc * 33) ^ (unsigned char) *in++;
    return acc;
}

int main(int argc, char *argv[])
{
    if ( argc < 3 ) {
        std::cout << "Usage : " << argv[0] << " <dataPath> <output>" << std::endl;
        return -1;
    }

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
        uint32_t pathCheck;
        uint32_t contentCheck;
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
        fileData.pathCheck = checksum( filename.c_str(), filename.size() );
    	// std::cout << "Checksum for " << filename << " : " << std::hex << fileData.pathCheck << std::dec << std::endl;
        fileData.size = file_size( fullpath );

        files.push_back( fileData );
      }
    }

    uint32_t nbEntries = files.size();

    std::fstream oFile( output.c_str(), std::ios_base::trunc | std::ios_base::in | std::ios_base::out | std::ios_base::binary );

    if ( !oFile.is_open() ) {
    	std::cout << "Failed to open output file '" << output << "'" << std::endl;
        return -2;
    }

    uint32_t nbUnused = 0;
    uint32_t maxEntries = nbEntries + nbUnused;
    uint32_t alignedPathsCount = ALIGN_16(pathsCount);

    uint32_t data;
    uint64_t pathsOffset = 0x30 * (1 + maxEntries);
    uint32_t dataOffset = pathsOffset + alignedPathsCount;

    // XAST header
    write32(oFile, 0x54534158);
    write32(oFile, 0x01010000); // Version : 1.01
    write32(oFile, nbEntries);
    write32(oFile, maxEntries);

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

    uint32_t unk1 = 0x8113D6D3;
    uint32_t tmpChecksum = 0xC75EB5E1;

    for ( FileData& file : files ) {

        // std::cout << "opening " << "xai_sums/" << file.filename << std::endl;

        // std::ifstream iSums( ("xai_sums/" + file.filename).c_str(), std::ios_base::binary );

        // if ( iSums.is_open() ) {

        //     // Filepath hash
        //     unk1 = read32(iSums);
        //     checksum = read32(iSums);
        // }
        // else{ printf("FATAL ERROR\n"); return -11;}

        // iSums.close();

    	std::cout << "Checksum for " << file.filename << " : " << std::hex << file.pathCheck << std::dec << std::endl;

        // Filepath checksum
        write32(oFile, file.pathCheck);

        write32(oFile, nextPathOffset);
        nextPathOffset += file.filename.size() + 1;

        write32(oFile, tmpChecksum); // File checksum

        bool isXast = extension(file.filename) == ".xai";
        // std::cout << extension(file.filename) << " : " << isXast << std::endl;

        write32(oFile, isXast); // boolean : is it another XAST .xai file ?

        write64(oFile, file.size);
        write64(oFile, 0); // Padding

        const uint64_t aSize = ALIGN_16(file.size);
        write64(oFile, nextFileOffset);
        write64(oFile, aSize); // Aligned file size

        nextFileOffset += aSize;
    }

    // Write unused entry slots
    for ( uint32_t i = 0; i < nbUnused; i++ ) {

        write32(oFile, -1);
        write32(oFile, 0);
        write32(oFile, -1);
        write32(oFile, 0);

        write64(oFile, 0);
        write64(oFile, 0);

        write64(oFile, 0);
        write64(oFile, 0);
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

    uint32_t n = 0;

    for ( FileData& file : files ) {

        std::ifstream input( file.fullpath.c_str(), std::ios_base::binary );

        if ( !input.is_open() )
            continue;

        std::cout << "Writing " << file.filename << std::endl;

        uint32_t last = 0;
        uint64_t read = 0;
        uint64_t totalRead = 0;

        while ( input && oFile ) {
            input.read( (char*)buf, RBUF_SIZE );

            if ( input )
                read = RBUF_SIZE;

            else
                read = input.gcount();

            last = checksum( (const char*)buf, read, last );
            oFile.write( (const char*)buf, read );
            totalRead += read;
        }

        input.close();

        if ( totalRead != file.size ) {
            std::cout << "Bad file size" << std::endl;
            return -3;
        }

        file.contentCheck = last;

        // Align for next file data if needed
        if ( ++n < files.size() && oFile.tellp() % 0x10 ) {
            memset( buf, 0, 0x10 );
            oFile.write( (char*)buf, 0x10 - (oFile.tellp() % 0x10) );
        }
    }

    fileSize = oFile.tellp();

    oFile.seekp(0x20);
    write64(oFile, fileSize);
    uint32_t i = 0;

    for ( FileData& file : files ) {
        oFile.seekp((0x30 * ++i) + 8);
        write32(oFile, file.contentCheck); // File checksum
    }

    oFile.seekp(0x30);

    uint32_t headerSize = 0x30 * maxEntries;

    if ( RBUF_SIZE < headerSize ) {
        delete buf;
        buf = new uint8_t[ headerSize ];
    }

    oFile.read( (char*)buf, headerSize );
    uint32_t headerCheck = checksum( (const char*)buf, headerSize);

    oFile.seekp(0x18);
    write32(oFile, headerCheck);

    oFile.close();

    std::cout << std::endl << n << " files were included" << std::endl;
    std::cout << "XAST archive successfully created" << std::endl;

    return 0;
}

