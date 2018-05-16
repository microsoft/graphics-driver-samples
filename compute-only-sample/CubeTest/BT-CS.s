
# Set up VPM to read XYZW RGB (6 floats)
ldi vr_setup, 0x00601a00

# Read VPM 6 times
# W is fixed at 1.0
mov ra0, vpm ; nop
mov ra1, vpm ; nop
mov ra2, vpm ; nop
mov ra3, vpm ; nop
mov ra4, vpm ; nop
mov ra5, vpm ; nop

# Screen space X has to be scaled by Width*16.0/2.0
mov r2, unif ; nop
mov r1, ra0  ; nop
nop ; fmul r0, r1, r2

ftoi ra8.16a, r0 ; nop

# Screen space Y has to be scaled by -Height*16.0/2.0
mov r2, unif ; nop
mov r1, ra1  ; nop
nop ; fmul r0, r1, r2

ftoi ra8.16b, r0 ; nop

# Screen space Z has to be scaled and offseted with the same setting in Clipper Z Scale and Offset

# Set up VPM to write shaded vertex for PTB
ldi vw_setup, 0x00001a00

# Xc, Yc, Zc, Wc
mov vpm, ra0
mov vpm, ra1
mov vpm, ra2
mov vpm, ra3

# Ys + Xs
mov vpm, ra8 ; nop

# Zs
mov vpm, ra2 ; nop

# 1/Wc
mov vpm, ra3 ; nop

# End of the CS
nop ; nop ; thrend
nop ; nop
nop ; nop

