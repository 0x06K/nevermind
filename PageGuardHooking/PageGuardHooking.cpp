#include <windows.h>
#include <stdio.h>

LONG CALLBACK GuardHandler(PEXCEPTION_POINTERS info) {
    if (info->ExceptionRecord->ExceptionCode
        == STATUS_GUARD_PAGE_VIOLATION) {

        puts("[+] Guard page hit");

#ifdef _WIN64
        printf("RIP = %p\n",
            (void*)info->ContextRecord->Rip);
#else
        printf("EIP = %p\n",
            (void*)info->ContextRecord->Eip);
#endif

        DWORD old;
        VirtualProtect(
            info->ExceptionRecord->ExceptionAddress,
            0x1000,
            PAGE_READWRITE | PAGE_GUARD,
            &old
        );

        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

int main() {
    AddVectoredExceptionHandler(1, GuardHandler);

    void* page = VirtualAlloc(
        NULL, 0x1000,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    DWORD old;
    VirtualProtect(
        page, 0x1000,
        PAGE_READWRITE | PAGE_GUARD,
        &old
    );

    *(int*)page = 1337; 
    *(int*)page = 1338;
    getchar();
    return 0;
}
