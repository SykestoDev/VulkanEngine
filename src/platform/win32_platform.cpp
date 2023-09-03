#include <windows.h>
#include "renderer/vk_renderer.cpp"
#include <iostream>
using namespace std;

// Declare a global variable for the window handle
    HWND hwnd;
    VkInstance vkInstance;
    VkContext vkContext;
    HINSTANCE hInstance = GetModuleHandle(NULL);

// Declare the window procedure
LRESULT __stdcall WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Function to initialize the window and run the message loop
bool InitializeWindow(HWND* hwnd, HINSTANCE hInstance, int nCmdShow) {
    // Register the window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_CROSS); // Default Cursor 
    wc.lpszClassName = "MyWindowClass";

    // Error check Window Registeration 
    if (!RegisterClass(&wc))
    {
        MessageBoxA(*hwnd, "Failed to Register Window Class.", "Error", MB_ICONEXCLAMATION | MB_OK);
    }

    // Create the window
    *hwnd = CreateWindowEx(
        0,
        "MyWindowClass",   // Window class name
        "My_Vulkan_Engine",       // Window title
        WS_OVERLAPPEDWINDOW, // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,           // Window dimensions
        NULL, NULL, hInstance, NULL
    );


    // Error check Window Creation
    if (*hwnd == NULL) {
        // Handle window creation failure
        OutputDebugStringA("Failed to create window.");
        return false;
    }
    
    // Show the window
    ShowWindow(*hwnd, nCmdShow);

    // Add debug output to indicate window creation success
    OutputDebugStringA("Window created successfully.");
    return true;
}

// Function to run the message loop
void RunMessageLoop() {
    MSG msg = {};

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Add debug output to indicate message loop termination
    OutputDebugStringA("\nMessage loop terminated.");
}

// Window procedure implementation
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int main() {
    // Add debug output to indicate main function entry
    OutputDebugStringA("Entering main function.");

    int nCmdShow = SW_SHOWDEFAULT;

    // Error check window Creation or if Shut down
    if (!InitializeWindow(&hwnd, hInstance, nCmdShow))
    {
        return -1;
    }
    
    /*
    RECT rc;
    GetWindowRect(hwnd, &rc);

    // Set the window size to the screen size
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);

    // Set the window style to fullscreen
    SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
    // Update the window
    UpdateWindow(hwnd);
    */

    // Iniitiate Vulkan
    if (!vk_Init(&vkContext, hwnd))
    {
        return -1;
    }
    else // Add debug output to indicate VkInstance failed to create
    OutputDebugStringA("VkInstance successfuly created.");

    if (!vk_render(&vkContext))
    {
        return -1;
    }
    
    // Message Loop
    RunMessageLoop();

    // Add debug output to indicate main function exit
    OutputDebugStringA("\nExiting main function.");
    return 0;
}

