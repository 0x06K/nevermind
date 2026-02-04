#include <iostream>
#include <string>
using namespace std;

class Player {
private:
    string name;
    int health;
    int score;
    
public:
    Player(string n) : name(n), health(100), score(0) {}
    
    virtual void takeDamage(int damage) {
        health -= damage;
        cout << name << " took " << damage << " damage! Health: " << health << endl;
        if (health <= 0) {
            cout << name << " has died!" << endl;
        }
    }
    
    virtual void addScore(int points) {
        score += points;
        cout << name << " gained " << points << " points! Score: " << score << endl;
    }
    
    virtual void displayStats() {
        cout << "=== " << name << " ===" << endl;
        cout << "Health: " << health << " | Score: " << score << endl;
    }
    
    virtual ~Player() {
        cout << "Player " << name << " destroyed" << endl;
    }
};

// Hook function types
typedef void (*TakeDamageFunc)(Player*, int);
typedef void (*AddScoreFunc)(Player*, int);
typedef void (*DisplayStatsFunc)(Player*);

// Original function pointers
TakeDamageFunc g_original_takeDamage = nullptr;
AddScoreFunc g_original_addScore = nullptr;
DisplayStatsFunc g_original_displayStats = nullptr;
void** g_originalVTable = nullptr;

// Hook functions
void hooked_takeDamage(Player* player, int damage) {
    cout << "[HOOK] Damage blocked: " << damage << " (God mode active!)" << endl;
}

void hooked_addScore(Player* player, int points) {
    cout << "[HOOK] Score multiplier active: " << points << " -> " << (points * 100) << endl;
    g_original_addScore(player, points * 100);
}

void hooked_displayStats(Player* player) {
    cout << "[HOOK] === HOOKED STATS ===" << endl;
    g_original_displayStats(player);
    cout << "[HOOK] (Cheats enabled)" << endl;
}

void InstallVTableHook(Player* player) {
    // Get original vtable
    g_originalVTable = *(void***)player;
    
    // Save original functions
    g_original_takeDamage = (TakeDamageFunc)g_originalVTable[0];
    g_original_addScore = (AddScoreFunc)g_originalVTable[1];
    g_original_displayStats = (DisplayStatsFunc)g_originalVTable[2];
    
    // Create new vtable (4 entries: 3 functions + destructor)
    void** newVTable = new void*[4];
    newVTable[0] = (void*)&hooked_takeDamage;
    newVTable[1] = (void*)&hooked_addScore;
    newVTable[2] = (void*)&hooked_displayStats;
    newVTable[3] = g_originalVTable[3];  // Keep destructor
    
    // Replace vtable pointer
    *(void***)player = newVTable;
    
    cout << "[INSTALL] VTable hook installed successfully!" << endl;
}

void RemoveVTableHook(Player* player) {
    // Restore original vtable
    *(void***)player = g_originalVTable;
    cout << "[REMOVE] VTable hook removed" << endl;
}

int main() {
    cout << "=== VTable Hooking Demo ===" << endl << endl;
    
    Player* player = new Player("Hero");
    
    cout << "--- Before Hook ---" << endl;
    player->displayStats();
    player->takeDamage(30);
    player->addScore(100);
    
    cout << "\n--- Installing Hook ---" << endl;
    InstallVTableHook(player);
    
    cout << "\n--- After Hook (God Mode) ---" << endl;
    player->displayStats();
    player->takeDamage(50);   // Should be blocked
    player->takeDamage(100);  // Should be blocked
    player->addScore(10);     // Should be multiplied by 100
    
    cout << "\n--- Removing Hook ---" << endl;
    RemoveVTableHook(player);
    
    cout << "\n--- After Removing Hook ---" << endl;
    player->takeDamage(20);  // Should work normally
    
    delete player;
    return 0;
}