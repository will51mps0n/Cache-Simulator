        add     0       0       0
        lw      0       1       cat
        sw      0       0       cat
        lw      0       2       cat
        add     1       2       3
        add     3       3       3
        sw      0       3       cat
        lw      0       3       cat
        nor     3       3       1
        add     1       3       3
        sw      0       3       cat
        add     3       3       3
        halt
cat     .fill   100       
