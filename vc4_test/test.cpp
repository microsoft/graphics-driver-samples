/*
The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "mailbox.h"
#include "v3d.h"

// I/O access
volatile unsigned *v3d;
int mbox;

#define screen_x  800
#define screen_y  600

int tile_x = (screen_x / 64) + ((screen_x % 64) ? 1 : 0);
int tile_y = (screen_y / 64) + ((screen_y % 64) ? 1 : 0);

int SaveBMP(const char* pFileName, void *pImage)
{
    FILE * fp = NULL;

    fp = fopen(pFileName, "wb");

#pragma pack(push, 2)

    struct BMPHeader
    {
        uint16_t  m_id;
        uint32_t  m_fileSize;
        uint32_t  m_unused;
        uint32_t  m_pixelArrayOffset;
    };

    struct DIBHeader
    {
        uint32_t  m_dibHeaderSize;
        uint32_t  m_widthPixels;
        uint32_t  m_heightPixels;
        uint16_t  m_numPlanes;
        uint16_t  m_bitsPerPixel;
        uint32_t  m_compressionMethod;
        uint32_t  m_pixelDataSize;
        uint32_t  m_pixelsPerMeterHorizontal;
        uint32_t  m_pixelsPerMeterVertical;
        uint32_t  m_colorsInPalette;
        uint32_t  m_importantColors;
    };

#pragma pack(pop)

    uint32_t pixelWidth = screen_x;
    uint32_t pixelHeight = screen_y;
    uint32_t byteWidthNoPadding = pixelWidth * 3;
    uint32_t byteWidth = (byteWidthNoPadding + 3) & ~3;
    uint32_t bytePadding = byteWidth - byteWidthNoPadding;
    uint32_t pixelDataSize = byteWidth * pixelHeight;

    BMPHeader bmpHeader;

    bmpHeader.m_id = 0x4D42;
    bmpHeader.m_fileSize = sizeof(BMPHeader) + sizeof(DIBHeader) + pixelDataSize;
    bmpHeader.m_pixelArrayOffset = sizeof(BMPHeader) + sizeof(DIBHeader);
    bmpHeader.m_unused = 0;

    DIBHeader dibHeader;

    dibHeader.m_bitsPerPixel = 24;
    dibHeader.m_colorsInPalette = 0;
    dibHeader.m_compressionMethod = 0;
    dibHeader.m_dibHeaderSize = sizeof(DIBHeader);
    dibHeader.m_heightPixels = pixelHeight;
    dibHeader.m_importantColors = 0;
    dibHeader.m_numPlanes = 1;
    dibHeader.m_pixelDataSize = pixelDataSize;
    dibHeader.m_pixelsPerMeterHorizontal = 2835;
    dibHeader.m_pixelsPerMeterVertical = 2835;
    dibHeader.m_widthPixels = pixelWidth;

    fwrite(&bmpHeader, sizeof(bmpHeader), 1, fp);
    fwrite(&dibHeader, sizeof(dibHeader), 1, fp);

    uint8_t * pData = reinterpret_cast<uint8_t*>(pImage);
    assert(pData);

    int padding = 0;
    for (uint32_t row = 0; row < pixelHeight; row++)
    {
        uint8_t * pRow = pData + ((pixelHeight - row - 1) * (pixelWidth * 4));

        for (uint32_t col = 0; col < pixelWidth; col++)
        {
                fwrite(pRow + 0, 1, 1, fp);
                fwrite(pRow + 1, 1, 1, fp);
                fwrite(pRow + 2, 1, 1, fp);

            pRow += 4;
        }
        if (bytePadding) fwrite(&padding, 1, bytePadding, fp);
    }
        
    fclose(fp);

    return 0;
}

void addbyte(uint8_t **list, uint8_t d) {
  *((*list)++) = d;
}

void addshort(uint8_t **list, uint16_t d) {
  *((*list)++) = (d) & 0xff;
  *((*list)++) = (d >> 8)  & 0xff;
}

void addword(uint8_t **list, uint32_t d) {
  *((*list)++) = (d) & 0xff;
  *((*list)++) = (d >> 8)  & 0xff;
  *((*list)++) = (d >> 16) & 0xff;
  *((*list)++) = (d >> 24) & 0xff;
}

void addfloat(uint8_t **list, float f) {
  uint32_t d = *((uint32_t *)&f);
  *((*list)++) = (d) & 0xff;
  *((*list)++) = (d >> 8)  & 0xff;
  *((*list)++) = (d >> 16) & 0xff;
  *((*list)++) = (d >> 24) & 0xff;
}

// Memory Map

enum bo {
    BO_BCL,
    BO_INDEX,
    BO_TSD,
    BO_TA,
    BO_RCL,
    BO_FS,
    BO_VSUNIFORM,
    BO_CSUNIFORM,
    BO_VS,
    BO_CS,
    BO_VERTEX,
    BO_SHADER,
    BO_FSUNIFORM,
    BO_TEXTURE,
    BO_FB,
    BO_TOTAL,
};

uint32_t offsets[] = {
    0,       // binning control list
    0x80,    // index buffer
    0x100,   // tile state data address
    0x6200,  // tile allocation memory address. 32bytes per tile.
    0xe200,  // render control list
    0xfe00,  // FS shader
    0x10000, // VS uniforms
    0x10080, // CS uniforms
    0x10100, // VS shader
    0x10200, // CS shader
    0x10300, // vertex buffer
    0x10800, // shader record
    0x10a00, // FS uniforms
    0x11000, // texture data (64*64*4)
    0x20000, // frame buffer
    0xa00000, // Total size
};

// Render a single triangle to memory.
void testTriangle() {
  // Like above, we allocate/lock/map some videocore memory
  // I'm just shoving everything in a single buffer because I'm lazy
  // 8Mb, 4k alignment
  unsigned int handle = mem_alloc(mbox, offsets[BO_TOTAL], 0x1000, MEM_FLAG_DIRECT | MEM_FLAG_ZERO);
  if (!handle) {
    printf("Error: Unable to allocate memory");
    return;
  }
  uint32_t bus_addr = mem_lock(mbox, handle); 
  uint8_t *list = (uint8_t*) mapmem((bus_addr & ~0xC0000000), offsets[BO_TOTAL]);

  printf("TA size check: %d %d\n", (tile_x * 32 * tile_y), (offsets[BO_TA+1] - offsets[BO_TA])); // make sure tile allocation memory is enough 
  assert((tile_x * 32 * tile_y) < (offsets[BO_TA+1] - offsets[BO_TA])); // make sure tile allocation memory is enough 

  printf("FB size check: %d %d\n", (screen_x * 4 * screen_y), (offsets[BO_FB+1] - offsets[BO_FB])); // make file frame buffer is enough
  assert((screen_x * 4 * screen_y) < (offsets[BO_FB+1] - offsets[BO_FB])); // make file frame buffer is enough

  uint8_t *p = list;

  // Configuration stuff
  // Tile Binning Configuration.
  //   Tile state data is 48 bytes per tile, I think it can be thrown away
  //   as soon as binning is finished.
  addbyte(&p, 112);
  addword(&p, bus_addr + offsets[BO_TA]); // tile allocation memory address
  addword(&p, offsets[BO_TA + 1] - offsets[BO_TA]); // tile allocation memory size
  addword(&p, bus_addr + offsets[BO_TSD]); // Tile state data address
  addbyte(&p, tile_x); // number of tile 
  addbyte(&p, tile_y); // number of tile
  addbyte(&p, 0x04); // config

  // Start tile binning.
  addbyte(&p, 6);

  // Primitive type
  addbyte(&p, 56);
  addbyte(&p, 0x12); // 16 bit index, triangle

  // Raster State
  addbyte(&p, 96);
  addbyte(&p, 0x07); // enable both foward and back facing polygons
  addbyte(&p, 0x00); // depth testing disabled
  addbyte(&p, 0x02); // enable early depth write

  // Clip Window
  addbyte(&p, 102);
  addshort(&p, 0);
  addshort(&p, 0);
  addshort(&p, screen_x); // width
  addshort(&p, screen_y); // height

  // Viewport offset
  addbyte(&p, 103);
  addshort(&p, (screen_x / 2) * 16.0f); // width
  addshort(&p, (screen_y / 2) * 16.0f); // height

  // Z min/max
  //addbyte(&p, 104);
  //addfloat(&p, 0.0f); // min
  //addfloat(&p, 1.0f); // max

  // clipper x/y scaling
  addbyte(&p, 105);
  addfloat(&p, (screen_x / 2) * 16.0f); // width
  addfloat(&p, (screen_y / 2) * 16.0f); // hight

  // clipper z scaling
  addbyte(&p, 106);
  addfloat(&p, 0.5f); // scale
  addfloat(&p, 0.5f); // offset

  // GL shader state
  addbyte(&p, 64);
  addword(&p, (bus_addr + offsets[BO_SHADER]) | 0x1); // Shader Record | number of attributes

  // primitive index list
  addbyte(&p, 32);
  addbyte(&p, 0x04); // 8bit index, trinagles
  addword(&p, 3); // Length
  addword(&p, bus_addr + offsets[BO_INDEX]); // index buffer address
  addword(&p, 2); // Maximum index

  // End of bin list
  // Flush
  addbyte(&p, 5);
  // Nop
  addbyte(&p, 1);
  // Halt
  addbyte(&p, 0);

  int length = p - list;
  assert(length < (offsets[BO_RCL + 1] - offsets[BO_RCL]));

// index buffer
  p = list + offsets[BO_INDEX];
  addbyte(&p, 0); // top
  addbyte(&p, 1); // bottom left
  addbyte(&p, 2); // bottom right

// Vertex Data
  p = list + offsets[BO_VERTEX];
#define NUM_VERTEX 7
#define NUM_VARY   3
  addfloat(&p,   0.0f); // Xc
  addfloat(&p,  -0.5f); // Yc
  addfloat(&p,   1.0f); // Z
  addfloat(&p,   1.0f); // W
  addfloat(&p,   1.0f); // Varying 0 (b)
  addfloat(&p,   0.0f); // Varying 1 (g)
  addfloat(&p,   0.0f); // Varying 2 (r)

  addfloat(&p,  -1.0f); // Xc
  addfloat(&p,   1.0f); // Yc
  addfloat(&p,   1.0f); // Z
  addfloat(&p,   1.0f); // W
  addfloat(&p,   0.0f); // Varying 0 (b)
  addfloat(&p,   1.0f); // Varying 1 (g)
  addfloat(&p,   0.0f); // Varying 2 (r)

  addfloat(&p,   1.0f); // Xc
  addfloat(&p,   1.0f); // Yc
  addfloat(&p,   1.0f); // Z
  addfloat(&p,   1.0f); // W
  addfloat(&p,   0.0f); // Varying 0 (b)
  addfloat(&p,   0.0f); // Varying 1 (g)
  addfloat(&p,   1.0f); // Varying 2 (r)

// VS uniforms
  p = list + offsets[BO_VSUNIFORM];
#define NUM_VSUNIFORM 2
  addfloat(&p, ((screen_x / 2) * 16.0f));
  addfloat(&p, ((screen_y / 2) * 16.0f));

// CS uniforms
#define NUM_CSUNIFORM 2
  p = list + offsets[BO_CSUNIFORM];
  addfloat(&p, ((screen_x / 2) * 16.0f));
  addfloat(&p, ((screen_y / 2) * 16.0f));

// FS uniforms
  p = list + offsets[BO_FSUNIFORM];
#define NUM_FSUNIFORM 0 

// GL Shader Record
  p = list + offsets[BO_SHADER];
  addshort(&p, 0x05); // enable clipping

  addbyte(&p, NUM_FSUNIFORM);    // FS: number of uniforms
  addbyte(&p, NUM_VARY);    // FS: number of varying
  addword(&p, bus_addr + offsets[BO_FS]); // FS code
  addword(&p, bus_addr + offsets[BO_FSUNIFORM]); // FS uniforms

  addshort(&p, NUM_VSUNIFORM);   // VS: number of uniforms
  addbyte(&p, 1);    // VS: number of attributes
  addbyte(&p, NUM_VERTEX*4);   // VS: total attribute size
  addword(&p, bus_addr + offsets[BO_VS]); // VS shader code
  addword(&p, bus_addr + offsets[BO_VSUNIFORM]); // VS uniforms 

  addshort(&p, NUM_CSUNIFORM);   // CS: number of uniforms
  addbyte(&p, 1);    // CS: number of attributes
  addbyte(&p, NUM_VERTEX*4);   // CS: total attribute size
  addword(&p, bus_addr + offsets[BO_CS]); // CS shader code
  addword(&p, bus_addr + offsets[BO_CSUNIFORM]); // CS uniforms 

  addword(&p, bus_addr + offsets[BO_VERTEX]); // Vertex Data
  addbyte(&p, (NUM_VERTEX*4)-1); // bytes - 1 in attributes
  addbyte(&p, NUM_VERTEX*4); // stride
  addbyte(&p, 0);  // VS: VPM offset
  addbyte(&p, 0);  // CS: VPM offset

// fragment shader
  p = list + offsets[BO_FS];
  uint64_t fs[] = {  
    0xd1724823958e0dbf, /* mov r0, vary; mov r3.8d, 1.0 */
    0x40024821818e7176, /* fadd r0, r0, r5; mov r1, vary */
    0x10024862818e7376, /* fadd r1, r1, r5; mov r2, vary */
    0x114248a3819e7540, /* fadd r2, r2, r5; mov r3.8a, r0 */
    0x115049e3809e7009, /* nop; mov r3.8b, r1 */
    0x116049e3809e7012, /* nop; mov r3.8c, r2 */
    0x30020ba7159e76c0, /* mov tlbc, r3; nop; thrend */
    0x100009e7009e7000, /* nop; nop; nop */
    0x500009e7009e7000, /* nop; nop; sbdone */
  };
  memcpy((void *)p,(void *)fs, sizeof(fs));

