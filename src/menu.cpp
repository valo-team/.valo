#include "menu.h"
#include <iostream>

// Global settings that will be controlled by the menu
bool g_espEnabled = true;
bool g_aimbotEnabled = false;
bool g_recoilControlEnabled = false;

// ESP Settings
bool g_espBoxes = true;
bool g_espCornerBoxes = false;
bool g_espSnaplines = true;
bool g_espDistance = true;
bool g_espHealthBar = true;
bool g_espBoneESP = false;
bool g_espHeadESP = true;
bool g_espNameESP = true;
bool g_espRadar = false;

// Aimbot Settings
bool g_aimbotSmooth = true;
bool g_aimbotVisibleOnly = true;
float g_aimbotFOV = 90.0f;
float g_aimbotSmoothness = 5.0f;
float g_aimbotMaxDistance = 100.0f;

// Recoil Settings
bool g_recoilControl = false;
float g_recoilStrength = 1.0f;

// Colors
float g_enemyColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};
float g_teamColor[4] = {0.0f, 1.0f, 0.0f, 1.0f};

Menu::Menu() : isVisible(false), currentTab(0), menuWindow(nullptr), isInitialized(false) {
    memset(keysPressed, 0, sizeof(keysPressed));
}

Menu::~Menu() {
    Shutdown();
}

bool Menu::Initialize() {
    // Create menu window
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MenuWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(30, 30, 30));
    wc.lpszClassName = L"ValorantCheatMenu";
    
    RegisterClassEx(&wc);
    
    menuWindow = CreateWindowEx(
        WS_EX_TOPMOST,
        L"ValorantCheatMenu",
        L"Valorant Cheat v1.0",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        100, 100, 400, 500,
        nullptr, nullptr, GetModuleHandle(nullptr), this
    );
    
    if (!menuWindow) {
        return false;
    }
    
    SetupDefaultMenu();
    isInitialized = true;
    
    std::cout << "[+] Menu initialized successfully!" << std::endl;
    return true;
}

void Menu::Shutdown() {
    if (menuWindow) {
        DestroyWindow(menuWindow);
        menuWindow = nullptr;
    }
    isInitialized = false;
}

void Menu::Render() {
    if (!isVisible || !isInitialized) {
        return;
    }
    
    ShowWindow(menuWindow, SW_SHOW);
    UpdateWindow(menuWindow);
    
    HandleInput();
}

void Menu::Toggle() {
    isVisible = !isVisible;
    
    if (isVisible) {
        ShowWindow(menuWindow, SW_SHOW);
        SetForegroundWindow(menuWindow);
    } else {
        ShowWindow(menuWindow, SW_HIDE);
    }
}

void Menu::AddTab(const MenuTab& tab) {
    tabs.push_back(tab);
}

bool Menu::IsESPEnabled() {
    return g_espEnabled;
}

bool Menu::IsAimbotEnabled() {
    return g_aimbotEnabled;
}

bool Menu::IsRecoilControlEnabled() {
    return g_recoilControlEnabled;
}

void Menu::SetupDefaultMenu() {
    // ESP Tab
    MenuTab espTab("ESP");
    espTab.AddCheckbox("Enable ESP", &g_espEnabled);
    espTab.AddSeparator();
    espTab.AddCheckbox("2D Boxes", &g_espBoxes);
    espTab.AddCheckbox("Corner Boxes", &g_espCornerBoxes);
    espTab.AddCheckbox("Snaplines", &g_espSnaplines);
    espTab.AddCheckbox("Distance", &g_espDistance);
    espTab.AddCheckbox("Health Bar", &g_espHealthBar);
    espTab.AddCheckbox("Bone ESP", &g_espBoneESP);
    espTab.AddCheckbox("Head ESP", &g_espHeadESP);
    espTab.AddCheckbox("Name ESP", &g_espNameESP);
    espTab.AddCheckbox("Radar ESP", &g_espRadar);
    espTab.AddSeparator();
    espTab.AddColorPicker("Enemy Color", g_enemyColor);
    espTab.AddColorPicker("Team Color", g_teamColor);
    AddTab(espTab);
    
    // Aimbot Tab
    MenuTab aimbotTab("Aimbot");
    aimbotTab.AddCheckbox("Enable Aimbot", &g_aimbotEnabled);
    aimbotTab.AddSeparator();
    aimbotTab.AddCheckbox("Smooth Aimbot", &g_aimbotSmooth);
    aimbotTab.AddCheckbox("Aim Only Visible", &g_aimbotVisibleOnly);
    aimbotTab.AddSlider("FOV", &g_aimbotFOV, 1.0f, 180.0f);
    aimbotTab.AddSlider("Smoothness", &g_aimbotSmoothness, 1.0f, 20.0f);
    aimbotTab.AddSlider("Max Distance", &g_aimbotMaxDistance, 10.0f, 500.0f);
    AddTab(aimbotTab);
    
    // Recoil Tab
    MenuTab recoilTab("Recoil");
    recoilTab.AddCheckbox("Recoil Control", &g_recoilControl);
    recoilTab.AddSlider("Strength", &g_recoilStrength, 0.1f, 2.0f);
    AddTab(recoilTab);
    
    // Misc Tab
    MenuTab miscTab("Misc");
    miscTab.AddText("Valorant External Cheat v1.0");
    miscTab.AddSeparator();
    miscTab.AddText("Controls:");
    miscTab.AddText("INSERT - Toggle Menu");
    miscTab.AddText("F4 - Exit Cheat");
    miscTab.AddText("RMB - Aimbot (when enabled)");
    miscTab.AddSeparator();
    miscTab.AddButton("Save Config", []() {
        std::cout << "[+] Config saved!" << std::endl;
    });
    miscTab.AddButton("Load Config", []() {
        std::cout << "[+] Config loaded!" << std::endl;
    });
    AddTab(miscTab);
}

