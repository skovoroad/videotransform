#include <fstream>
#include <vector>
#include <iostream>

#include "bmp.h"

inline uint32_t make_stride_aligned(uint32_t align_stride, uint32_t row_stride) {
   uint32_t new_stride = row_stride;
   while (new_stride % align_stride != 0) 
     new_stride++;
   return new_stride;
}

bool write_bmp(const char *fname, const uint8_t * data, size_t data_size, int32_t width, int32_t height) {
  BMPFileHeader file_header;
  BMPInfoHeader bmp_info_header;
  BMPColorHeader bmp_color_header;
      
  bmp_info_header.width = width;
  bmp_info_header.height = height;
  bmp_info_header.size = sizeof(BMPInfoHeader);
  file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

  bmp_info_header.bit_count = 24;
  bmp_info_header.compression = 0;
  uint32_t row_stride = width * 3;
  uint32_t new_stride = make_stride_aligned(4, row_stride);
  file_header.file_size = file_header.offset_data + static_cast<uint32_t>(data_size) + bmp_info_header.height * (new_stride - row_stride);
  
  std::ofstream of{ fname, std::ios_base::binary };
  if (!of)
    return false;
  
  if (bmp_info_header.bit_count != 24) 
    return false;

  if (bmp_info_header.width % 4 == 0) {
    of.write((const char*)&file_header, sizeof(file_header));
    of.write((const char*)&bmp_info_header, sizeof(bmp_info_header));
    if(bmp_info_header.bit_count == 32) 
        of.write((const char*)&bmp_color_header, sizeof(bmp_color_header));
    
    of.write((const char*)data, data_size);
  }
  else {
    uint32_t new_stride = make_stride_aligned(4, row_stride);
    std::vector<uint8_t> padding_row(new_stride - row_stride);
    of.write((const char*)&file_header, sizeof(file_header));
    of.write((const char*)&bmp_info_header, sizeof(bmp_info_header));
    if(bmp_info_header.bit_count == 32) 
      of.write((const char*)&bmp_color_header, sizeof(bmp_color_header));
     
    for (int y = 0; y < bmp_info_header.height; ++y) {
      of.write((const char*)(data + row_stride * y), row_stride);
      of.write((const char*)padding_row.data(), padding_row.size());
    }
  }
  return true;
}

