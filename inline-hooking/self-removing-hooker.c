#include <windows.h>
#include <stdio.h>
#include <Zydis/Zydis.h>

typedef int (WINAPI* MessageBoxA_t)(HWND, LPCSTR, LPCSTR, UINT);

// Globals for unhooking
void* g_pMessageBoxA = NULL;
uint8_t g_original_bytes[32];
size_t g_hook_length = 0;

int WINAPI callmyname(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    printf("\n=== DETOUR CALLED ===\n");    
    // Restore original bytes
    printf("Bytes before removing hook: ");
    for (int i = 0; i < g_hook_length; i++) {
        printf("%02X ", ((uint8_t*)g_pMessageBoxA)[i]);
    }
    DWORD old;
    VirtualProtect(g_pMessageBoxA, g_hook_length, PAGE_EXECUTE_READWRITE, &old);
    memcpy(g_pMessageBoxA, g_original_bytes, g_hook_length);
    VirtualProtect(g_pMessageBoxA, g_hook_length, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_pMessageBoxA, g_hook_length);
    
    printf("\nHook removed. Calling original MessageBoxA at %p\n", g_pMessageBoxA);
    printf("Bytes after removing hook: ");
    for (int i = 0; i < g_hook_length; i++) {
        printf("%02X ", ((uint8_t*)g_pMessageBoxA)[i]);
    }
    // Call original (now unhooked)
    MessageBoxA_t original = (MessageBoxA_t)g_pMessageBoxA;
    return original(hWnd, lpText, lpCaption, uType);
}

int main() {
    ZydisDecoder d;
    ZydisDecoderInit(&d, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
    
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    g_pMessageBoxA = GetProcAddress(hUser32, "MessageBoxA");
    
    printf("=== HOOK SETUP ===\n");
    printf("MessageBoxA address: %p\n", g_pMessageBoxA);
    
    uint8_t* ip = (uint8_t*)g_pMessageBoxA;
    printf("First 2 bytes at target: %02X %02X\n", ip[0], ip[1]);
    
    size_t len = 0;
    
    // Decode until we have >= 14 bytes
    printf("\nDecoding instructions:\n");
    while (len < 14) {
        ZydisDecodedInstruction ins;
        ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
        
        if (ZYAN_FAILED(ZydisDecoderDecodeFull(&d, ip + len, 15, &ins, operands))) {
            printf("Decode failed\n");
            return 1;
        }
        
        printf("  Instruction at offset %zu: %u bytes\n", len, ins.length);
        len += ins.length;
    }
    
    g_hook_length = len;
    printf("Total hook length: %zu bytes\n", len);
    
    // Save original bytes
    memcpy(g_original_bytes, g_pMessageBoxA, len);
    printf("\nStolen bytes from MessageBoxA:\n  ");
    for (int i = 0; i < len; i++) {
        printf("%02X ", g_original_bytes[i]);
    }
    printf("\n");
    
    // Build hook
    uint8_t hook[15];
    hook[0] = 0xFF;
    hook[1] = 0x25;
    *(int32_t*)(hook + 2) = 0;
    *(uint64_t*)(hook + 6) = (uint64_t)callmyname;
    
    printf("\nHook JMP bytes:\n  ");
    for (int i = 0; i < 14; i++) {
        printf("%02X ", hook[i]);
    }
    printf("\n");
    printf("JMP target (detour): %p\n", callmyname);
    
    for(int i = 14; i < len; i++){
        hook[i] = 0x90;
    }
    
    if (len > 14) {
        printf("NOP padding: %zu bytes\n", len - 14);
    }
    
    // Install hook
    printf("\nInstalling hook...\n");
    DWORD old;
    VirtualProtect(g_pMessageBoxA, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy(g_pMessageBoxA, hook, len);
    VirtualProtect(g_pMessageBoxA, len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_pMessageBoxA, len);
    
    printf("First 2 bytes after hook: %02X %02X\n", ip[0], ip[1]);
    printf("=== HOOK INSTALLED ===\n\n");
    
    // Test - hook will fire once and remove itself
    printf("Calling MessageBoxA\n");
    MessageBoxA(NULL, "Try To Hack ME", "Hooked Function", MB_OK);
    
    return 0;
}