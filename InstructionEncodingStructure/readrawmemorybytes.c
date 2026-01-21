#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t code[16];
    memcpy(code, (void*)(main), 16);

    for(int i = 0; i < 16; i++)
        printf("%x ", code[i]);

    return 0;
}