// VS
  p = list + offsets[BO_VS];
  uint64_t vs[] = {
    0xe0020c6700701a00, // ldi  ; mov vr_setup, 0x00701a00 
    0xe0021c6700001a00, // ldi  ; mov vw_setup, 0x00001a00 
    0x1002082715c27d80, //      ; mov r0, vpm ; nop      // pm = 0, sf = 0, ws = 0
    0x1002086715c27d80, //      ; mov r1, vpm ; nop      // pm = 0, sf = 0, ws = 0
    0x100200a715c27d80, //      ; mov ra2, vpm ; nop     // pm = 0, sf = 0, ws = 0
    0x100200e715c27d80, //      ; mov ra3, vpm ; nop     // pm = 0, sf = 0, ws = 0
    0x100059c020827006, //      ; nop  ; fmul ra0, r0, uniform   // pm = 0, sf = 0, ws = 1
    0x100059c12082700e, //      ; nop  ; fmul ra1, r1, uniform   // pm = 0, sf = 0, ws = 1
    0x1012012707027d80, //      ; ftoi ra4.16a, ra0, ra0 ; nop   // pm = 0, sf = 0, ws = 0
    0x1022012707067d80, //      ; ftoi ra4.16b, ra1, ra1 ; nop   // pm = 0, sf = 0, ws = 0
    0x1002016715c27d80, //      ; mov ra5, vpm ; nop     // pm = 0, sf = 0, ws = 0
    0x100201a715c27d80, //      ; mov ra6, vpm ; nop     // pm = 0, sf = 0, ws = 0
    0x100201e715c27d80, //      ; mov ra7, vpm ; nop     // pm = 0, sf = 0, ws = 0
    0x10020c2715127d80, //      ; mov vpm, ra4 ; nop     // pm = 0, sf = 0, ws = 0
    0x10020c27150a7d80, //      ; mov vpm, ra2 ; nop     // pm = 0, sf = 0, ws = 0
    0x10020c27150e7d80, //      ; mov vpm, ra3 ; nop     // pm = 0, sf = 0, ws = 0
    0x10020c2715167d80, //      ; mov vpm, ra5 ; nop     // pm = 0, sf = 0, ws = 0
    0x10020c27151a7d80, //      ; mov vpm, ra6 ; nop     // pm = 0, sf = 0, ws = 0
    0x10020c27151e7d80, //      ; mov vpm, ra7 ; nop     // pm = 0, sf = 0, ws = 0
    0x300009e7009e7000, // thrend       ; nop  ; nop
    0x100009e7009e7000, //      ; nop  ; nop
    0x100009e7009e7000, //      ; nop  ; nop
  };
  memcpy((void *)p,(void *)vs, sizeof(vs));

