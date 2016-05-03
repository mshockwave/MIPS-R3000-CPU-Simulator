          lw $2, 0($3)
          or $3, $1, $4
         beq $2, $3, anyway
         and $1, $1, $0
anyway: halt
        halt
        halt
        halt
        halt