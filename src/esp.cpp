#include "esp.h"
#include <iostream>

ESP::ESP() : memory(nullptr), device(nullptr), context(nullptr), swapChain(nullptr), 
             renderTargetView(nullptr), overlayWindow(nullptr), targetWindow(nullptr), 
             isInitialized(false) {}

ESP::~ESP() {
    Shutdown();
}

bool ESP::Initialize(Memory* mem) {
    memory = mem;
    
    // Find target window (Valorant)
    targetWindow = FindWindow(NULL, L"VALORANT");
    if (!targetWindow) {
        std::cout << "[-] Could not find Valorant window!" << std::endl;
        return false;
    }
    
    if (!CreateOverlay()) {
        std::cout << "[-] Failed to create overlay window!" << std::endl;
        return false;
    }
    
    if (!InitializeDirectX()) {
        std::cout << "[-] Failed to initialize DirectX!" << std::endl;
        return false;
    }
    
    isInitialized = true;
    std::cout << "[+] ESP initialized successfully!" << std::endl;
    return true;
}

void ESP::Shutdown() {
    CleanupDirectX();
    
    if (overlayWindow) {
        DestroyWindow(overlayWindow);
        overlayWindow = nullptr;
    }
    
    isInitialized = false;
}

void ESP::Render() {
    if (!isInitialized || !memory) {
        return;
    }
    
    // Update overlay position to match target window
    RECT targetRect;
    GetWindowRect(targetWindow, &targetRect);
    SetWindowPos(overlayWindow, HWND_TOPMOST, targetRect.left, targetRect.top, 
                 targetRect.right - targetRect.left, targetRect.bottom - targetRect.top, 
                 SWP_NOACTIVATE);
    
    // Clear the render target
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // Transparent
    context->ClearRenderTargetView(renderTargetView, clearColor);
    
    // Get entities
    Entity localPlayer = memory->GetLocalPlayer();
    std::vector<Entity> entities = memory->GetEntityList();
    
    // Render ESP for each entity
    for (const auto& entity : entities) {
        if (!entity.isValid || entity.address == localPlayer.address) {
            continue;
        }
        
        RenderEntityESP(entity, localPlayer);
    }
    
    // Render crosshair if enabled
    if (settings.enabled) {
        RenderCrosshair();
    }
    
    // Render radar if enabled
    if (settings.radarESP) {
        DrawRadar(entities, localPlayer);
    }
    
    // Present the frame
    swapChain->Present(1, 0);
}

void ESP::RenderEntityESP(const Entity& entity, const Entity& localPlayer) {
    Vector2 headScreen, feetScreen;
    
    // Convert world positions to screen coordinates
    if (!memory->WorldToScreen(entity.headPosition, headScreen) ||
        !memory->WorldToScreen(entity.position, feetScreen)) {
        return;
    }
    
    // Calculate box dimensions
    float entityHeight = abs(headScreen.y - feetScreen.y);
    float entityWidth = entityHeight * 0.4f; // Approximate width based on height
    
    Vector2 topLeft(headScreen.x - entityWidth / 2, headScreen.y);
    Vector2 bottomRight(headScreen.x + entityWidth / 2, feetScreen.y);
    
    // Choose color based on team and visibility
    const float* color = entity.isEnemy ? settings.enemyColor : settings.teamColor;
    if (entity.isVisible && settings.colorESP) {
        color = settings.visibleColor;
    }
    
    // Draw boxes
    if (settings.boxes) {
        DrawBox(topLeft, bottomRight, color, settings.boxThickness);
    }
    
    if (settings.cornerBoxes) {
        DrawCornerBox(topLeft, bottomRight, color, settings.boxThickness);
    }
    
    // Draw snaplines
    if (settings.snaplines) {
        Vector2 screenCenter = GetScreenCenter();
        DrawLine(Vector2(screenCenter.x, screenCenter.y), 
                Vector2(feetScreen.x, feetScreen.y), color, settings.snaplineThickness);
    }
    
    // Draw health bar
    if (settings.healthBar && entity.health > 0) {
        Vector2 healthPos(topLeft.x - 8, topLeft.y);
        DrawHealthBar(healthPos, entity.health, entity.maxHealth);
    }
    
    // Draw distance
    if (settings.distance) {
        float distance = localPlayer.position.Distance(entity.position) / 100.0f; // Convert to meters
        std::string distanceText = std::to_string((int)distance) + "m";
        Vector2 textPos(bottomRight.x + 5, bottomRight.y);
        DrawText(textPos, distanceText, color);
    }
    
    // Draw name
    if (settings.nameESP && !entity.name.empty()) {
        Vector2 namePos(topLeft.x, topLeft.y - 15);
        DrawText(namePos, entity.name, color);
    }
    
    // Draw head ESP
    if (settings.headESP) {
        Vector2 headBoxTopLeft(headScreen.x - 8, headScreen.y - 8);
        Vector2 headBoxBottomRight(headScreen.x + 8, headScreen.y + 8);
        DrawBox(headBoxTopLeft, headBoxBottomRight, color, 1.0f);
    }
    
    // Draw bones
    if (settings.boneESP) {
        DrawBones(entity, color);
    }
}