// CS
  p = list + offsets[BO_CS];
  uint64_t cs[] =
  {
    0xe0020c6700701a00, // ldi  ; mov vr_setup, 0x00701a00 
    0xe0021c6700001a00, // ldi  ; mov vw_setup, 0x00001a00 
    0x1002082715c27d80, //      ; mov r0, vpm ; nop      // pm = 0, sf = 0, ws = 0
    0x1002086715c27d80, //      ; mov r1, vpm ; nop      // pm = 0, sf = 0, ws = 0
    0x100200a715c27d80, //      ; mov ra2, vpm ; nop     // pm = 0, sf = 0, ws = 0
    0x100200e715c27d80, //      ; mov ra3, vpm ; nop     // pm = 0, sf = 0, ws = 0
    0x100059c020827006, //      ; nop  ; fmul ra0, r0, uniform   // pm = 0, sf = 0, ws = 1
    0x100059c12082700e, //      ; nop  ; fmul ra1, r1, uniform   // pm = 0, sf = 0, ws = 1
    0x1012012707027d80, //      ; ftoi ra4.16a, ra0, ra0 ; nop   // pm = 0, sf = 0, ws = 0
    0x1022012707067d80, //      ; ftoi ra4.16b, ra1, ra1 ; nop   // pm = 0, sf = 0, ws = 0
    0x1002016715c27d80, //      ; mov ra5, vpm ; nop     // pm = 0, sf = 0, ws = 0
    0x100201a715c27d80, //      ; mov ra6, vpm ; nop     // pm = 0, sf = 0, ws = 0
    0x100201e715c27d80, //      ; mov ra7, vpm ; nop     // pm = 0, sf = 0, ws = 0
    0x10020c27159e7000, //      ; mov vpm, r0 ; nop      // pm = 0, sf = 0, ws = 0
    0x10020c27159e7240, //      ; mov vpm, r1 ; nop      // pm = 0, sf = 0, ws = 0
    0x10020c27150a7d80, //      ; mov vpm, ra2 ; nop     // pm = 0, sf = 0, ws = 0
    0x10020c27150e7d80, //      ; mov vpm, ra3 ; nop     // pm = 0, sf = 0, ws = 0
    0x10020c2715127d80, //      ; mov vpm, ra4 ; nop     // pm = 0, sf = 0, ws = 0
    0x10020c27150a7d80, //      ; mov vpm, ra2 ; nop     // pm = 0, sf = 0, ws = 0
    0x10020c27150e7d80, //      ; mov vpm, ra3 ; nop     // pm = 0, sf = 0, ws = 0
    0x300009e7009e7000, // thrend       ; nop  ; nop
    0x100009e7009e7000, //      ; nop  ; nop
    0x100009e7009e7000, //      ; nop  ; nop
  };

   memcpy((void *)p, (void *)cs, sizeof(cs));

