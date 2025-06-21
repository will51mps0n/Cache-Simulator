        lw     0    1   cat
        sw     0    1   cat1
        lw     0    2   cat1
        add    1    2   3
        sw     0    3   twocat
loop    lw     0    4   twocat
        nor    4    4   4
        nor    4    4   4
        nor    4    4   4
        lw     0    6   one
        nor    1    1   1
        add    1    6   1  
        add    4    1   4
        sw     0    4   twocat
        beq    4    0   loop
        halt   
cat     .fill   10
cat1    .fill   20
one     .fill   1
twocat  .fill   0