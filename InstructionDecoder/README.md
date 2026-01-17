# x86-64 Instruction Decoder
**Goal:** Understand instruction encoding by manually decoding machine code from live process memory.

**How:** Attach to a process, read bytes from a function, parse instruction components (prefixes, opcode, ModR/M, SIB, displacement, immediate) using bitwise operations. Implement ~20 common instructions to cover all encoding patterns.

**Output:** Address, raw bytes, decoded instruction, and length.

**Why:** Learn how instructions are structured as bytes - essential for calculating instruction boundaries when hooking functions.