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

// Defines for v3d register offsets
#define V3D_IDENT0  (0x000>>2) // V3D Identification 0 (V3D block identity)
#define V3D_IDENT1  (0x004>>2) // V3D Identification 1 (V3D Configuration A)
#define V3D_IDENT2  (0x008>>2) // V3D Identification 1 (V3D Configuration B)

#define V3D_SCRATCH (0x010>>2) // Scratch Register

#define V3D_L2CACTL (0x020>>2) // 2 Cache Control
#define V3D_SLCACTL (0x024>>2) // Slices Cache Control

#define V3D_INTCTL  (0x030>>2) // Interrupt Control
#define V3D_INTENA  (0x034>>2) // Interrupt Enables
#define V3D_INTDIS  (0x038>>2) // Interrupt Disables

#define V3D_CT0CS   (0x100>>2) // Control List Executor Thread 0 Control and Status.
#define V3D_CT1CS   (0x104>>2) // Control List Executor Thread 1 Control and Status.
#define V3D_CT0EA   (0x108>>2) // Control List Executor Thread 0 End Address.
#define V3D_CT1EA   (0x10c>>2) // Control List Executor Thread 1 End Address.
#define V3D_CT0CA   (0x110>>2) // Control List Executor Thread 0 Current Address.
#define V3D_CT1CA   (0x114>>2) // Control List Executor Thread 1 Current Address.
#define V3D_CT00RA0 (0x118>>2) // Control List Executor Thread 0 Return Address.
#define V3D_CT01RA0 (0x11c>>2) // Control List Executor Thread 1 Return Address.
#define V3D_CT0LC   (0x120>>2) // Control List Executor Thread 0 List Counter
#define V3D_CT1LC   (0x124>>2) // Control List Executor Thread 1 List Counter
#define V3D_CT0PC   (0x128>>2) // Control List Executor Thread 0 Primitive List Counter
#define V3D_CT1PC   (0x12c>>2) // Control List Executor Thread 1 Primitive List Counter

#define V3D_PCS     (0x130>>2) // V3D Pipeline Control and Status
#define V3D_BFC     (0x134>>2) // Binning Mode Flush Count
#define V3D_RFC     (0x138>>2) // Rendering Mode Frame Count

#define V3D_BPCA    (0x300>>2) // Current Address of Binning Memory Pool
#define V3D_BPCS    (0x304>>2) // Remaining Size of Binning Memory Pool
#define V3D_BPOA    (0x308>>2) // Address of Overspill Binning Memory Block
#define V3D_BPOS    (0x30c>>2) // Size of Overspill Binning Memory Block
#define V3D_BXCF    (0x310>>2) // Binner Debug

#define V3D_SQRSV0  (0x410>>2) // Reserve QPUs 0-7
#define V3D_SQRSV1  (0x414>>2) // Reserve QPUs 8-15
#define V3D_SQCNTL  (0x418>>2) // QPU Scheduler Control

#define V3D_SRQPC   (0x430>>2) // QPU User Program Request Program Address
#define V3D_SRQUA   (0x434>>2) // QPU User Program Request Uniforms Address
#define V3D_SRQUL   (0x438>>2) // QPU User Program Request Uniforms Length
#define V3D_SRQCS   (0x43c>>2) // QPU User Program Request Control and Status

#define V3D_VPACNTL (0x500>>2) // VPM Allocator Control
#define V3D_VPMBASE (0x504>>2) // VPM base (user) memory reservation

#define V3D_PCTRC   (0x670>>2) // Performance Counter Clear
#define V3D_PCTRE   (0x674>>2) // Performance Counter Enables

#define V3D_PCTR0   (0x680>>2) // Performance Counter Count 0
#define V3D_PCTRS0  (0x684>>2) // Performance Counter Mapping 0
#define V3D_PCTR1   (0x688>>2) // Performance Counter Count 1
#define V3D_PCTRS1  (0x68c>>2) // Performance Counter Mapping 1
#define V3D_PCTR2   (0x690>>2) // Performance Counter Count 2
#define V3D_PCTRS2  (0x694>>2) // Performance Counter Mapping 2
#define V3D_PCTR3   (0x698>>2) // Performance Counter Count 3
#define V3D_PCTRS3  (0x69c>>2) // Performance Counter Mapping 3
#define V3D_PCTR4   (0x6a0>>2) // Performance Counter Count 4
#define V3D_PCTRS4  (0x6a4>>2) // Performance Counter Mapping 4
#define V3D_PCTR5   (0x6a8>>2) // Performance Counter Count 5
#define V3D_PCTRS5  (0x6ac>>2) // Performance Counter Mapping 5
#define V3D_PCTR6   (0x6b0>>2) // Performance Counter Count 6
#define V3D_PCTRS6  (0x6b4>>2) // Performance Counter Mapping 6
#define V3D_PCTR7   (0x6b8>>2) // Performance Counter Count 7
#define V3D_PCTRS7  (0x6bc>>2) // Performance Counter Mapping 7 
#define V3D_PCTR8   (0x6c0>>2) // Performance Counter Count 8
#define V3D_PCTRS8  (0x6c4>>2) // Performance Counter Mapping 8
#define V3D_PCTR9   (0x6c8>>2) // Performance Counter Count 9
#define V3D_PCTRS9  (0x6cc>>2) // Performance Counter Mapping 9
#define V3D_PCTR10  (0x6d0>>2) // Performance Counter Count 10
#define V3D_PCTRS10 (0x6d4>>2) // Performance Counter Mapping 10
#define V3D_PCTR11  (0x6d8>>2) // Performance Counter Count 11
#define V3D_PCTRS11 (0x6dc>>2) // Performance Counter Mapping 11
#define V3D_PCTR12  (0x6e0>>2) // Performance Counter Count 12
#define V3D_PCTRS12 (0x6e4>>2) // Performance Counter Mapping 12
#define V3D_PCTR13  (0x6e8>>2) // Performance Counter Count 13
#define V3D_PCTRS13 (0x6ec>>2) // Performance Counter Mapping 13
#define V3D_PCTR14  (0x6f0>>2) // Performance Counter Count 14
#define V3D_PCTRS14 (0x6f4>>2) // Performance Counter Mapping 14
#define V3D_PCTR15  (0x6f8>>2) // Performance Counter Count 15
#define V3D_PCTRS15 (0x6fc>>2) // Performance Counter Mapping 15
#define V3D_PCTR16  (0x700>>2) // Performance Counter Count 16
#define V3D_PCTRS16 (0x704>>2) // Performance Counter Mapping 16

#define V3D_DBGE    (0xf00>>2) // PSE Error Signals
#define V3D_FDBGO   (0xf04>>2) // FEP Overrun Error Signals
#define V3D_FDBGB   (0xf08>>2) // FEP Interface Ready and Stall Signals, FEP Busy Signals
#define V3D_FDBGR   (0xf0c>>2) // FEP Internal Ready Signals
#define V3D_FDBGS   (0xf10>>2) // FEP Internal Stall Input Signals

#define V3D_ERRSTAT (0xf20>>2) // Miscellaneous Error Signals (VPM, VDW, VCD, VCM, L2C)
