#include <windows.h>
#include <iostream>
#include <cstring>

// Example class with virtual functions
class Animal {
public:
    Animal() : {
        std::cout << "An instance of Animal class has been created." << std::endl;
    }
    virtual void Speak() {
        std::cout << "[Original] Generic animal sound" << std::endl;
    }
    
    virtual void Move() {
        std::cout << "[Original] Animal is moving" << std::endl;
    }
    
    virtual int GetLegs() {
        std::cout << "[Original] Returning legs count" << std::endl;
        return 4;
    }
};

// Original function pointers (to call original functions)
typedef void (*Speak_t)(Animal*);
typedef void (*Move_t)(Animal*);
typedef int (*GetLegs_t)(Animal*);

Speak_t original_Speak = nullptr;
Move_t original_Move = nullptr;
GetLegs_t original_GetLegs = nullptr;

// Our detour functions
void Hooked_Speak(Animal* thisptr) {
    std::cout << "[HOOKED] Speak() called!" << std::endl;
    std::cout << "[HOOKED] This pointer: " << thisptr << std::endl;
    
    // Call original
    if (original_Speak) {
        original_Speak(thisptr);
    }
    
    std::cout << "[HOOKED] Speak() finished!" << std::endl;
}

void Hooked_Move(Animal* thisptr) {
    std::cout << "[HOOKED] Move() intercepted!" << std::endl;
    
    // Call original
    if (original_Move) {
        original_Move(thisptr);
    }
}

int Hooked_GetLegs(Animal* thisptr) {
    std::cout << "[HOOKED] GetLegs() intercepted!" << std::endl;
    
    // Call original and modify return value
    int original_result = original_GetLegs ? original_GetLegs(thisptr) : 4;
    
    std::cout << "[HOOKED] Original returned: " << original_result << std::endl;
    std::cout << "[HOOKED] Returning modified value: 100" << std::endl;
    
    return 100; // Return fake value
}

// Helper function to calculate vtable size
size_t GetVTableSize(void** vtable) {
    size_t size = 0;
    MEMORY_BASIC_INFORMATION mbi;
    
    while (true) {
        // Query memory at vtable[size]
        if (VirtualQuery(vtable[size], &mbi, sizeof(mbi)) == 0) {
            break;
        }
        
        // Check if it points to executable memory
        if (mbi.Protect != PAGE_EXECUTE_READ && 
            mbi.Protect != PAGE_EXECUTE_READWRITE &&
            mbi.Protect != PAGE_EXECUTE_WRITECOPY &&
            mbi.Protect != PAGE_EXECUTE) {
            break;
        }
        
        // Check if it's a valid code pointer
        if (!vtable[size] || IsBadCodePtr((FARPROC)vtable[size])) {
            break;
        }
        
        size++;
        
        // Safety limit
        if (size > 100) break;
    }
    
    return size;
}

