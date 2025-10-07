#include "recoil.h"
#include <iostream>
#include <chrono>

RecoilControl::RecoilControl() : memory(nullptr), isShooting(false), shotsFired(0), 
                                 lastShotTime(0), currentWeapon(0), totalRecoil(0, 0) {}

RecoilControl::~RecoilControl() {
    Shutdown();
}

bool RecoilControl::Initialize(Memory* mem) {
    memory = mem;
    
    InitializeRecoilPatterns();
    ResetRecoilState();
    
    std::cout << "[+] Recoil Control initialized successfully!" << std::endl;
    return true;
}

void RecoilControl::Shutdown() {
    ResetRecoilState();
}

void RecoilControl::Update() {
    if (!memory || !settings.enabled) {
        return;
    }
    
    UpdateShootingState();
    
    if (isShooting) {
        ControlRecoil();
    }
}

void RecoilControl::ControlRecoil() {
    currentWeapon = GetCurrentWeapon();
    float weaponMultiplier = GetWeaponMultiplier(currentWeapon);
    
    // Get recoil offset for current shot
    Vector2 recoilOffset = GetRecoilOffset(shotsFired, currentWeapon);
    
    // Apply weapon and settings multipliers
    recoilOffset.x *= weaponMultiplier * settings.strength;
    recoilOffset.y *= weaponMultiplier * settings.strength;
    
    // Apply smoothing
    recoilOffset.x /= settings.smoothness;
    recoilOffset.y /= settings.smoothness;
    
    // Apply compensation
    ApplyRecoilCompensation(recoilOffset);
    
    // Update total recoil for crosshair
    totalRecoil.x += recoilOffset.x;
    totalRecoil.y += recoilOffset.y;
}

Vector2 RecoilControl::GetRecoilOffset(int shotNumber, int weaponId) {
    WeaponRecoilPattern* pattern = GetWeaponPattern(weaponId);
    if (!pattern || shotNumber >= (int)pattern->pattern.size()) {
        // Use last known pattern point or default
        if (pattern && !pattern->pattern.empty()) {
            return pattern->pattern.back();
        }
        return Vector2(0, 2.0f); // Default upward recoil
    }
    
    return pattern->pattern[shotNumber];
}

void RecoilControl::ApplyRecoilCompensation(const Vector2& offset) {
    // Convert recoil offset to mouse movement
    // Negative values because we want to counter the recoil
    float mouseX = -offset.x * 2.0f; // Horizontal sensitivity
    float mouseY = -offset.y * 1.5f; // Vertical sensitivity
    
    MoveMouse(mouseX, mouseY);
}

int RecoilControl::GetCurrentWeapon() {
    if (!memory) return 0;
    
    // This would read the current weapon ID from game memory
    // For now, return a default weapon ID (Vandal)
    return 1; // Vandal
}

float RecoilControl::GetWeaponMultiplier(int weaponId) {
    if (!settings.adaptiveRecoil) {
        return 1.0f;
    }
    
    switch (weaponId) {
        case 1: return settings.vandalMultiplier;    // Vandal
        case 2: return settings.phantomMultiplier;   // Phantom
        case 3: return settings.spectreMultiplier;   // Spectre
        case 4: return settings.operatorMultiplier;  // Operator
        default: return 1.0f;
    }
}

bool RecoilControl::IsPlayerShooting() {
    // Check if fire key is pressed and player has ammo
    return IsFireKeyPressed();
}

void RecoilControl::UpdateShootingState() {
    bool currentlyShooting = IsPlayerShooting();
    float currentTime = GetCurrentTime();
    
    if (currentlyShooting && !isShooting) {
        // Started shooting
        isShooting = true;
        shotsFired = 0;
        totalRecoil = Vector2(0, 0);
        lastShotTime = currentTime;
    } else if (currentlyShooting && isShooting) {
        // Continue shooting
        WeaponRecoilPattern* pattern = GetWeaponPattern(currentWeapon);
        float fireRate = pattern ? pattern->fireRate : 150.0f; // Default fire rate (ms)
        
        if (currentTime - lastShotTime >= fireRate) {
            shotsFired++;
            lastShotTime = currentTime;
        }
    } else if (!currentlyShooting && isShooting) {
        // Stopped shooting
        ResetRecoilState();
    }
    
    // Reset if too much time has passed without shooting
    if (isShooting && currentTime - lastShotTime > 500.0f) {
        ResetRecoilState();
    }
}

