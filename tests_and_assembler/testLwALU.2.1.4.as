        lw      0       1       cat
        lw      0       2       car
        lw      0       3       car2
        lw      0       4       cat2
        add     4       4       4
        nor     4       3       4
        nor     1       1       1
        nor     1       2       1
        add     2       1       2
        add     2       2       2
        add     2       2       4
        nor     4       4       4
        noop
        add     4       0       5
        add     1       0       5
        add     2       0       5     
        halt
car     .fill   1
car2    .fill   2
cat     .fill   3
cat2    .fill   4