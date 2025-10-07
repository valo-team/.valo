#include "aimbot.h"
#include <iostream>
#include <algorithm>
#include <cmath>

Aimbot::Aimbot() : memory(nullptr), currentTarget(nullptr), isAiming(false) {}

Aimbot::~Aimbot() {
    Shutdown();
}

bool Aimbot::Initialize(Memory* mem) {
    memory = mem;
    currentTarget = nullptr;
    isAiming = false;
    
    std::cout << "[+] Aimbot initialized successfully!" << std::endl;
    return true;
}

void Aimbot::Shutdown() {
    currentTarget = nullptr;
    isAiming = false;
}

void Aimbot::Update() {
    if (!memory || !settings.enabled) {
        return;
    }
    
    // Check if aim key is pressed
    bool aimKeyPressed = IsKeyPressed(settings.aimKey);
    
    if (!aimKeyPressed) {
        currentTarget = nullptr;
        isAiming = false;
        return;
    }
    
    // Get game data
    Entity localPlayer = memory->GetLocalPlayer();
    std::vector<Entity> entities = memory->GetEntityList();
    
    if (!localPlayer.isValid) {
        return;
    }
    
    // Find best target
    Entity* bestTarget = FindBestTarget(entities, localPlayer);
    if (!bestTarget) {
        currentTarget = nullptr;
        isAiming = false;
        return;
    }
    
    currentTarget = bestTarget;
    isAiming = true;
    
    // Check visibility if required
    if (settings.aimOnlyVisible && !IsTargetVisible(*bestTarget, localPlayer)) {
        return;
    }
    
    // Get target position (with prediction if enabled)
    Vector3 targetPos = GetBonePosition(*bestTarget, settings.targetBone);
    if (settings.prediction) {
        targetPos = PredictTargetPosition(*bestTarget, 0.016f); // Assume 60 FPS
    }
    
    // Calculate aim angles
    Vector3 aimAngles = CalculateAimAngles(localPlayer.position, targetPos);
    Vector3 currentAngles = GetViewAngles();
    
    // Apply smoothing or snap
    if (settings.smoothAiming) {
        SmoothAim(aimAngles, currentAngles);
    } else {
        SetViewAngles(aimAngles);
    }
}

Entity* Aimbot::FindBestTarget(const std::vector<Entity>& entities, const Entity& localPlayer) {
    Entity* bestTarget = nullptr;
    float bestScore = FLT_MAX;
    
    for (const auto& entity : entities) {
        if (!entity.isValid || !entity.isEnemy || entity.health <= 0) {
            continue;
        }
        
        // Check distance
        float distance = GetDistanceToTarget(entity, localPlayer);
        if (distance > settings.maxDistance) {
            continue;
        }
        
        // Check FOV
        float fov = GetFOVToTarget(entity, localPlayer);
        if (fov > settings.fov) {
            continue;
        }
        
        // Calculate score (lower is better)
        // Prioritize targets based on FOV and distance
        float score = fov * 0.7f + (distance / settings.maxDistance) * 0.3f;
        
        if (score < bestScore) {
            bestScore = score;
            bestTarget = const_cast<Entity*>(&entity);
        }
    }
    
    return bestTarget;
}

Vector3 Aimbot::CalculateAimAngles(const Vector3& from, const Vector3& to) {
    Vector3 delta = to - from;
    
    float distance = delta.Length();
    if (distance == 0) {
        return Vector3(0, 0, 0);
    }
    
    float pitch = -asin(delta.z / distance) * (180.0f / M_PI);
    float yaw = atan2(delta.y, delta.x) * (180.0f / M_PI);
    
    return ClampAngles(Vector3(pitch, yaw, 0));
}

Vector3 Aimbot::PredictTargetPosition(const Entity& target, float deltaTime) {
    // Simple linear prediction based on velocity
    return target.position + (target.velocity * deltaTime);
}

bool Aimbot::IsTargetVisible(const Entity& target, const Entity& localPlayer) {
    // This is a simplified visibility check
    // In a real implementation, you'd perform ray casting or read visibility flags from the game
    return target.isVisible;
}

void Aimbot::SmoothAim(const Vector3& targetAngles, const Vector3& currentAngles) {
    Vector3 delta = targetAngles - currentAngles;
    
    // Normalize angles
    delta.x = NormalizeAngle(delta.x);
    delta.y = NormalizeAngle(delta.y);
    
    // Apply smoothing
    float smoothFactor = settings.smoothness;
    Vector3 smoothedDelta = delta * (1.0f / smoothFactor);
    
    Vector3 newAngles = currentAngles + smoothedDelta;
    SetViewAngles(ClampAngles(newAngles));
}

void Aimbot::SetMousePosition(float x, float y) {
    // Convert angles to mouse movement
    POINT currentPos;
    GetCursorPos(&currentPos);
    
    // This is a simplified mouse movement
    // In reality, you'd need to calculate proper sensitivity scaling
    int deltaX = (int)(x * 2.0f);
    int deltaY = (int)(y * 2.0f);
    
    SetCursorPos(currentPos.x + deltaX, currentPos.y + deltaY);
}

float Aimbot::GetFOVToTarget(const Entity& target, const Entity& localPlayer) {
    Vector3 targetPos = GetBonePosition(target, settings.targetBone);
    Vector3 aimAngles = CalculateAimAngles(localPlayer.position, targetPos);
    Vector3 currentAngles = GetViewAngles();
    
    Vector3 delta = aimAngles - currentAngles;
    delta.x = NormalizeAngle(delta.x);
    delta.y = NormalizeAngle(delta.y);
    
    return sqrt(delta.x * delta.x + delta.y * delta.y);
}

float Aimbot::GetDistanceToTarget(const Entity& target, const Entity& localPlayer) {
    return localPlayer.position.Distance(target.position) / 100.0f; // Convert to meters
}

Vector3 Aimbot::GetBonePosition(const Entity& entity, int boneIndex) {
    // This would read bone positions from the game's bone matrix
    // For now, we'll use simplified positions
    switch (boneIndex) {
        case 8: // Head
            return entity.headPosition;
        case 6: // Chest
            return Vector3(entity.position.x, entity.position.y, entity.position.z + 50.0f);
        default:
            return entity.position;
    }
}

Vector3 Aimbot::GetViewAngles() {
    // This would read the current view angles from the game
    // For now, return a default value
    return Vector3(0, 0, 0);
}

void Aimbot::SetViewAngles(const Vector3& angles) {
    // This would write view angles to the game memory
    // Implementation depends on the specific game's memory layout
    
    // For external aiming, we can simulate mouse movement instead
    static Vector3 lastAngles(0, 0, 0);
    Vector3 delta = angles - lastAngles;
    
    if (abs(delta.x) > 0.1f || abs(delta.y) > 0.1f) {
        SetMousePosition(delta.y, delta.x); // Note: X and Y are swapped for mouse movement
        lastAngles = angles;
    }
}

bool Aimbot::IsKeyPressed(int key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

float Aimbot::NormalizeAngle(float angle) {
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

Vector3 Aimbot::ClampAngles(const Vector3& angles) {
    Vector3 clamped = angles;
    
    // Clamp pitch
    if (clamped.x > 89.0f) clamped.x = 89.0f;
    if (clamped.x < -89.0f) clamped.x = -89.0f;
    
    // Normalize yaw
    clamped.y = NormalizeAngle(clamped.y);
    
    return clamped;
}