void RecoilControl::ResetRecoilState() {
    isShooting = false;
    shotsFired = 0;
    totalRecoil = Vector2(0, 0);
    lastShotTime = 0;
}

void RecoilControl::InitializeRecoilPatterns() {
    recoilPatterns.clear();
    
    // Vandal pattern
    WeaponRecoilPattern vandal(1, 100.0f); // 100ms between shots
    vandal.pattern = {
        Vector2(0, 0),      // Shot 1
        Vector2(0, 1.5f),   // Shot 2
        Vector2(0, 2.2f),   // Shot 3
        Vector2(-0.5f, 2.8f), // Shot 4
        Vector2(-1.2f, 3.2f), // Shot 5
        Vector2(-1.8f, 3.5f), // Shot 6
        Vector2(-2.2f, 3.7f), // Shot 7
        Vector2(-2.5f, 3.8f), // Shot 8
        Vector2(-2.3f, 3.9f), // Shot 9
        Vector2(-1.8f, 4.0f), // Shot 10
        Vector2(-1.0f, 4.0f), // Shot 11
        Vector2(0.2f, 4.0f),  // Shot 12
        Vector2(1.5f, 3.9f),  // Shot 13
        Vector2(2.8f, 3.8f),  // Shot 14
        Vector2(3.5f, 3.7f),  // Shot 15
        Vector2(3.8f, 3.5f),  // Shot 16
        Vector2(3.9f, 3.2f),  // Shot 17
        Vector2(3.7f, 2.8f),  // Shot 18
        Vector2(3.2f, 2.5f),  // Shot 19
        Vector2(2.5f, 2.2f),  // Shot 20
        Vector2(1.5f, 2.0f),  // Shot 21
        Vector2(0.2f, 1.8f),  // Shot 22
        Vector2(-1.0f, 1.8f), // Shot 23
        Vector2(-2.0f, 1.8f), // Shot 24
        Vector2(-2.5f, 1.8f), // Shot 25
        Vector2(-2.8f, 1.8f), // Shot 26
        Vector2(-2.8f, 1.8f), // Shot 27
        Vector2(-2.5f, 1.8f), // Shot 28
        Vector2(-2.0f, 1.8f), // Shot 29
        Vector2(-1.5f, 1.8f)  // Shot 30
    };
    recoilPatterns.push_back(vandal);
    
    // Phantom pattern
    WeaponRecoilPattern phantom(2, 90.0f); // 90ms between shots
    phantom.pattern = {
        Vector2(0, 0),      // Shot 1
        Vector2(0, 1.2f),   // Shot 2
        Vector2(0, 1.8f),   // Shot 3
        Vector2(-0.3f, 2.3f), // Shot 4
        Vector2(-0.8f, 2.7f), // Shot 5
        Vector2(-1.3f, 3.0f), // Shot 6
        Vector2(-1.6f, 3.2f), // Shot 7
        Vector2(-1.8f, 3.3f), // Shot 8
        Vector2(-1.7f, 3.4f), // Shot 9
        Vector2(-1.4f, 3.4f), // Shot 10
        Vector2(-0.8f, 3.4f), // Shot 11
        Vector2(0.1f, 3.4f),  // Shot 12
        Vector2(1.0f, 3.3f),  // Shot 13
        Vector2(1.8f, 3.2f),  // Shot 14
        Vector2(2.4f, 3.0f),  // Shot 15
        Vector2(2.7f, 2.7f),  // Shot 16
        Vector2(2.8f, 2.3f),  // Shot 17
        Vector2(2.6f, 1.8f),  // Shot 18
        Vector2(2.2f, 1.5f),  // Shot 19
        Vector2(1.6f, 1.3f),  // Shot 20
        Vector2(0.8f, 1.2f),  // Shot 21
        Vector2(0.1f, 1.2f),  // Shot 22
        Vector2(-0.6f, 1.2f), // Shot 23
        Vector2(-1.2f, 1.2f), // Shot 24
        Vector2(-1.5f, 1.2f), // Shot 25
        Vector2(-1.6f, 1.2f), // Shot 26
        Vector2(-1.5f, 1.2f), // Shot 27
        Vector2(-1.2f, 1.2f), // Shot 28
        Vector2(-0.8f, 1.2f), // Shot 29
        Vector2(-0.4f, 1.2f)  // Shot 30
    };
    recoilPatterns.push_back(phantom);
    
    // Spectre pattern (SMG)
    WeaponRecoilPattern spectre(3, 75.0f); // 75ms between shots
    spectre.pattern = {
        Vector2(0, 0),      // Shot 1
        Vector2(0, 0.8f),   // Shot 2
        Vector2(0, 1.2f),   // Shot 3
        Vector2(-0.2f, 1.5f), // Shot 4
        Vector2(-0.5f, 1.7f), // Shot 5
        Vector2(-0.7f, 1.8f), // Shot 6
        Vector2(-0.8f, 1.9f), // Shot 7
        Vector2(-0.7f, 1.9f), // Shot 8
        Vector2(-0.5f, 1.9f), // Shot 9
        Vector2(-0.2f, 1.9f), // Shot 10
        Vector2(0.2f, 1.9f),  // Shot 11
        Vector2(0.5f, 1.8f),  // Shot 12
        Vector2(0.7f, 1.7f),  // Shot 13
        Vector2(0.8f, 1.5f),  // Shot 14
        Vector2(0.7f, 1.2f),  // Shot 15
        Vector2(0.5f, 1.0f),  // Shot 16
        Vector2(0.2f, 0.8f),  // Shot 17
        Vector2(-0.1f, 0.8f), // Shot 18
        Vector2(-0.3f, 0.8f), // Shot 19
        Vector2(-0.4f, 0.8f), // Shot 20
        Vector2(-0.3f, 0.8f), // Shot 21
        Vector2(-0.2f, 0.8f), // Shot 22
        Vector2(-0.1f, 0.8f), // Shot 23
        Vector2(0.1f, 0.8f),  // Shot 24
        Vector2(0.2f, 0.8f),  // Shot 25
        Vector2(0.2f, 0.8f),  // Shot 26
        Vector2(0.1f, 0.8f),  // Shot 27
        Vector2(0.0f, 0.8f),  // Shot 28
        Vector2(-0.1f, 0.8f), // Shot 29
        Vector2(-0.1f, 0.8f)  // Shot 30
    };
    recoilPatterns.push_back(spectre);
}

WeaponRecoilPattern* RecoilControl::GetWeaponPattern(int weaponId) {
    for (auto& pattern : recoilPatterns) {
        if (pattern.weaponId == weaponId) {
            return &pattern;
        }
    }
    return nullptr;
}

Vector2 RecoilControl::GetRecoilCrosshairOffset() {
    if (!settings.drawRecoilCrosshair || !isShooting) {
        return Vector2(0, 0);
    }
    
    return totalRecoil;
}

float RecoilControl::GetCurrentTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

void RecoilControl::MoveMouse(float x, float y) {
    // Get current cursor position
    POINT currentPos;
    GetCursorPos(&currentPos);
    
    // Calculate new position
    int newX = currentPos.x + (int)x;
    int newY = currentPos.y + (int)y;
    
    // Set new cursor position
    SetCursorPos(newX, newY);
}

bool RecoilControl::IsFireKeyPressed() {
    // Check if left mouse button is pressed
    return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
}