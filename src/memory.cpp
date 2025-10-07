#include "memory.h"
#include <tlhelp32.h>
#include <iostream>

Memory::Memory() : processHandle(nullptr), processId(0), baseAddress(0) {}

Memory::~Memory() {
    Shutdown();
}

bool Memory::Initialize(const std::string& processName) {
    processId = GetProcessId(processName);
    if (processId == 0) {
        return false;
    }
    
    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (processHandle == nullptr) {
        return false;
    }
    
    baseAddress = GetModuleBaseAddress(processName);
    if (baseAddress == 0) {
        CloseHandle(processHandle);
        processHandle = nullptr;
        return false;
    }
    
    return true;
}

void Memory::Shutdown() {
    if (processHandle) {
        CloseHandle(processHandle);
        processHandle = nullptr;
    }
}

std::vector<BYTE> Memory::ReadBytes(uintptr_t address, size_t size) {
    std::vector<BYTE> buffer(size);
    ReadProcessMemory(processHandle, (LPVOID)address, buffer.data(), size, nullptr);
    return buffer;
}

bool Memory::WriteBytes(uintptr_t address, const std::vector<BYTE>& bytes) {
    return WriteProcessMemory(processHandle, (LPVOID)address, bytes.data(), bytes.size(), nullptr);
}

Entity Memory::GetLocalPlayer() {
    Entity localPlayer;
    
    uintptr_t localPlayerAddress = Read<uintptr_t>(baseAddress + LOCAL_PLAYER_OFFSET);
    if (localPlayerAddress == 0) {
        return localPlayer;
    }
    
    localPlayer.address = localPlayerAddress;
    localPlayer.position = Read<Vector3>(localPlayerAddress + 0x180); // Position offset
    localPlayer.health = Read<float>(localPlayerAddress + 0x140); // Health offset
    localPlayer.maxHealth = Read<float>(localPlayerAddress + 0x144); // Max health offset
    localPlayer.teamId = Read<int>(localPlayerAddress + 0x3C8); // Team ID offset
    localPlayer.isValid = true;
    
    return localPlayer;
}

std::vector<Entity> Memory::GetEntityList() {
    std::vector<Entity> entities;
    
    uintptr_t entityListAddress = Read<uintptr_t>(baseAddress + ENTITY_LIST_OFFSET);
    if (entityListAddress == 0) {
        return entities;
    }
    
    Entity localPlayer = GetLocalPlayer();
    
    for (int i = 0; i < 64; i++) { // Max players in Valorant
        uintptr_t entityAddress = Read<uintptr_t>(entityListAddress + (i * 0x8));
        if (!IsValidEntity(entityAddress)) {
            continue;
        }
        
        Entity entity;
        entity.address = entityAddress;
        entity.position = Read<Vector3>(entityAddress + 0x180);
        entity.headPosition = Vector3(entity.position.x, entity.position.y, entity.position.z + 70.0f);
        entity.health = Read<float>(entityAddress + 0x140);
        entity.maxHealth = Read<float>(entityAddress + 0x144);
        entity.teamId = Read<int>(entityAddress + 0x3C8);
        entity.isEnemy = (entity.teamId != localPlayer.teamId);
        entity.isValid = true;
        
        // Basic visibility check (simplified)
        entity.isVisible = true; // This would require more complex ray casting
        
        entities.push_back(entity);
    }
    
    return entities;
}

bool Memory::WorldToScreen(const Vector3& worldPos, Vector2& screenPos) {
    // This is a simplified world-to-screen conversion
    // In reality, you'd need to read the view matrix from the game
    
    float viewMatrix[16];
    ReadProcessMemory(processHandle, (LPVOID)(baseAddress + VIEW_MATRIX_OFFSET), &viewMatrix, sizeof(viewMatrix), nullptr);
    
    float screenW = (viewMatrix[12] * worldPos.x) + (viewMatrix[13] * worldPos.y) + (viewMatrix[14] * worldPos.z) + viewMatrix[15];
    
    if (screenW < 0.01f) {
        return false;
    }
    
    float screenX = (viewMatrix[0] * worldPos.x) + (viewMatrix[1] * worldPos.y) + (viewMatrix[2] * worldPos.z) + viewMatrix[3];
    float screenY = (viewMatrix[4] * worldPos.x) + (viewMatrix[5] * worldPos.y) + (viewMatrix[6] * worldPos.z) + viewMatrix[7];
    
    screenPos.x = (GetSystemMetrics(SM_CXSCREEN) / 2.0f) + (GetSystemMetrics(SM_CXSCREEN) / 2.0f) * screenX / screenW;
    screenPos.y = (GetSystemMetrics(SM_CYSCREEN) / 2.0f) - (GetSystemMetrics(SM_CYSCREEN) / 2.0f) * screenY / screenW;
    
    return true;
}

void Memory::UpdateGameData() {
    // This would be called each frame to update cached game data
    // For now, we'll just ensure the process is still valid
    if (processHandle) {
        DWORD exitCode;
        GetExitCodeProcess(processHandle, &exitCode);
        if (exitCode != STILL_ACTIVE) {
            Shutdown();
        }
    }
}

DWORD Memory::GetProcessId(const std::string& processName) {
    DWORD processId = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(processEntry);
        
        if (Process32First(snapshot, &processEntry)) {
            do {
                std::string currentProcessName(processEntry.szExeFile);
                if (currentProcessName == processName) {
                    processId = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &processEntry));
        }
        
        CloseHandle(snapshot);
    }
    
    return processId;
}

uintptr_t Memory::GetModuleBaseAddress(const std::string& moduleName) {
    uintptr_t baseAddress = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    
    if (snapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 moduleEntry;
        moduleEntry.dwSize = sizeof(moduleEntry);
        
        if (Module32First(snapshot, &moduleEntry)) {
            do {
                std::string currentModuleName(moduleEntry.szModule);
                if (currentModuleName == moduleName) {
                    baseAddress = (uintptr_t)moduleEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(snapshot, &moduleEntry));
        }
        
        CloseHandle(snapshot);
    }
    
    return baseAddress;
}

bool Memory::IsValidEntity(uintptr_t entityAddress) {
    if (entityAddress == 0) {
        return false;
    }
    
    // Basic validation - check if the entity has valid health
    float health = Read<float>(entityAddress + 0x140);
    return (health > 0 && health <= 150); // Valorant max health range
}