void Menu::HandleInput() {
    UpdateKeyStates();
    
    // Tab switching
    if (IsKeyJustPressed(VK_LEFT) && currentTab > 0) {
        currentTab--;
    }
    if (IsKeyJustPressed(VK_RIGHT) && currentTab < (int)tabs.size() - 1) {
        currentTab++;
    }
    
    // Close menu with ESC
    if (IsKeyJustPressed(VK_ESCAPE)) {
        Toggle();
    }
}

void Menu::RenderTab(const MenuTab& tab) {
    // This would render the current tab's items
    // Implementation would depend on the graphics library used
}

void Menu::RenderMenuItem(const MenuItem& item, int& yPos) {
    // This would render individual menu items
    // Implementation would depend on the graphics library used
}

void Menu::DrawText(int x, int y, const std::string& text, COLORREF color) {
    // Simplified text drawing using Windows GDI
    HDC hdc = GetDC(menuWindow);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    
    // Convert string to wide string
    std::wstring wtext(text.begin(), text.end());
    TextOut(hdc, x, y, wtext.c_str(), (int)wtext.length());
    
    ReleaseDC(menuWindow, hdc);
}

void Menu::DrawRect(int x, int y, int width, int height, COLORREF color) {
    HDC hdc = GetDC(menuWindow);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    
    Rectangle(hdc, x, y, x + width, y + height);
    
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
    ReleaseDC(menuWindow, hdc);
}

void Menu::DrawFilledRect(int x, int y, int width, int height, COLORREF color) {
    HDC hdc = GetDC(menuWindow);
    HBRUSH brush = CreateSolidBrush(color);
    RECT rect = {x, y, x + width, y + height};
    
    FillRect(hdc, &rect, brush);
    
    DeleteObject(brush);
    ReleaseDC(menuWindow, hdc);
}

void Menu::DrawCheckbox(int x, int y, bool checked) {
    DrawRect(x, y, 15, 15, RGB(200, 200, 200));
    if (checked) {
        DrawFilledRect(x + 3, y + 3, 9, 9, RGB(0, 255, 0));
    }
}

void Menu::DrawSlider(int x, int y, float value, float min, float max, int width) {
    // Draw slider track
    DrawRect(x, y + 5, width, 5, RGB(100, 100, 100));
    
    // Draw slider handle
    float normalizedValue = (value - min) / (max - min);
    int handleX = x + (int)(normalizedValue * width) - 5;
    DrawFilledRect(handleX, y, 10, 15, RGB(0, 150, 255));
}

void Menu::DrawButton(int x, int y, const std::string& text, int width, int height) {
    DrawRect(x, y, width, height, RGB(100, 100, 100));
    DrawText(x + 10, y + 5, text, RGB(255, 255, 255));
}

void Menu::DrawColorPicker(int x, int y, float* color, int size) {
    COLORREF colorRef = RGB(
        (int)(color[0] * 255),
        (int)(color[1] * 255),
        (int)(color[2] * 255)
    );
    DrawFilledRect(x, y, size, size, colorRef);
    DrawRect(x, y, size, size, RGB(255, 255, 255));
}

bool Menu::IsKeyPressed(int key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

bool Menu::IsKeyJustPressed(int key) {
    bool currentState = IsKeyPressed(key);
    bool wasPressed = keysPressed[key];
    return currentState && !wasPressed;
}

void Menu::UpdateKeyStates() {
    for (int i = 0; i < 256; i++) {
        keysPressed[i] = IsKeyPressed(i);
    }
}

LRESULT CALLBACK Menu::MenuWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Menu* menu = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        menu = (Menu*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)menu);
    } else {
        menu = (Menu*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            if (menu && menu->isVisible) {
                // Draw menu background
                RECT rect;
                GetClientRect(hwnd, &rect);
                FillRect(hdc, &rect, (HBRUSH)CreateSolidBrush(RGB(30, 30, 30)));
                
                // Draw title
                SetTextColor(hdc, RGB(255, 255, 255));
                SetBkMode(hdc, TRANSPARENT);
                TextOut(hdc, 10, 10, L"Valorant Cheat v1.0", 19);
                
                // Draw tab names
                if (!menu->tabs.empty()) {
                    for (size_t i = 0; i < menu->tabs.size(); i++) {
                        COLORREF color = (i == menu->currentTab) ? RGB(0, 255, 0) : RGB(200, 200, 200);
                        SetTextColor(hdc, color);
                        
                        std::wstring tabName(menu->tabs[i].name.begin(), menu->tabs[i].name.end());
                        TextOut(hdc, 10 + (int)i * 80, 40, tabName.c_str(), (int)tabName.length());
                    }
                }
            }
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_CLOSE:
            menu->SetVisible(false);
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}