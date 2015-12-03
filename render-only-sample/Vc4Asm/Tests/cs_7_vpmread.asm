ldi  ; mov vr_setup, 0x00701a00 ; nop
ldi  ; mov vw_setup, 0x00001a00 ; nop
     ; mov r0, vpm ; nop
     ; mov r1, vpm ; nop
     ; mov ra2, vpm ; nop
     ; mov ra3, vpm ; nop
     ; nop  ; fmul ra0, r0, uniform
     ; nop  ; fmul ra1, r1, uniform
     ; ftoi ra4.16a, ra0, ra0 ; nop
     ; ftoi ra4.16b, ra1, ra1 ; nop
     ; mov ra5, vpm ; nop
     ; mov ra6, vpm ; nop
     ; mov ra7, vpm ; nop
     ; mov vpm, r0 ; nop 
     ; mov vpm, r1 ; nop
     ; mov vpm, ra2 ; nop 
     ; mov vpm, ra3 ; nop
     ; mov vpm, ra4 ; nop
     ; mov vpm, ra2 ; nop
     ; mov vpm, ra3 ; nop
thrend ; nop  ; nop
     ; nop  ; nop
     ; nop  ; nop
