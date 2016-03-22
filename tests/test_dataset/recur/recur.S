        addi $sp, $sp, -4   # i:0x270
        sw   $ra, 0($sp)    # push $ra
        lw   $a0, 0($0)     # $a0 = n
        jal  sumToN
        sw   $v0, 4($0)     # store ans, i:0x280
        lw   $ra, 0($sp)    # pop $ra
        addi $sp, $sp, 4
        lw   $v1, 4($0)
        halt
        halt
        halt
        halt
        halt
sumToN: slti $t0, $a0, 2    # whether in base cases, i:0x2A4
        beq  $t0, $0, recur
        or   $v0, $a0, $0
        jr   $ra
 recur: addi $sp, $sp, -8
        sw   $ra, 4($sp)    # push $ra
        sw   $a0, 0($sp)    # push old $a0
        addi $a0, $a0, -1   # new $a0
        jal  sumToN
        lw   $a0, 0($sp)    # pop old $a0, i:0x2C8
        lw   $ra, 4($sp)    # pop $ra
        addi $sp, $sp, 8
        add  $v0, $v0, $a0
        jr   $ra
