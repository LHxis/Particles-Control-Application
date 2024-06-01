#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <random>
#include <math.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <dwmapi.h>

#define WIDTH  620 // Loader Size X
#define HEIGHT 500 // Loader Size Y

bool drawLine = true;
bool active = false;
float max_distance = 30.0f;
bool enableRainbow = false;
float red = 255.0f;
float gren = 0.0f;
float blue = 0.0f;
float mi = 0.0f;
static float r = 255.0f;
static float g = 0.0f;
static float b = 0.0f;
void SetR(float set) { r = set; }
void SetG(float set1) { g = set1; }
void SetB(float set2) { b = set2; }
ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);

std::vector<ImVec2> circles_pos;
std::vector<ImVec2> circles_dir;
std::vector<int> circles_radius;


void SaveColor(const ImVec4& color) {
    std::ofstream file("C:\\Windows\\Logs\\MeasuredBoot\\0000000000 - 0000000000.log");
    if (file.is_open()) {
        file << color.x << " " << color.y << " " << color.z << " " << color.w;
        file.close();
    }
    else {
        std::cerr << "Unable to open file for writing\n";
    }
}

ImVec4 LoadColor() {
    ImVec4 color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    std::ifstream file("C:\\Windows\\Logs\\MeasuredBoot\\0000000000-0000000000.log");
    if (file.is_open()) {
        file >> color.x >> color.y >> color.z >> color.w;
        file.close();
    }
    else {
        std::cerr << "Unable to open file for reading\n";
    }
    return color;
}

bool getRainbowEnabled() {
    return enableRainbow;
}

void updateRainbowColors() {
    const float speed = 0.002f; // Ajuste a velocidade da transição aqui
    if (red == 255 && blue == 0 && gren < 255) {
        gren += speed;
        if (gren > 255) gren = 255;
    }
    else if (gren == 255 && blue == 0 && red > 0) {
        red -= speed;
        if (red < 0) red = 0;
    }
    else if (red == 0 && gren == 255 && blue < 255) {
        blue += speed;
        if (blue > 255) blue = 255;
    }
    else if (red == 0 && blue == 255 && gren > 0) {
        gren -= speed;
        if (gren < 0) gren = 0;
    }
    else if (gren == 0 && blue == 255 && red < 255) {
        red += speed;
        if (red > 255) red = 255;
    }
    else if (red == 255 && gren == 0 && blue > 0) {
        blue -= speed;
        if (blue < 0) blue = 0;
    }
    SetR(red);
    SetG(gren);
    SetB(blue);
}

void draw_circle(ImVec2 pos, int radius, ImU32 color) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddCircleFilled(pos, radius - 1, color);
}

void draw_line(ImVec2 pos1, ImVec2 pos2, ImU32 color, int radius) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float distance = std::sqrt(std::pow(pos2.x - pos1.x, 2) + std::pow(pos2.y - pos1.y, 2));
    float alpha = (distance <= 20.0f) ? 255.0f : (1.0f - ((distance - 20.0f) / 25.0f)) * 255.0f;

    unsigned char* color_ptr = (unsigned char*)&color;

    draw_list->AddLine(pos1, pos2, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 75));
    if (distance >= 40.0f) {
        draw_list->AddCircleFilled(pos1, radius - 0.96f, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 75));
        draw_list->AddCircleFilled(pos2, radius - 0.96f, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 75));
    }
    else if (distance <= 20.0f) {
        draw_list->AddCircleFilled(pos1, radius, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 75));
        draw_list->AddCircleFilled(pos2, radius, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 75));
    }
    else {
        float radius_factor = 1.0f - ((distance - 20.0f) / 20.0f);
        float offset_factor = 1.0f - radius_factor;
        float offset = (radius - radius * radius_factor) * offset_factor;
        draw_list->AddCircleFilled(pos1, radius - offset, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 75));
        draw_list->AddCircleFilled(pos2, radius - offset, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 75));
    }
}

void move_circles(ImVec2 aaaa) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 360);

    ImVec2 window_size = ImGui::GetWindowSize();

    for (int i = 0; i < circles_pos.size(); i++) {
        ImVec2& pos = circles_pos[i];
        ImVec2& dir = circles_dir[i];
        int radius = circles_radius[i];

        pos.x += dir.x * 0.6;
        pos.y += dir.y * 0.6;

        if (pos.x - radius < 0 || pos.x + radius > aaaa.x + window_size.x) {
            dir.x = -dir.x;
            dir.y = dis(gen) % 2 == 0 ? -1 : 1;
        }

        if (pos.y - radius < 0 || pos.y + radius > aaaa.y + window_size.y) {
            dir.y = -dir.y;
            dir.x = dis(gen) % 2 == 0 ? -1 : 1;
        }
    }
}