int main() {
    std::cout << "=== VTable Hooking Manual Implementation ===\n\n";
    
    // Step 1: Create an instance of the class
    Animal* animal = new Animal();
    Animal obj = *animal;
    std::cout <<  obj.GetLegs() << std::endl;
    std::cout << "Step 1: Created Animal instance at: " << animal << "\n\n";
    
    // Step 2: Get the vtable pointer from the instance
    // The first 8 bytes of any C++ object with virtual functions is a pointer to its vtable
    void** original_vtable = *(void***)animal;
    
    std::cout << "Step 2: Original VTable address: " << original_vtable << "\n";
    
    // Step 3: Calculate vtable size
    size_t vtable_size = GetVTableSize(original_vtable);
    std::cout << "Step 3: VTable size: " << vtable_size << " functions\n\n";
    
    // Step 4: Print original vtable
    std::cout << "Step 4: Original VTable contents:\n";
    for (size_t i = 0; i < vtable_size; i++) {
        std::cout << "  [" << i << "] = " << original_vtable[i] << "\n";
    }
    std::cout << "\n";
    
    // Step 5: Test original functions
    std::cout << "Step 5: Testing original functions:\n";
    animal->Speak();
    animal->Move();
    int legs = animal->GetLegs();
    std::cout << "GetLegs returned: " << legs << "\n\n";
    
    // Step 6: Create a copy of the vtable
    void** new_vtable = new void*[vtable_size];
    memcpy(new_vtable, original_vtable, vtable_size * sizeof(void*));
    
    std::cout << "Step 6: Created new VTable at: " << new_vtable << "\n";
    std::cout << "Copied " << vtable_size << " function pointers\n\n";
    
    // Step 7: Save original function pointers
    original_Speak = (Speak_t)original_vtable[0];
    original_Move = (Move_t)original_vtable[1];
    original_GetLegs = (GetLegs_t)original_vtable[2];
    
    std::cout << "Step 7: Saved original function pointers:\n";
    std::cout << "  Speak  : " << (void*)original_Speak << "\n";
    std::cout << "  Move   : " << (void*)original_Move << "\n";
    std::cout << "  GetLegs: " << (void*)original_GetLegs << "\n\n";
    
    // Step 8: Replace function pointers in new vtable with our hooks
    new_vtable[0] = (void*)&Hooked_Speak;
    new_vtable[1] = (void*)&Hooked_Move;
    new_vtable[2] = (void*)&Hooked_GetLegs;
    
    std::cout << "Step 8: Modified new VTable with hooks:\n";
    for (size_t i = 0; i < vtable_size; i++) {
        std::cout << "  [" << i << "] = " << new_vtable[i];
        if (new_vtable[i] != original_vtable[i]) {
            std::cout << " (HOOKED!)";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    
    // Step 9: Change memory protection to allow writing to the object
    DWORD old_protect;
    if (!VirtualProtect(animal, sizeof(void*), PAGE_READWRITE, &old_protect)) {
        std::cout << "Failed to change memory protection!\n";
        return 1;
    }
    
    std::cout << "Step 9: Changed memory protection to PAGE_READWRITE\n\n";
    
    // Step 10: Swap the vtable pointer in the instance
    std::cout << "Step 10: Swapping VTable pointer...\n";
    std::cout << "  Before: animal's vtable = " << *(void***)animal << "\n";
    
    *(void***)animal = new_vtable;
    
    std::cout << "  After : animal's vtable = " << *(void***)animal << "\n\n";
    
    // Step 11: Restore memory protection
    DWORD temp;
    VirtualProtect(animal, sizeof(void*), old_protect, &temp);
    
    std::cout << "Step 11: Restored memory protection\n\n";
    
    // Step 12: Test hooked functions
    std::cout << "=== Step 12: Testing Hooked Functions ===\n\n";
    
    std::cout << "Calling animal->Speak():\n";
    animal->Speak();
    std::cout << "\n";
    
    std::cout << "Calling animal->Move():\n";
    animal->Move();
    std::cout << "\n";
    
    std::cout << "Calling animal->GetLegs():\n";
    int hooked_legs = animal->GetLegs();
    std::cout << "GetLegs returned: " << hooked_legs << "\n\n";
    
    // Step 13: Restore original vtable
    std::cout << "=== Step 13: Restoring Original VTable ===\n";
    
    VirtualProtect(animal, sizeof(void*), PAGE_READWRITE, &old_protect);
    *(void***)animal = original_vtable;
    VirtualProtect(animal, sizeof(void*), old_protect, &temp);
    
    std::cout << "VTable restored to: " << *(void***)animal << "\n\n";
    
    // Step 14: Test restored functions
    std::cout << "Step 14: Testing restored functions:\n";
    animal->Speak();
    animal->Move();
    int restored_legs = animal->GetLegs();
    std::cout << "GetLegs returned: " << restored_legs << "\n\n";
    
    // Cleanup
    delete[] new_vtable;
    delete animal;
    
    std::cout << "=== Cleanup Complete ===\n";
    
    return 0;
}