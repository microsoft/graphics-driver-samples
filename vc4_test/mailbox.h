/*
Copyright (c) 2012, Broadcom Europe Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef C_PLUS_PLUS
extern "C" {
#endif

#include <linux/ioctl.h>

#define MAJOR_NUM 100
#define IOCTL_MBOX_PROPERTY _IOWR(MAJOR_NUM, 0, char *)
#define DEVICE_FILE_NAME "/dev/vcio"

int mbox_open();
void mbox_close(int file_desc);

unsigned int get_version(int file_desc);
unsigned int mem_alloc(int file_desc, unsigned int size, unsigned int align, unsigned int flags);
unsigned int mem_free(int file_desc, unsigned int handle);
unsigned int mem_lock(int file_desc, unsigned int handle);
unsigned int mem_unlock(int file_desc, unsigned int handle);
void *mapmem(unsigned int base, unsigned int size);
void *unmapmem(void *addr, unsigned int size);

unsigned int execute_code(int file_desc, unsigned int code, unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3, unsigned int r4, unsigned int r5);
unsigned int execute_qpu(int file_desc, unsigned int num_qpus, unsigned int control, unsigned int noflush, unsigned int timeout);
unsigned int qpu_enable(int file_desc, unsigned int enable);


// Flags for allocate memory.
enum {
MEM_FLAG_DISCARDABLE = 1 << 0, /* can be resized to 0 at any time. Use for cached data */
MEM_FLAG_NORMAL = 0 << 2, /* normal allocating alias. Don't use from ARM */
MEM_FLAG_DIRECT = 1 << 2, /* 0xC alias uncached */
MEM_FLAG_COHERENT = 2 << 2, /* 0x8 alias. Non-allocating in L2 but coherent */
MEM_FLAG_L1_NONALLOCATING = (MEM_FLAG_DIRECT | MEM_FLAG_COHERENT), /* Allocating in L2 */
MEM_FLAG_ZERO = 1 << 4,  /* initialise buffer to all zeros */
MEM_FLAG_NO_INIT = 1 << 5, /* don't initialise (default is initialise to all ones */
MEM_FLAG_HINT_PERMALOCK = 1 << 6, /* Likely to be locked for long periods of time. */
};

#ifdef C_PLUS_PLUS
}
#endif