// Render control list
  p = list + offsets[BO_RCL];

  // Clear color
  addbyte(&p, 114);
  addword(&p, 0xffffffff); // Opaque White
  addword(&p, 0xffffffff); // 32 bit clear colours need to be repeated twice
  addword(&p, 0); // Z/vg
  addbyte(&p, 0); // stencil 

  // Tile Rendering Mode Configuration
  addbyte(&p, 113);
  addword(&p, bus_addr + offsets[BO_FB]); // framebuffer addresss
  addshort(&p, screen_x); // width
  addshort(&p, screen_y); // height
  addbyte(&p, 0x04); // framebuffer mode (linear rgba8888)
  addbyte(&p, 0x00);

  // Do a store of the first tile to force the tile buffer to be cleared
  // Tile Coordinates
  addbyte(&p, 115);
  addbyte(&p, 0);
  addbyte(&p, 0);

  // Store Tile Buffer General
  addbyte(&p, 28);
  addshort(&p, 0); // Store nothing (just clear)
  addword(&p, 0); // no address is needed

  // Link all binned lists together
  for(int x = 0; x < tile_x ; x++) {
    for(int y = 0; y < tile_y ; y++) {

      // Tile Coordinates
      addbyte(&p, 115);
      addbyte(&p, x);
      addbyte(&p, y);
      
      // Call Tile sublist
      addbyte(&p, 17);
      addword(&p, bus_addr + offsets[BO_TA] + ((y * tile_x) + x) * 32);

      // Last tile needs a special store instruction
      if((x == (tile_x - 1)) && (y == (tile_y - 1))) {
        // Store resolved tile color buffer and signal end of frame
        addbyte(&p, 25);
      } else {
        // Store resolved tile color buffer
        addbyte(&p, 24);
      }
    }
  }

  int render_length = p - (list + offsets[BO_RCL]);

