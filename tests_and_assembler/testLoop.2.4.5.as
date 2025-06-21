        lw     0    1   cat
        sw     0    1   cat1
        lw     0    2   cat1
        add    1    2   3
        sw     0    3   twocat 
        halt   
cat     .fill   10
cat1    .fill   20
twocat  .fill   0