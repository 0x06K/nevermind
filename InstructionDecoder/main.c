#include <stdio.h>

int main() {
   unsigned char byte1 = 0x48;  // REX.W prefix
    unsigned char byte2 = 0x89;  // Opcode
    unsigned char byte3 = 0xC8;  // ModR/M

    // Parse REX prefix (0x48)
    // Format: 0100WRXB
    // 0100 = REX prefix marker
    // W=1 (64-bit operand), R=0, X=0, B=0
    int rex_w = (byte1 >> 3) & 1;  // Extract W bit
    printf("REX.W=%d -> 64-bit operation\n", rex_w);

    // Parse opcode (0x89)
    // From Intel manual: 89 = MOV r/m, r (direction: reg->r/m)
    printf("Opcode 0x89 = MOV\n");

    // Parse ModR/M (0xC8)
    // Format: MMRRRAAA (2 bits Mod, 3 bits Reg, 3 bits R/M)
    int mod = (byte3 >> 6) & 0x03;     // Bits 7-6
    int reg = (byte3 >> 3) & 0x07;     // Bits 5-3  
    int rm  = byte3 & 0x07;             // Bits 2-0

    printf("ModR/M: mod=%d reg=%d rm=%d\n", mod, reg, rm);
    // mod=11 (register-to-register)
    // reg=001 (RCX - source register)
    // rm=000 (RAX - destination register)

    printf("Result: MOV RAX, RCX\n");
    return 0;
}