// Run our control list
  printf("screen_x = %d, screen_y = %d\n", screen_x, screen_y);
  printf("tile_x   = %d, tile_y   = %d\n", tile_x, tile_y);
  printf("list     = 0x%08x\n", list);
  printf("\n");

// Clear all perf counter.
  v3d[V3D_PCTRC] = 0xffff;
// enable all perf counter.
  v3d[V3D_PCTRE] = 0xffff;
  assert((v3d[V3D_PCTRE] & 0xffff) == 0xffff);

  printf("Binner control list constructed\n");
  printf("Start Address: 0x%08x, length: 0x%x\n", bus_addr, length);

  v3d[V3D_CT0CS] = 0x30;
  while(v3d[V3D_CT0CS] & 0x20);

  v3d[V3D_L2CACTL] = 0x4;
  v3d[V3D_CT0CA] = bus_addr;
  v3d[V3D_CT0EA] = bus_addr + length;
  printf("V3D_CT0CS: 0x%08x, CA: 0x%08x\n", v3d[V3D_CT0CS], v3d[V3D_CT0CA]);
  printf("V3D_CT0CS: 0x%08x, EA: 0x%08x\n", v3d[V3D_CT0CS], v3d[V3D_CT0EA]);

  // Wait for control list to execute
  while(v3d[V3D_CT0CS] & 0x20);

  v3d[V3D_CT0CS] = 0x20;
  printf("V3D_CT0CS: 0x%08x, CA: 0x%08x\n", v3d[V3D_CT0CS], v3d[V3D_CT0CA]);
  printf("V3D_CT0CS: 0x%08x, EA: 0x%08x\n", v3d[V3D_CT0CS], v3d[V3D_CT0EA]);
  printf("\n");

  v3d[V3D_L2CACTL] = 0x4;

  printf("Render control list constructed\n");
  printf("Start Address: 0x%08x, length: 0x%x\n", bus_addr + 0xe200, render_length);

  v3d[V3D_CT1CS] = 0x30;
  while(v3d[V3D_CT1CS] & 0x20);

  v3d[V3D_L2CACTL] = 0x4;
  v3d[V3D_CT1CA] = bus_addr + 0xe200;
  v3d[V3D_CT1EA] = bus_addr + 0xe200 + render_length;

  printf("V3D_CT1CS: 0x%08x, CA: 0x%08x\n", v3d[V3D_CT1CS], v3d[V3D_CT1CA]);
  printf("V3D_CT1CS: 0x%08x, EA: 0x%08x\n", v3d[V3D_CT1CS], v3d[V3D_CT1EA]);

  while(v3d[V3D_CT1CS] & 0x20);

  v3d[V3D_CT1CS] = 0x20;
  printf("V3D_CT1CS: 0x%08x, CA: 0x%08x\n", v3d[V3D_CT1CS], v3d[V3D_CT1CA]);
  printf("V3D_CT1CS: 0x%08x, EA: 0x%08x\n", v3d[V3D_CT1CS], v3d[V3D_CT1EA]);
  printf("\n");

  v3d[V3D_L2CACTL] = 0x4;

