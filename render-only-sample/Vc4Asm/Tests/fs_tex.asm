       ; mov r0, varying ; nop
       ; mov r1, varying ; nop
sbwait ; fadd r2, r0, r5 ; nop
       ; fadd r3, r1, r5 ; nop
       ; mov tmu0_t, r2  ; nop
       ; mov tmu0_s, r3  ; nop
ldtmu0 ; nop ; nop
thrend ; mov tlb_colour, r4 ; nop
       ; nop ; nop
sbdone ; nop ; nop
