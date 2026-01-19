# Problem: we got starting address of the function now we want to decode its first instruction. steps to implement:

2. **Read bytes starting at that address** from process memory.
3. **Set CPU mode correctly** (x86 vs x64 matters).
4. **Feed bytes to an instruction decoder** (manual or mental model).
5. **Determine instruction length** from opcode + prefixes.
6. **Interpret operands using addressing mode + registers**.
7. **Verify by checking next instruction address** (address + length).


---
## Decoder

1. **Read prefixes** (lock, rep, segment, operand-size, address-size).
2. **Read primary opcode byte**.
3. **Check for opcode extensions** (0F escape, map selection).
4. **Determine instruction class** (ALU, control-flow, SIMD, etc.).
5. **Check if ModRM byte is required**.
6. **Decode ModRM** (mod / reg / r-m roles).
7. **If needed, decode SIB byte** (scale, index, base).
8. **Read displacement** (size depends on mod + addressing mode).
9. **Read immediate value** (size depends on opcode).
10. **Resolve operand sizes** (prefixes + mode decide this).
11. **Assemble mnemonic + operands**.
12. **Compute total instruction length**.
13. **Validate legality for current CPU mode**.