// Dump tile allocation memory
   printf("Dump tile allocation memory\n");
   for (int x = 0; x < tile_x; x++) {
      for (int y = 0; y < tile_y; y++) {
          int *p = (int *)(list + offsets[BO_TA] + (((y * tile_x) + x) * 32));
          for (int z = 0; z < 32 / 4; z++) {
              printf("0x%08x ", *p); p++;
          }
          printf("\n");
      }
  }
  printf("\n");
  printf("V3D_PCTR9: %d\n", v3d[V3D_PCTR9]);
  printf("V3D_PCTR10: %d\n", v3d[V3D_PCTR10]);
  printf("V3D_PCTR14: %d\n", v3d[V3D_PCTR14]);
  printf("V3D_PCTR15: %d\n", v3d[V3D_PCTR15]);
  printf("\n");
  printf("V3D_DBGE: 0x%08x\n", v3d[V3D_DBGE]);
  printf("V3D_FDBGO: 0x%08x\n", v3d[V3D_FDBGO]);
  printf("V3D_FDBGB: 0x%08x\n", v3d[V3D_FDBGR]);
  printf("V3D_FDBGR: 0x%08x\n", v3d[V3D_FDBGB]);
  printf("V3D_FDBGS: 0x%08x\n", v3d[V3D_FDBGS]);
  printf("\n");
  printf("V3D_ERRSTAT: 0x%08x\n", v3d[V3D_ERRSTAT]);

// Disable perf counter
   v3d[V3D_PCTRE] = 0;

   SaveBMP("./frame.bmp", list + offsets[BO_FB]);
   printf("frame buffer memory dumpped to frame.data\n");

// Release resources
   unmapmem((void *) list, offsets[BO_TOTAL]);
   mem_unlock(mbox, handle);
   mem_free(mbox, handle);

   printf("done\n");
}

int main(int argc, char **argv) {
  mbox = mbox_open();

  // The blob now has this nice handy call which powers up the v3d pipeline.
  qpu_enable(mbox, 1);

  // map v3d's registers into our address space.
  v3d = (unsigned *) mapmem(0x3fc00000, 0x1000);

  printf("v3d = 0x%x\n", (int)v3d);
  printf("v3d[V3D_IDENT0] = 0x%x\n", v3d[V3D_IDENT0]);

  if(v3d[V3D_IDENT0] != 0x02443356) { // Magic number.
    printf("Error: V3D pipeline isn't powered up and accessable.\n");
    exit(-1);
  }

  // We now have access to the v3d registers, we should do something.
  testTriangle();

  unmapmem((void *) v3d, 0x1000);
  qpu_enable(mbox, 0);
  mbox_close(mbox);

  return 0;
}





