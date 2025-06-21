    lw  0   1   one
    lw  0   2   two
    lw  0   3   three
    sw  0   1   two2
    sw  0   3   two2
    sw  0   2   two2   
    lw  0   4   two2
    add 3   4   5
    sw  0   5   five
    halt
one   .fill   1
two    .fill   2 
three  .fill   3
two2    .fill 1000
five    .fill   0
