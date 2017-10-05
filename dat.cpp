// DAT Packer by yosh778

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

typedef struct FileData_ {
    std::string fullpath;
    std::string filename;
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
        std::cerr << "Usage : " << argv[0] << " <dataPath> <outputDat>" << std::endl;
        return -1;
    }

    std::string inPath = argv[1];
    std::string output = argv[2];
    uint32_t mimicUnused = 0;

    // Check input path existence
    if ( !exists( inPath ) ) {
      std::cerr << "ERROR : Input directory " << inPath << " does not exist" << std::endl;
      return EXIT_FAILURE;
    }

    // if ( argc > 3 ) {
    //     mimicUnused = getHeaderOrder( headerOrder, argv[3] );
    // }
    // else {
    // 	std::cout << "WARNING : No original XAST archive specified, this could cause game crashes" << std::endl << std::endl;
    // }

    std::vector<FileData> files;

    recursive_directory_iterator it( inPath ), end;
    FileData fileData;
    std::unordered_map<std::string, int> headerMap;
    uint32_t i = 0;

    // Read input directory recursively
    for ( ; it != end; ++it ) {
      bool is_dir = is_directory( *it );

      std::string fullpath = it->path().string();
      std::replace(fullpath.begin(), fullpath.end(), '\\', '/');
      std::string filename = fullpath.substr( inPath.size() );

      if ( filename.front() == '/' ) {
        filename.erase(0, 1);
      }

      // Skip any non-existing file
      if ( !exists( fullpath ) )
        continue;

      if ( !is_dir ) {
        fileData.filename = filename;
        fileData.fullpath = fullpath;
    	// std::cout << "Checksum for " << filename << " : " << std::hex << fileData.pathCheck << std::dec << std::endl;
        fileData.size = file_size( fullpath );

        files.push_back( fileData );
      }
    }

    // if ( argc > 3 && (headerOrder.size()-mimicUnused) != files.size() ) {
    //     std::cout << "When specifying an original file, input folder must contain the same number of files & the same filepaths as the original .xai" << std::endl;
    //     return -1;
    // }

    std::sort (files.begin(), files.end(), alphaSorter);

    uint32_t nbEntries = files.size();

    std::fstream oFile( output.c_str(), std::ios_base::trunc | std::ios_base::in | std::ios_base::out | std::ios_base::binary );

    if ( !oFile.is_open() ) {
    	std::cerr << "Failed to open output file '" << output << "'" << std::endl;
        return -2;
    }

    uint32_t dataOffset = 0x10 + 0x20 * nbEntries;

    // XAST header
    write32(oFile, nbEntries);
    write32(oFile, nbEntries);
    write32(oFile, nbEntries);
    write32(oFile, nbEntries);

    i = 0;
    char nameBuf[16];
    uint64_t nextFileOffset = dataOffset;

    for ( FileData& file : files ) {

        // std::cout << "Checksum for " << file.filename << " : " std::endl;

        strncpy(nameBuf, file.filename.c_str(), 0x10);
        nameBuf[15] = 0;

        oFile.write( nameBuf, 0x10 );


        write32(oFile, nextFileOffset);
        write32(oFile, file.size);
        write64(oFile, 0);

        nextFileOffset += file.size;
    }

    uint8_t *buf = new uint8_t[RBUF_SIZE];

    if ( oFile.tellp() != dataOffset ) {
        std::cerr << "Error while writing header : " << std::hex << oFile.tellp()
            << " != " << dataOffset << std::dec << std::endl;
        return -2;
    }

    uint32_t n = 0; i = 0;

    for ( FileData& file : files ) {

        std::ifstream input( file.fullpath.c_str(), std::ios_base::binary );

        if ( !input.is_open() ) {
	        std::cerr << "Failed to open " << file.fullpath << std::endl;
	        return -2;
        }

        std::cout << "Writing " << file.filename << std::endl;

        uint64_t read = 0;
        uint64_t totalRead = 0;

        while ( input && oFile ) {
            input.read( (char*)buf, RBUF_SIZE );

            if ( input )
                read = RBUF_SIZE;

            else
                read = input.gcount();

            oFile.write( (const char*)buf, read );
            totalRead += read;
        }

        input.close();

        if ( totalRead != file.size ) {
            std::cerr << "Bad file size for " << file.fullpath << std::endl;
            return -3;
        }

        n++;
    }

    oFile.close();

    // if ( argc == 3 ) {
    //     std::cout << std::endl << "WARNING : No original XAST archive specified, this could cause game crashes" << std::endl;
    // }

    std::cerr << std::endl << n << " files were included" << std::endl;
    std::cerr << "DAT archive successfully created" << std::endl;

    return 0;
}