void draw_circles_and_lines(ImU32 color, ImVec2 aaa) {
    move_circles(aaa);
    if (active) {
        for (int i = 0; i < circles_pos.size(); i++) {
            if (getRainbowEnabled()) {
                updateRainbowColors();
                color = IM_COL32(r, g, b, 255);
            }
            draw_circle(circles_pos[i], circles_radius[i], color);
            for (int j = i + 1; j < circles_pos.size(); j++) {
                float distance = ImGui::GetIO().FontGlobalScale * std::sqrt(std::pow(circles_pos[j].x - circles_pos[i].x, 2) + std::pow(circles_pos[j].y - circles_pos[i].y, 2));
                if (distance <= max_distance && drawLine) {
                    draw_line(circles_pos[i], circles_pos[j], color, circles_radius[i]);
                }
            }
        }
    }
}

void setup_circles() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    std::uniform_int_distribution<> pos_dis(0, static_cast<int>(1920));
    std::uniform_int_distribution<> pos_dis_y(0, static_cast<int>(1080));

    for (int i = 0; i < 510; i++) {
        circles_pos.push_back(ImVec2(pos_dis(gen), pos_dis_y(gen)));
        circles_dir.push_back(ImVec2(dis(gen) == 0 ? -1 : 1, dis(gen) == 0 ? -1 : 1));
        circles_radius.push_back(3);
    }
}

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ColorPicker(const char* name, ImVec4& color) {
    static ImVec4 backup_color;
    bool open_popup = ImGui::ColorButton((std::string(name) + std::string("##3b")).c_str(), color);
    if (open_popup) {
        ImGui::OpenPopup(name);
        backup_color = color;
    }
    if (ImGui::BeginPopup(name)) {
        ImGui::ColorPicker4((std::string(name) + std::string("##picker")).c_str(), (float*)&color,
            ImGuiColorEditFlags_NoSidePreview |
            ImGuiColorEditFlags_NoSmallPreview |
            ImGuiColorEditFlags_NoLabel |
            ImGuiColorEditFlags_NoTooltip |
            ImGuiColorEditFlags_NoInputs);
        ImGui::EndPopup();
    }
}

// Main code
int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    ImVec4 clear_color = LoadColor();
    WNDCLASSEXW wc;
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = nullptr;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = L"ImGui";
    wc.lpszClassName = L"Example";
    wc.hIconSm = LoadIcon(0, IDI_APPLICATION);

    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowExW(NULL, wc.lpszClassName, L"ImGui", WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) / 2) - (WIDTH / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (HEIGHT / 2), WIDTH, HEIGHT, 0, 0, 0, 0);

    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
 
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    POINT mouse;
    RECT rc = { 0 };
    GetWindowRect(hwnd, &rc);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = NULL;
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);
    
    // Main loop
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            SaveColor(clear_color);
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT));
            ImGui::Begin("Windows", NULL, ImGuiWindowFlags_NoDecoration);
            {
                GetWindowRect(hwnd, &rc);
                if (ImGui::GetWindowPos().x != 0 || ImGui::GetWindowPos().y != 0)
                {
                    MoveWindow(hwnd, rc.left + ImGui::GetWindowPos().x, rc.top + ImGui::GetWindowPos().y, WIDTH, HEIGHT, TRUE);
                    ImGui::SetWindowPos(ImVec2(0.f, 0.f));
                }
                
                ImU32 color = getRainbowEnabled() ? IM_COL32(r, g, b, 255) : IM_COL32(clear_color.x * 255.0f, clear_color.y * 255.0f, clear_color.z * 255.0f, 255);
                draw_circles_and_lines(color, ImGui::GetWindowPos());

                if (ImGui::Checkbox("Toggle", &active)) {
                    if (active) {
                        setup_circles();
                    }
                    else {
                        circles_pos.clear();
                        circles_dir.clear();
                        circles_radius.clear();
                    }
                }
                ImGui::Checkbox("Enable Rainbow", &enableRainbow);
                ImGui::Checkbox("Connect", &drawLine);
                ImGui::Text("Connect Distance");
                ImGui::SliderFloat("##ConnectDistance", &max_distance, 0.0f, 100.0f, "%.1f");
                ImGui::Text("Color");
                ImGui::SameLine();
                ColorPicker("##ColorPicker", clear_color);

                if (!enableRainbow) {
                    r = clear_color.x * 255.0f;
                    g = clear_color.y * 255.0f;
                    b = clear_color.z * 255.0f;
                }

                ImGui::End();
            }
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0 };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}