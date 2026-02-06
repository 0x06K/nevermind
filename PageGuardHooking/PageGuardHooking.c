#include <windows.h>
#include <stdio.h>

void TargetFunction(){
    printf("Listen this is Donald Trump. We are going to launch an operation against Anonymous :D.");
    return;
}
void HookFunction(){
    printf("Operation Hijacked. Donald Trump Don't Ever Think of it Again 0_0");
    return;
}
LONG CALLBACK Handler(PEXCEPTION_POINTERS info){
    if (info->ExceptionRecord->ExceptionCode != STATUS_GUARD_PAGE_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH;

    // 1) Decide if THIS is the instruction you care about
    void* rip = (void*)info->ContextRecord->Rip;

    // 2) Hijack control flow (optional)
    // if(rip == (void*)TargetFunction)
        info->ContextRecord->Rip = (DWORD64)HookFunction;
    
    // // 3) Re-arm PAGE_GUARD (Windows clears it after the fault)
    // DWORD old;
    // VirtualProtect(
    //     (LPVOID)((ULONG_PTR)rip & ~0xFFF), // page-align
    //     0x1000,
    //     PAGE_EXECUTE_READ | PAGE_GUARD,
    //     &old
    // );

    
    // 4) Resume execution
    
    return EXCEPTION_CONTINUE_EXECUTION;

}

int main() {

    // Register handlers
    AddVectoredExceptionHandler(1, Handler);
    
    DWORD old;
    LPVOID pageBase =
    (LPVOID)((ULONG_PTR)TargetFunction & ~(0x1000 - 1));

    VirtualProtect(pageBase, 0x1000, PAGE_EXECUTE_READ | PAGE_GUARD, &old);


    printf("[*] Triggering exception...\n");
    TargetFunction();

    return 0;
}
