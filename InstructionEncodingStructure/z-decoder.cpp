#include <Zydis/Zydis.h>
#include <cstdint>
#include <cstring>
#include <cstdio>

using namespace std;

#define BYTES 150

int main() {
    uint8_t buffer[BYTES];
    std::memcpy(buffer, reinterpret_cast<void*>(main), BYTES);

    ZydisDecoder decoder;
    ZydisDecoderContext ctx;
    ZydisDecodedInstruction instr;
    ZydisDecodedOperand ops[ZYDIS_MAX_OPERAND_COUNT];

    ZydisFormatter formatter;
    char text[256];

    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    size_t offset = 0;
    ZyanU64 runtime_addr = reinterpret_cast<ZyanU64>(main);

    while (offset < BYTES) {
        if (!ZYAN_SUCCESS(ZydisDecoderDecodeInstruction(
                &decoder, &ctx, buffer + offset, BYTES - offset, &instr)))
            break;

        ZydisDecoderDecodeOperands(
            &decoder, &ctx, &instr, ops, instr.operand_count);

        ZydisFormatterFormatInstruction(
            &formatter, &instr, ops, instr.operand_count,
            text, sizeof(text), runtime_addr + offset, ZYAN_NULL);

        puts(text);
        offset += instr.length;   // THIS is the missing brain cell
    }

}
