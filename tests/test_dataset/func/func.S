        addi $sp, $sp, -4
        sw   $ra, 0($sp)   # push $ra
        lw   $a0, 0($0)    # $a0 = a
        lw   $a1, 4($0)    # $a1 = b
        jal  sum
        sw   $v0, 8($0)    # store s = a + b
        lw   $ra, 0($sp)   # pop $ra
        addi $sp, $sp, 4
        lw   $v1, 8($0)
        halt
        halt
        halt
        halt
        halt
   sum: add  $v0, $a0, $a1
        jr   $ra
