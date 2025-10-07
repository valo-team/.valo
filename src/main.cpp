#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "memory.h"
#include "esp.h"
#include "aimbot.h"
#include "menu.h"
#include "recoil.h"

class ValorantCheat {
private:
    Memory memory;
    ESP esp;
    Aimbot aimbot;
    Menu menu;
    RecoilControl recoil;
    bool running;
    
public:
    ValorantCheat() : running(false) {}
    
    bool Initialize() {
        std::cout << "[+] Valorant External Cheat v1.0" << std::endl;
        std::cout << "[+] Initializing..." << std::endl;
        
        // Initialize memory manager
        if (!memory.Initialize("VALORANT-Win64-Shipping.exe")) {
            std::cout << "[-] Failed to find Valorant process!" << std::endl;
            return false;
        }
        
        std::cout << "[+] Process found!" << std::endl;
        
        // Initialize components
        esp.Initialize(&memory);
        aimbot.Initialize(&memory);
        recoil.Initialize(&memory);
        menu.Initialize();
        
        std::cout << "[+] All components initialized successfully!" << std::endl;
        return true;
    }
    
    void Run() {
        running = true;
        
        while (running) {
            // Update game data
            memory.UpdateGameData();
            
            // Run ESP if enabled
            if (menu.IsESPEnabled()) {
                esp.Render();
            }
            
            // Run aimbot if enabled
            if (menu.IsAimbotEnabled()) {
                aimbot.Update();
            }
            
            // Run recoil control if enabled
            if (menu.IsRecoilControlEnabled()) {
                recoil.Update();
            }
            
            // Check for exit key
            if (GetAsyncKeyState(VK_F4) & 0x8000) {
                running = false;
            }
            
            // Toggle menu
            if (GetAsyncKeyState(VK_INSERT) & 0x8000) {
                menu.Toggle();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    void Shutdown() {
        std::cout << "[+] Shutting down..." << std::endl;
        esp.Shutdown();
        aimbot.Shutdown();
        recoil.Shutdown();
        menu.Shutdown();
        memory.Shutdown();
    }
};

int main() {
    // Set console title
    SetConsoleTitle(L"Valorant External Cheat v1.0");
    
    ValorantCheat cheat;
    
    if (!cheat.Initialize()) {
        std::cout << "[-] Failed to initialize cheat!" << std::endl;
        std::cout << "[*] Press any key to exit..." << std::endl;
        std::cin.get();
        return -1;
    }
    
    std::cout << "[+] Cheat loaded successfully!" << std::endl;
    std::cout << "[*] Controls:" << std::endl;
    std::cout << "[*] INSERT - Toggle Menu" << std::endl;
    std::cout << "[*] F4 - Exit" << std::endl;
    std::cout << "[*] Starting main loop..." << std::endl;
    
    cheat.Run();
    cheat.Shutdown();
    
    return 0;
}