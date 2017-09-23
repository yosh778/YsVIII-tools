// XAST Packer by yosh778

#include <string>
#include <iostream>
#include <algorithm>

#include <fstream>
#include <unordered_map>
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

typedef struct FileData_ {
    std::string fullpath;
    std::string filename;
    uint32_t pathOffset;
    uint32_t pathCheck;
    uint32_t contentCheck;
    uint64_t offset;
    uint64_t size;
} FileData;

std::vector<char*> headerOrder;

bool alphaSorter(const FileData& a, const FileData& b)
{
	return a.filename < b.filename;
}

uint32_t getHeaderOrder(std::vector<char*>& order, std::string input);


int main(int argc, char *argv[])
{
    if ( argc < 3 ) {
        std::cout << "Usage : " << argv[0] << " <dataPath> <output>" << std::endl;
        return -1;
    }

    std::string inPath = argv[1];
    std::string output = argv[2];
    uint32_t mimicUnused = 0;

    // Check input path existence
    if ( !exists( inPath ) ) {
      std::cout << "ERROR : Input directory " << inPath << " does not exist" << std::endl;
      return EXIT_FAILURE;
    }

    if ( argc > 3 ) {
        mimicUnused = getHeaderOrder( headerOrder, argv[3] );
    }

    std::vector<FileData> files;

    recursive_directory_iterator it( inPath ), end;
    FileData fileData;
    uint32_t pathsCount = 0;
    std::unordered_map<std::string, int> headerMap;
    uint32_t i = 0;

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

    std::sort (files.begin(), files.end(), alphaSorter);

    uint32_t nbEntries = files.size();

    std::fstream oFile( output.c_str(), std::ios_base::trunc | std::ios_base::in | std::ios_base::out | std::ios_base::binary );

    if ( !oFile.is_open() ) {
    	std::cout << "Failed to open output file '" << output << "'" << std::endl;
        return -2;
    }

    uint32_t nbUnused = 0;
    uint32_t maxEntries = nbEntries + nbUnused + mimicUnused;
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
    i = 0;

    if ( argc == 3 )
    for ( FileData& file : files ) {

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

        uint64_t aSize = ALIGN_16(file.size);

        // if ( ++i >= files.size() )
        //     aSize = file.size;

        write64(oFile, nextFileOffset);
        write64(oFile, aSize); // Aligned file size

        nextFileOffset += aSize;
    }

    else
    for ( char *filePath : headerOrder ) {

        write32(oFile, -1);
        write32(oFile, 0);
        write32(oFile, -1);
        write32(oFile, 0);

        write64(oFile, 0);
        write64(oFile, 0);

        write64(oFile, 0);
        write64(oFile, 0);
    }

    // Write additional unused entry slots
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
        std::cout << "Error while writing header : " << std::hex << oFile.tellp()
            << " != " << dataOffset << std::dec << std::endl;
        return -2;
    }

    uint32_t n = 0; i = 0;

    for ( FileData& file : files ) {

        std::ifstream input( file.fullpath.c_str(), std::ios_base::binary );

        if ( !input.is_open() )
            continue;

        file.pathOffset = nextPathOffset;
        nextPathOffset += file.filename.size() + 1;

        headerMap[ file.filename ] = i++;

        uint64_t aSize = ALIGN_16(file.size);

        file.offset = nextFileOffset;

        nextFileOffset += aSize;

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
    i = 0;
    uint32_t j = 0;

    if ( argc == 3 )
    for ( FileData& file : files ) {
        oFile.seekp((0x30 * ++i) + 8);

        // write32(oFile, file.pathCheck);
        // write32(oFile, file.pathOffset);
        write32(oFile, file.contentCheck); // File checksum
    }

    else
    for ( char *filePath : headerOrder ) {
        std::string hpath;
        oFile.seekp(0x30 * (i+1));

        if ( filePath && filePath != (char*)-1 ) {
            hpath = std::string(filePath);
            FileData& file = files[ headerMap[ hpath ] ];
            std::cout << "Writing checksum for " << file.filename << std::endl;

            write32(oFile, file.pathCheck);
            write32(oFile, file.pathOffset);
            write32(oFile, file.contentCheck); // File checksum

            bool isXast = extension(file.filename) == ".xai";
            // std::cout << extension(file.filename) << " : " << isXast << std::endl;

            write32(oFile, isXast); // boolean : is it another XAST .xai file ?

            write64(oFile, file.size);
            write64(oFile, 0); // Padding

            write64(oFile, file.offset);
            uint32_t aSize = ALIGN_16(file.size);

            if ( (file.offset + file.size) >= fileSize ) {
                aSize = file.size;
            }

            write64(oFile, aSize); // Aligned file size

            j++;

            // if ( j > pFiles.size() )
            //     return -1;
        }

        i++;
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

uint32_t getHeaderOrder(std::vector<char*>& order, std::string input)
{
    std::ifstream file( input.c_str(), std::ios_base::binary );

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

    uint64_t *fileSizes = new uint64_t[maxEntries];
    uint64_t *fileOffsets = new uint64_t[maxEntries];
    uint32_t *pathOffsets = new uint32_t[maxEntries];
    uint32_t *unk1 = new uint32_t[maxEntries];
    uint32_t *fileChecksums = new uint32_t[maxEntries];
    uint32_t nbActualEntries = 0;
    uint32_t i = 0;

    file.seekg(pathsOffset);

    char *pathsData = new char[pathsCount];
    file.read(pathsData, pathsCount);

    file.seekg(0x30);

    uint32_t nbUnused = 0;

    for (i = 0; i < maxEntries; i++) {

        unk1[i] = read32(file);
        pathOffsets[i] = read32(file);
        fileChecksums[i] = read32(file);
        uint32_t isXai = read32(file);

        if ( pathOffsets[i] && pathOffsets[i] != -1 ) {
            order.push_back( pathsData + (pathOffsets[i] - pathsOffset) );
        // std::cout << "Detected " << ( pathsData + (pathOffsets[i] - pathsOffset) ) << file.filename << std::endl;

        }

        else {
            order.push_back( (char*)-1 );
            nbUnused++;
        }

        fileSizes[i] = read64(file);
        uint64_t padding = read64(file);

        fileOffsets[i] = read64(file);
        uint64_t aSize = read64(file);

        if ( file.tellg() >= pathsOffset ) {
            // std::cout << "break!" << std::endl;
            // std::cout << std::hex << file.tellg() << std::endl;
            break;
        }
    }

    delete fileSizes;
    delete fileOffsets;
    delete pathOffsets;
    delete unk1;
    delete fileChecksums;

    return nbUnused;
}

