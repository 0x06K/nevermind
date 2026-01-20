#include <stdio.h>
#include <stdint.h>
#include<strings.h>

static const char *reg32[8] = {
    "eax","ecx","edx","ebx","esp","ebp","esi","edi"
};

int decode_steps_32(const unsigned char *p) {
    int i = 0;

    /* 1) PREFIXES */
    while (
        p[i] == 0xF0 || p[i] == 0xF2 || p[i] == 0xF3 ||   // lock / rep
        p[i] == 0x2E || p[i] == 0x36 || p[i] == 0x3E ||   // segment
        p[i] == 0x26 || p[i] == 0x64 || p[i] == 0x65 ||
        p[i] == 0x66 || p[i] == 0x67                    // size overrides
    ) i++;

    /* 2) OPCODE */
    unsigned char opcode = p[i++];
    if (opcode == 0x0F) opcode = p[i++]; // two-byte opcode escape

    /* 3) MODRM */
    int has_modrm = (opcode < 0xA0 || opcode > 0xA3);
    if (has_modrm) {
        unsigned char modrm = p[i++];
        int mod = modrm >> 6;
        int rm  = modrm & 7;

        /* 4) SIB */
        if (mod != 3 && rm == 4)
            i++; // SIB byte

        /* 5) DISPLACEMENT */
        if (mod == 1) i += 1;
        else if (mod == 2 || (mod == 0 && rm == 5))
            i += 4;
    }

    /* 6) IMMEDIATE (example cases) */
    if (opcode == 0x6A || opcode == 0x83) i += 1;
    if (opcode == 0x68 || opcode == 0xB8) i += 4;

    return i; // total instruction length
}

int main(void) {
    uint8_t code[16];
    memcpy(code, (void*)(main), 16);

    for(int i = 0; i < 16; i++)
        printf("%x ", code[i]);
    // int ip = 0;
    // char buf[64];

    // while (ip < sizeof(code)) {
    //     int len = decode_x86_32(code + ip, buf);
    //     printf("%08X: %s\n", ip, buf);
    //     ip += len;
    // }
    return 0;
}
