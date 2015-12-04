            mov r0, vary    ; nop
            fadd r0, r0, r5 ; mov r1, vary
sbwait    ; fadd r1, r1, r5 ; nop     
# Write texture coordinates, S must be the last            
            mov t0t, r0     ; nop
            mov t0s, r1     ; nop
            ldtmu0          ; nop
            mov r0, r4.8a   ; nop
            mov r1, r4.8b   ; fmul r0.8c, r0, unif
            mov r2, r4.8c   ; fmul r0.8b, r1, unif
            nop             ; fmul r0.8a, r2, unif
            nop             ; mov r0.8d, 1.0
            mov tlbz, rb15  ; nop
thrend    ; mov tlbc, r0    ; nop
            nop             ; nop
sbdone    ; nop             ; nop