void ESP::DrawBox(const Vector2& topLeft, const Vector2& bottomRight, const float color[4], float thickness) {
    // Draw four lines to make a rectangle
    DrawLine(topLeft, Vector2(bottomRight.x, topLeft.y), color, thickness); // Top
    DrawLine(Vector2(bottomRight.x, topLeft.y), bottomRight, color, thickness); // Right
    DrawLine(bottomRight, Vector2(topLeft.x, bottomRight.y), color, thickness); // Bottom
    DrawLine(Vector2(topLeft.x, bottomRight.y), topLeft, color, thickness); // Left
}

void ESP::DrawCornerBox(const Vector2& topLeft, const Vector2& bottomRight, const float color[4], float thickness) {
    float cornerSize = 10.0f;
    
    // Top-left corner
    DrawLine(topLeft, Vector2(topLeft.x + cornerSize, topLeft.y), color, thickness);
    DrawLine(topLeft, Vector2(topLeft.x, topLeft.y + cornerSize), color, thickness);
    
    // Top-right corner
    DrawLine(Vector2(bottomRight.x, topLeft.y), Vector2(bottomRight.x - cornerSize, topLeft.y), color, thickness);
    DrawLine(Vector2(bottomRight.x, topLeft.y), Vector2(bottomRight.x, topLeft.y + cornerSize), color, thickness);
    
    // Bottom-left corner
    DrawLine(Vector2(topLeft.x, bottomRight.y), Vector2(topLeft.x + cornerSize, bottomRight.y), color, thickness);
    DrawLine(Vector2(topLeft.x, bottomRight.y), Vector2(topLeft.x, bottomRight.y - cornerSize), color, thickness);
    
    // Bottom-right corner
    DrawLine(bottomRight, Vector2(bottomRight.x - cornerSize, bottomRight.y), color, thickness);
    DrawLine(bottomRight, Vector2(bottomRight.x, bottomRight.y - cornerSize), color, thickness);
}

void ESP::DrawLine(const Vector2& start, const Vector2& end, const float color[4], float thickness) {
    // This is a simplified line drawing function
    // In a real implementation, you'd use DirectX primitives or a graphics library
    // For now, this is a placeholder that represents the concept
}

void ESP::DrawText(const Vector2& position, const std::string& text, const float color[4]) {
    // This is a simplified text drawing function
    // In a real implementation, you'd use DirectWrite or a text rendering library
    // For now, this is a placeholder that represents the concept
}

void ESP::DrawHealthBar(const Vector2& position, float health, float maxHealth, float width, float height) {
    float healthPercentage = health / maxHealth;
    
    // Background (red)
    float bgColor[4] = {0.5f, 0.0f, 0.0f, 0.8f};
    Vector2 bgTopLeft = position;
    Vector2 bgBottomRight(position.x + width, position.y + height);
    DrawBox(bgTopLeft, bgBottomRight, bgColor, 1.0f);
    
    // Health bar (green)
    float healthHeight = height * healthPercentage;
    Vector2 healthTopLeft(position.x, position.y + height - healthHeight);
    Vector2 healthBottomRight(position.x + width, position.y + height);
    DrawBox(healthTopLeft, healthBottomRight, settings.healthColor, 1.0f);
}

void ESP::DrawBones(const Entity& entity, const float color[4]) {
    // This would draw a skeleton overlay
    // Implementation would require bone position offsets from the game
    // For now, this is a placeholder
}

