// Scene tool

#pragma once
#include <cinttypes>

struct __attribute__((__packed__)) FILE_HEADER_V2 {
  char                        magic[8];                 //"YS7_SCP\0"
  uint8_t                     reserved[3];              //Isn't accessed by implementation
  uint8_t                     version;               //0x4 in mine, 0x2-0x4 use V2 header
  uint64_t                    checksum;    //Doesn't appear to be checked by loader
  uint32_t                    segs_count;            //Segments count
  // SEGMENT_HEADER[segs_count]  segments;              //Segment headers
};

struct __attribute__((__packed__)) SEGMENT_HEADER {
  char        name[0x20];      //Segment name
  uint32_t    size;      //Size of segment data
  uint32_t    offset;    //Offset to segment data from the beginning of file
};

typedef uint16_t OPCODE;
typedef uint16_t DATA_TAG;

enum DataTag {

	INT_TAG = 0x82DD,
	FLOAT_TAG = 0x82DE,
  STRING_TAG = 0x82DF,
	UNK0_TAG = 0x82E0,

  POPUP_TAG = 0x2020,
};


struct __attribute__((__packed__)) GENERIC_ARG {

  uint16_t  tag;

  union __attribute__((__packed__)) {
    int32_t  iVal;
    float    fVal;
    uint32_t uVal;
  };
};


struct __attribute__((__packed__)) INT_ARG {
  uint16_t  tag_82DD;
  int32_t   value;
};

struct __attribute__((__packed__)) FLOAT_ARG {
  uint16_t  tag_82DE;
  float     value;
};

struct __attribute__((__packed__)) STRING_ARG {
  uint16_t  tag_82DF;
  uint32_t  cch;
  // char[cch] text;
};

struct __attribute__((__packed__)) UNK0_ARG {
  uint16_t        tag_82E0;
  uint32_t        length;
  // uint8_t[length] value;
};

struct __attribute__((__packed__)) POPUP_ARG {
  uint16_t  tag_2020;
  uint32_t  nLines;
  uint32_t  len;
  // uint16_t[nLines] linePositions?
  // char[len] popupText
};

