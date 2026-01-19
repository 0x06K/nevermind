#include <stdio.h>
#include <stdint.h>
#include<strings.h>

static const char *reg32[8] = {
    "eax","ecx","edx","ebx","esp","ebp","esi","edi"
};

int decode_x86_32(const uint8_t *p, char *out) {
    int i = 0;

    /* ---- prefixes ---- */
    int operand16 = 0;
    while (1) {
        uint8_t b = p[i];
        if (b == 0x66) { operand16 = 1; i++; }
        else if (b == 0xF3 || b == 0xF2 ||
                 b == 0x2E || b == 0x36 ||
                 b == 0x3E || b == 0x26 ||
                 b == 0x64 || b == 0x65 ||
                 b == 0x67) i++;
        else break;
    }

    /* ---- opcode ---- */
    uint8_t op = p[i++];

    /* MOV r32, imm */
    if ((op & 0xF8) == 0xB8) {
        uint32_t imm = *(uint32_t *)(p + i);
        sprintf(out, "mov %s, 0x%x", reg32[op & 7], imm);
        return i + 4;
    }

    /* ADD r/m32, imm8 */
    if (op == 0x83) {
        uint8_t modrm = p[i++];
        int mod = modrm >> 6;
        int reg = (modrm >> 3) & 7;
        int rm  = modrm & 7;

        int disp = 0;
        if (mod == 1) disp = (int8_t)p[i++];
        else if (mod == 2 || (mod == 0 && rm == 5)) {
            disp = *(int32_t *)(p + i);
            i += 4;
        }

        int8_t imm = (int8_t)p[i++];

        if (reg == 0) { /* ADD */
            if (mod == 3)
                sprintf(out, "add %s, %d", reg32[rm], imm);
            else
                sprintf(out, "add [%s+%d], %d", reg32[rm], disp, imm);
            return i;
        }
    }

    /* NOP */
    if (op == 0x90) {
        sprintf(out, "nop");
        return i;
    }

    sprintf(out, "db 0x%02X", op);
    return i;
}

/* ---- demo ---- */
int main(void) {
    uint8_t code[16];
    memcpy(code, (void*)(main), 16);

    int ip = 0;
    char buf[64];

    while (ip < sizeof(code)) {
        int len = decode_x86_32(code + ip, buf);
        printf("%08X: %s\n", ip, buf);
        ip += len;
    }
    return 0;
}