void ESP::DrawRadar(const std::vector<Entity>& entities, const Entity& localPlayer) {
    Vector2 radarCenter(100, 100);
    float radarRadius = 80.0f;
    
    // Draw radar background
    float bgColor[4] = {0.0f, 0.0f, 0.0f, 0.5f};
    Vector2 radarTopLeft(radarCenter.x - radarRadius, radarCenter.y - radarRadius);
    Vector2 radarBottomRight(radarCenter.x + radarRadius, radarCenter.y + radarRadius);
    DrawBox(radarTopLeft, radarBottomRight, bgColor, 2.0f);
    
    // Draw entities on radar
    for (const auto& entity : entities) {
        if (!entity.isValid || entity.address == localPlayer.address) {
            continue;
        }
        
        Vector3 relative = entity.position - localPlayer.position;
        float distance = relative.Length();
        
        if (distance > 5000.0f) continue; // 50 meter range
        
        float normalizedX = (relative.x / 5000.0f) * radarRadius;
        float normalizedY = (relative.z / 5000.0f) * radarRadius; // Z is forward in Valorant
        
        Vector2 radarPos(radarCenter.x + normalizedX, radarCenter.y + normalizedY);
        
        const float* color = entity.isEnemy ? settings.enemyColor : settings.teamColor;
        
        // Draw entity dot
        Vector2 dotTopLeft(radarPos.x - 2, radarPos.y - 2);
        Vector2 dotBottomRight(radarPos.x + 2, radarPos.y + 2);
        DrawBox(dotTopLeft, dotBottomRight, color, 1.0f);
    }
    
    // Draw local player in center
    float localColor[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // White
    Vector2 localDotTopLeft(radarCenter.x - 2, radarCenter.y - 2);
    Vector2 localDotBottomRight(radarCenter.x + 2, radarCenter.y + 2);
    DrawBox(localDotTopLeft, localDotBottomRight, localColor, 1.0f);
}

void ESP::RenderCrosshair() {
    Vector2 center = GetScreenCenter();
    float size = 10.0f;
    float thickness = 2.0f;
    float crosshairColor[4] = {1.0f, 1.0f, 1.0f, 0.8f}; // White
    
    // Draw crosshair lines
    DrawLine(Vector2(center.x - size, center.y), Vector2(center.x + size, center.y), crosshairColor, thickness);
    DrawLine(Vector2(center.x, center.y - size), Vector2(center.x, center.y + size), crosshairColor, thickness);
}

void ESP::RenderFOVCircle(float fov) {
    Vector2 center = GetScreenCenter();
    float radius = fov;
    float circleColor[4] = {1.0f, 1.0f, 1.0f, 0.3f}; // Semi-transparent white
    
    // This would draw a circle representing the FOV
    // Implementation would require circle drawing primitives
}

bool ESP::CreateOverlay() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    wc.lpszClassName = L"ESPOverlay";
    
    RegisterClassEx(&wc);
    
    overlayWindow = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        L"ESPOverlay",
        L"ESP Overlay",
        WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr
    );
    
    if (!overlayWindow) {
        return false;
    }
    
    SetLayeredWindowAttributes(overlayWindow, RGB(0, 0, 0), 255, LWA_COLORKEY);
    ShowWindow(overlayWindow, SW_SHOW);
    UpdateWindow(overlayWindow);
    
    return true;
}

bool ESP::InitializeDirectX() {
    // This is a simplified DirectX initialization
    // In a real implementation, you'd set up the full DirectX pipeline
    // For now, this returns true to indicate successful "initialization"
    return true;
}

void ESP::CleanupDirectX() {
    if (renderTargetView) {
        renderTargetView->Release();
        renderTargetView = nullptr;
    }
    
    if (swapChain) {
        swapChain->Release();
        swapChain = nullptr;
    }
    
    if (context) {
        context->Release();
        context = nullptr;
    }
    
    if (device) {
        device->Release();
        device = nullptr;
    }
}

Vector2 ESP::GetScreenCenter() {
    return Vector2(GetSystemMetrics(SM_CXSCREEN) / 2.0f, GetSystemMetrics(SM_CYSCREEN) / 2.0f);
}

float ESP::CalculateDistance2D(const Vector2& pos1, const Vector2& pos2) {
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    return sqrt(dx * dx + dy * dy);
}

LRESULT CALLBACK ESP::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}