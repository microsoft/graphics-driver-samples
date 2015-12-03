loadsm ; mov  r0, vary   ; mov r3.8d, 1.0
       ; fadd r0, r0, r5 ; mov r1, vary
sbwait ; fadd r1, r1, r5 ; mov r2, vary 
       ; fadd r2, r2, r5 ; mov r3.8a, r0 
       ; nop             ; mov r3.8b, r1
       ; nop             ; mov r3.8c, r2
thrend ; mov tlb_c, r3   ; nop
       ; nop             ; nop
sbdone ; nop             ; nop
