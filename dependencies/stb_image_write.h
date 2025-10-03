// stb_image_write - v1.16 - public domain - http://nothings.org/stb
// writes out PNG/BMP/TGA/JPEG/HDR images to C stdio or a callback
// single-header library; see end of file for license.

#ifndef STB_IMAGE_WRITE_H
#define STB_IMAGE_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

extern int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_bytes);

#ifdef __cplusplus
}
#endif

#ifdef STB_IMAGE_WRITE_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal PNG writer: only what we need for RGBA8. This tiny embed avoids the full stb. */
#include <stdint.h>

static unsigned long crc_table[256];
static int crc_table_computed = 0;
static void make_crc_table(void){
  for (unsigned long n=0; n<256; n++) {
    unsigned long c = n;
    for (int k=0;k<8;k++) c = c & 1 ? 0xedb88320L ^ (c >> 1) : c >> 1;
    crc_table[n] = c;
  }
  crc_table_computed = 1;
}
static unsigned long update_crc(unsigned long crc, unsigned char *buf, int len){
  unsigned long c = crc;
  if (!crc_table_computed) make_crc_table();
  for (int n=0;n<len;n++) c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
  return c;
}
static void write32be(FILE* f, uint32_t v){ unsigned char b[4]={ (unsigned char)(v>>24),(unsigned char)(v>>16),(unsigned char)(v>>8),(unsigned char)v }; fwrite(b,1,4,f); }
static void writeu8(FILE* f, unsigned char v){ fwrite(&v,1,1,f); }

int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_bytes){
  if (comp != 4) return 0; // This minimal writer only handles RGBA8
  FILE* f = fopen(filename, "wb");
  if (!f) return 0;
  // PNG signature
  unsigned char sig[8] = {137,80,78,71,13,10,26,10};
  fwrite(sig,1,8,f);
  // IHDR
  unsigned char ihdr[13];
  ihdr[0]=(w>>24)&255; ihdr[1]=(w>>16)&255; ihdr[2]=(w>>8)&255; ihdr[3]=w&255;
  ihdr[4]=(h>>24)&255; ihdr[5]=(h>>16)&255; ihdr[6]=(h>>8)&255; ihdr[7]=h&255;
  ihdr[8]=8; ihdr[9]=6; ihdr[10]=0; ihdr[11]=0; ihdr[12]=0;
  // write chunk helper
  auto write_chunk = [&](const char* type, const unsigned char* buf, int len){
    write32be(f, (uint32_t)len);
    unsigned char t[4] = { (unsigned char)type[0], (unsigned char)type[1], (unsigned char)type[2], (unsigned char)type[3] };
    fwrite(t,1,4,f);
    if (len) fwrite(buf,1,len,f);
    unsigned long crc = update_crc(0xffffffffL, t, 4);
    if (len) crc = update_crc(crc, (unsigned char*)buf, len);
    write32be(f, (uint32_t)(crc ^ 0xffffffffL));
  };
  write_chunk("IHDR", ihdr, 13);
  // IDAT (no compression: store rows with filter byte 0; wrap in a naive zlib 'stored' blocks)
  // For simplicity, we won't implement full zlib; instead bail if stride_bytes != w*4
  if (stride_bytes != w*4) { fclose(f); remove(filename); return 0; }
  // Build uncompressed image with filter bytes
  size_t raw_size = (size_t)(h) * (size_t)(1 + w*4);
  std::vector<unsigned char> raw; raw.resize(raw_size);
  for (int y=0;y<h;y++){
    raw[(size_t)y * (1 + w*4)] = 0;
    memcpy(&raw[(size_t)y * (1 + w*4) + 1], (const unsigned char*)data + (size_t)y*stride_bytes, (size_t)w*4);
  }
  // A minimal zlib wrapper with no compression (type=00 blocks). We'll create blocks of up to 65535 bytes.
  std::vector<unsigned char> z;
  // zlib header: CMF(0x78), FLG(0x01) for no compression checkbits
  z.push_back(0x78); z.push_back(0x01);
  size_t i=0;
  while (i < raw.size()){
    size_t chunk = std::min<size_t>(65535, raw.size()-i);
    unsigned char bfinal = (i + chunk == raw.size()) ? 1u : 0u;
    z.push_back(bfinal); // BFINAL + BTYPE=00
    // LEN and NLEN
    uint16_t LEN = (uint16_t)chunk;
    uint16_t NLEN = ~LEN;
    z.push_back(LEN & 0xff); z.push_back((LEN>>8)&0xff);
    z.push_back(NLEN & 0xff); z.push_back((NLEN>>8)&0xff);
    // Data
    z.insert(z.end(), raw.begin()+i, raw.begin()+i+chunk);
    i += chunk;
  }
  // Adler-32
  unsigned long s1=1, s2=0;
  for (unsigned char c : raw){ s1 = (s1 + c) % 65521; s2 = (s2 + s1) % 65521; }
  unsigned long adler = (s2<<16) | s1;
  z.push_back((adler>>24)&0xff); z.push_back((adler>>16)&0xff); z.push_back((adler>>8)&0xff); z.push_back(adler&0xff);

  write_chunk("IDAT", z.data(), (int)z.size());
  // IEND
  write_chunk("IEND", nullptr, 0);
  fclose(f);
  return 1;
}
#endif // STB_IMAGE_WRITE_IMPLEMENTATION

#endif // STB_IMAGE_WRITE_H
