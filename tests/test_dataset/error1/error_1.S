addi $0 $0 10
addi $1 $0 1
lw $2 0($0)
add $2 $1 $2
lw $3 4($0)
lw $4 8($0)
add $4 $4 $3
sw $3 1022($0)
halt
halt
halt
halt
halt

