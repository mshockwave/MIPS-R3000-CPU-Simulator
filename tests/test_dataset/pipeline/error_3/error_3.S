addi  $1, $0, 0x7FFF

sll   $1, $1, 16

ori   $1, $1, 0xFFFF #$1 = 7FFFFFFF

addi  $2, $0, 2      #$2 = 2

add   $3, $1, $2     #overflow

addu  $4, $1, $2

addi  $5, $1, 0xF    #overflow

addiu $6, $1, 0xF

bgtz  $2, 3          # $2 = 1 > 0, so

addi  $7, $0, 0xFFFF #Line 10~12 should not be executed

and   $1, $1, $2

andi  $2, $2, 0

bgtz  $3, 1          #$3 = 0x10000000 < 0, so line 14 will be executed

add   $0, $0, $2     #write to register 0 error

halt

halt

halt

halt

halt
