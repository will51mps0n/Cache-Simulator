        lw      0       1       incrm
        lw      0       2       num
        lw      0       3       addr1
        sw      3       2       done
done    halt                           
addr1   .fill   16
incrm   .fill   5
num     .fill   -25