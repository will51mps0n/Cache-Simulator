    lw     0    2   i
    lw     0    3   j
    lw     0    1   one
    lw     0    4   three
    lw     0    5   three
    lw     0    6   total
outLp     noop
    beq    2    4   endOut
    add    0    3    3
inLp    noop
    beq    3    5   endIn
    add    6    2    6
    add    6    3    6
    add    3    1    3
    beq    0    0   inLp
endIn   noop
    add    2    1    2
    beq    0    0   outLp
endOut  noop
    sw     0    6   total
    halt
i       .fill  0
j       .fill  0
one     .fill  1
three   .fill  3
total   .fill  0
