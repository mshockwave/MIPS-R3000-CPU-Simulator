addi $1, $0 -1
addi $2, $0, 0x7FFF
sub  $3, $2, $1
add  $4, $1, $2
sra  $1, $1, 1
srl  $1, $1, 2
sra  $1, $1, 3
srl  $1, $1, 4
sll  $2, $2, 1
and  $4, $1, $2
ori  $4, $4, 1
xor  $4, $3, $4
nor  $4, $4, $4
nand $4, $4, $4
lui  $1, 0xFFFF
ori  $1, $1, 0xFFFF
slt  $5, $4, $1
slti $5, $4, -0x8000
andi $4, $4, 0x7FFF
or   $5, $2, $3
halt
halt
halt
halt
halt
