    lw  0   1   cat 
    sw  0   1   cat2
    add 1   1   1
    sw  0   1   cat
    lw  0   1   cat
    lw  0   2   cat2
    add 1   2   3
    sw  0   3   sum
    halt
cat     .fill   10
cat2    .fill   0   
sum     .fill   0
