#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <commctrl.h>
#include <iomanip>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

class WindowInfo {
public:
    HWND handle;
    std::string title;
    std::string className;
    RECT rect;
    bool isVisible;
    
    WindowInfo(HWND h) : handle(h) {
        char titleBuffer[256] = {0};
        char classBuffer[256] = {0};
        
        GetWindowTextA(h, titleBuffer, sizeof(titleBuffer));
        GetClassNameA(h, classBuffer, sizeof(classBuffer));
        GetWindowRect(h, &rect);
        isVisible = IsWindowVisible(h);
        
        title = std::string(titleBuffer);
        className = std::string(classBuffer);
    }
};

class WindowGraphicalEditor {
private:
    std::vector<WindowInfo> windows;
    HWND mainWindow;
    HWND listBox;
    HWND statusBar;
    
    // Control IDs
    static const int ID_LISTBOX = 1001;
    static const int ID_REFRESH = 1002;
    static const int ID_MOVE = 1003;
    static const int ID_RESIZE = 1004;
    static const int ID_MINIMIZE = 1005;
    static const int ID_MAXIMIZE = 1006;
    static const int ID_RESTORE = 1007;
    static const int ID_HIDE = 1008;
    static const int ID_SHOW = 1009;
    static const int ID_CREATE = 1010;
    static const int ID_CLOSE_WIN = 1011;
    static const int ID_STATUS = 1012;

public:
    WindowGraphicalEditor() : mainWindow(nullptr), listBox(nullptr), statusBar(nullptr) {}
    
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
        std::vector<WindowInfo>* windows = reinterpret_cast<std::vector<WindowInfo>*>(lParam);
        
        if (IsWindow(hwnd)) {
            char title[256];
            GetWindowTextA(hwnd, title, sizeof(title));
            
            // Only add windows with titles and that are visible or minimized
            if (strlen(title) > 0 && (IsWindowVisible(hwnd) || IsIconic(hwnd))) {
                windows->push_back(WindowInfo(hwnd));
            }
        }
        return TRUE;
    }
    
    void RefreshWindowList() {
        windows.clear();
        EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
        
        // Clear and populate listbox
        SendMessage(listBox, LB_RESETCONTENT, 0, 0);
        
        for (size_t i = 0; i < windows.size(); ++i) {
            std::string displayText = windows[i].title + " [" + windows[i].className + "]";
            if (!windows[i].isVisible) {
                displayText += " (Hidden)";
            }
            SendMessage(listBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(displayText.c_str()));
        }
        
        UpdateStatus("Window list refreshed. Found " + std::to_string(windows.size()) + " windows.");
    }
    
    void UpdateStatus(const std::string& message) {
        if (statusBar) {
            SendMessage(statusBar, SB_SETTEXT, 0, reinterpret_cast<LPARAM>(message.c_str()));
        }
    }
    
    WindowInfo* GetSelectedWindow() {
        int selection = SendMessage(listBox, LB_GETCURSEL, 0, 0);
        if (selection == LB_ERR || selection >= static_cast<int>(windows.size())) {
            UpdateStatus("No window selected!");
            return nullptr;
        }
        return &windows[selection];
    }
    
    int GetIntegerInput(const std::string& prompt, int defaultValue = 0) {
        std::string input = GetStringInput(prompt, std::to_string(defaultValue));
        try {
            return std::stoi(input);
        } catch (...) {
            return defaultValue;
        }
    }
    
    std::string GetStringInput(const std::string& prompt, const std::string& defaultValue = "") {
        // Create a simple input dialog
        char buffer[256];
        strcpy(buffer, defaultValue.c_str());
        
        // Use a simple message box for input (basic implementation)
        std::string message = prompt + "\n\nCurrent value: " + defaultValue + 
                            "\n\nEnter new value in console and press Enter.";
        
        MessageBoxA(mainWindow, message.c_str(), "Input Required", MB_OK | MB_ICONINFORMATION);
        
        // Show console window and get input
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONIN$", "r", stdin);
        
        std::cout << prompt << " [" << defaultValue << "]: ";
        std::string result;
        std::getline(std::cin, result);
        
        if (result.empty()) {
            result = defaultValue;
        }
        
        return result;
    }
    
    void MoveSelectedWindow() {
        WindowInfo* winInfo = GetSelectedWindow();
        if (!winInfo) return;
        
        UpdateStatus("Getting new position for window: " + winInfo->title);
        
        int currentX = winInfo->rect.left;
        int currentY = winInfo->rect.top;
        
        int x = GetIntegerInput("Enter X coordinate", currentX);
        int y = GetIntegerInput("Enter Y coordinate", currentY);
        
        int width = winInfo->rect.right - winInfo->rect.left;
        int height = winInfo->rect.bottom - winInfo->rect.top;
        
        if (SetWindowPos(winInfo->handle, nullptr, x, y, width, height, SWP_NOZORDER)) {
            UpdateStatus("Window moved to (" + std::to_string(x) + ", " + std::to_string(y) + ")");
        } else {
            UpdateStatus("Failed to move window!");
        }
    }
    
    void ResizeSelectedWindow() {
        WindowInfo* winInfo = GetSelectedWindow();
        if (!winInfo) return;
        
        UpdateStatus("Getting new size for window: " + winInfo->title);
        
        int currentWidth = winInfo->rect.right - winInfo->rect.left;
        int currentHeight = winInfo->rect.bottom - winInfo->rect.top;
        
        int width = GetIntegerInput("Enter width", currentWidth);
        int height = GetIntegerInput("Enter height", currentHeight);
        
        if (SetWindowPos(winInfo->handle, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER)) {
            UpdateStatus("Window resized to " + std::to_string(width) + "x" + std::to_string(height));
        } else {
            UpdateStatus("Failed to resize window!");
        }
    }
    
    void ChangeWindowState(int state) {
        WindowInfo* winInfo = GetSelectedWindow();
        if (!winInfo) return;
        
        std::string action;
        switch (state) {
            case SW_MINIMIZE:
                action = "minimized";
                break;
            case SW_MAXIMIZE:
                action = "maximized";
                break;
            case SW_RESTORE:
                action = "restored";
                break;
            case SW_HIDE:
                action = "hidden";
                break;
            case SW_SHOW:
                action = "shown";
                break;
        }
        
        if (ShowWindow(winInfo->handle, state)) {
            UpdateStatus("Window " + action + ": " + winInfo->title);
        } else {
            UpdateStatus("Failed to change window state!");
        }
    }
    
    void CloseSelectedWindow() {
        WindowInfo* winInfo = GetSelectedWindow();
        if (!winInfo) return;
        
        if (SendMessage(winInfo->handle, WM_CLOSE, 0, 0) == 0) {
            UpdateStatus("Close message sent to: " + winInfo->title);
            // Refresh list after a short delay
            Sleep(500);
            RefreshWindowList();
        } else {
            UpdateStatus("Failed to close window!");
        }
    }
    
    static HWND createdWindow;
    
    static LRESULT CALLBACK CreatedWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                // Draw something in the window
                SetBkColor(hdc, RGB(240, 240, 240));
                TextOut(hdc, 10, 10, "Custom Window Created by Lab 4", 31);
                TextOut(hdc, 10, 30, "This window demonstrates WinAPI", 31);
                TextOut(hdc, 10, 50, "window creation capabilities", 27);
                
                // Draw a simple rectangle
                HBRUSH brush = CreateSolidBrush(RGB(100, 150, 200));
                RECT rect = {10, 80, 200, 150};
                FillRect(hdc, &rect, brush);
                DeleteObject(brush);
                
                EndPaint(hwnd, &ps);
                return 0;
            }
            case WM_CLOSE:
                DestroyWindow(hwnd);
                return 0;
            case WM_DESTROY:
                createdWindow = nullptr;
                return 0;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    void CreateNewWindow() {
        if (createdWindow && IsWindow(createdWindow)) {
            UpdateStatus("Custom window already exists!");
            SetForegroundWindow(createdWindow);
            return;
        }
        
        const char* className = "Lab4CustomWindow";
        
        WNDCLASS wc = {};
        wc.lpfnWndProc = CreatedWindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = className;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        
        if (!RegisterClass(&wc)) {
            UpdateStatus("Failed to register window class!");
            return;
        }
        
        createdWindow = CreateWindow(
            className,
            "Lab 4 - Custom Created Window",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            400, 300,
            nullptr, nullptr,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        if (createdWindow) {
            ShowWindow(createdWindow, SW_SHOW);
            UpdateWindow(createdWindow);
            UpdateStatus("New custom window created successfully!");
            RefreshWindowList();
        } else {
            UpdateStatus("Failed to create new window!");
        }
    }
    
    static LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        WindowGraphicalEditor* editor = reinterpret_cast<WindowGraphicalEditor*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        
        switch (msg) {
            case WM_CREATE:
                return 0;
                
            case WM_COMMAND:
                if (editor) {
                    int controlId = LOWORD(wParam);
                    int notification = HIWORD(wParam);
                    
                    // Debug output
                    std::cout << "Button clicked - ID: " << controlId << ", Notification: " << notification << std::endl;
                    
                    switch (controlId) {
                        case ID_REFRESH:
                            std::cout << "Refreshing window list..." << std::endl;
                            editor->RefreshWindowList();
                            break;
                        case ID_MOVE:
                            std::cout << "Move window selected..." << std::endl;
                            editor->MoveSelectedWindow();
                            break;
                        case ID_RESIZE:
                            std::cout << "Resize window selected..." << std::endl;
                            editor->ResizeSelectedWindow();
                            break;
                        case ID_MINIMIZE:
                            std::cout << "Minimize selected..." << std::endl;
                            editor->ChangeWindowState(SW_MINIMIZE);
                            break;
                        case ID_MAXIMIZE:
                            std::cout << "Maximize selected..." << std::endl;
                            editor->ChangeWindowState(SW_MAXIMIZE);
                            break;
                        case ID_RESTORE:
                            std::cout << "Restore selected..." << std::endl;
                            editor->ChangeWindowState(SW_RESTORE);
                            break;
                        case ID_HIDE:
                            std::cout << "Hide selected..." << std::endl;
                            editor->ChangeWindowState(SW_HIDE);
                            break;
                        case ID_SHOW:
                            std::cout << "Show selected..." << std::endl;
                            editor->ChangeWindowState(SW_SHOW);
                            break;
                        case ID_CREATE:
                            std::cout << "Create new window..." << std::endl;
                            editor->CreateNewWindow();
                            break;
                        case ID_CLOSE_WIN:
                            std::cout << "Close window selected..." << std::endl;
                            editor->CloseSelectedWindow();
                            break;
                        default:
                            std::cout << "Unknown button ID: " << controlId << std::endl;
                            break;
                    }
                }
                break;
                
            case WM_SIZE: {
                if (editor && editor->statusBar) {
                    SendMessage(editor->statusBar, WM_SIZE, 0, 0);
                    
                    // Get client area
                    RECT clientRect;
                    GetClientRect(hwnd, &clientRect);
                    
                    // Get status bar height
                    RECT statusRect;
                    GetWindowRect(editor->statusBar, &statusRect);
                    int statusHeight = statusRect.bottom - statusRect.top;
                    
                    // Calculate button area
                    int buttonHeight = 30;
                    int buttonRows = 2;
                    int buttonAreaHeight = buttonRows * (buttonHeight + 5) + 10;
                    
                    // Resize listbox (top area)
                    if (editor->listBox) {
                        int listHeight = clientRect.bottom - buttonAreaHeight - statusHeight - 20;
                        SetWindowPos(editor->listBox, nullptr, 10, 10, 
                                   clientRect.right - 20, listHeight,
                                   SWP_NOZORDER);
                    }
                    
                    std::cout << "Window resized. Listbox should adjust automatically." << std::endl;
                }
                break;
            }
                
            case WM_CLOSE:
                DestroyWindow(hwnd);
                return 0;
                
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    bool CreateMainWindow() {
        const char* className = "Lab4WindowEditor";
        
        WNDCLASS wc = {};
        wc.lpfnWndProc = MainWindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = className;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszMenuName = nullptr;
        
        if (!RegisterClass(&wc)) {
            std::cerr << "Failed to register window class!" << std::endl;
            return false;
        }
        
        mainWindow = CreateWindow(
            className,
            "Lab 4 - Window Graphical Editor",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            700, 550,
            nullptr, nullptr,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        if (!mainWindow) {
            std::cerr << "Failed to create main window!" << std::endl;
            return false;
        }
        
        SetWindowLongPtr(mainWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        
        // Create controls
        CreateControls();
        
        ShowWindow(mainWindow, SW_SHOW);
        UpdateWindow(mainWindow);
        
        return true;
    }
    
    void CreateControls() {
        RECT clientRect;
        GetClientRect(mainWindow, &clientRect);
        
        // Calculate positions
        int statusHeight = 25;
        int buttonHeight = 30;
        int buttonRows = 2;
        int buttonAreaHeight = buttonRows * (buttonHeight + 5) + 10;
        int listHeight = clientRect.bottom - buttonAreaHeight - statusHeight - 20;
        
        // Create listbox for windows (top area)
        listBox = CreateWindow(
            "LISTBOX",
            nullptr,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY,
            10, 10, clientRect.right - 20, listHeight,
            mainWindow,
            reinterpret_cast<HMENU>(ID_LISTBOX),
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Create buttons (bottom area in 2 rows)
        const char* buttonLabels[] = {
            "Refresh List", "Move Window", "Resize Window", "Minimize", "Maximize",
            "Restore", "Hide", "Show", "Create New", "Close Window"
        };
        
        int buttonIDs[] = {
            ID_REFRESH, ID_MOVE, ID_RESIZE, ID_MINIMIZE, ID_MAXIMIZE,
            ID_RESTORE, ID_HIDE, ID_SHOW, ID_CREATE, ID_CLOSE_WIN
        };
        
        int buttonWidth = (clientRect.right - 40) / 5; // 5 buttons per row
        int startY = listHeight + 20;
        
        for (int i = 0; i < 10; ++i) {
            int row = i / 5;
            int col = i % 5;
            int x = 10 + col * (buttonWidth + 5);
            int y = startY + row * (buttonHeight + 5);
            
            HWND button = CreateWindow(
                "BUTTON",
                buttonLabels[i],
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                x, y, buttonWidth - 5, buttonHeight,
                mainWindow,
                reinterpret_cast<HMENU>(buttonIDs[i]),
                GetModuleHandle(nullptr),
                nullptr
            );
            
            if (!button) {
                std::cout << "Failed to create button: " << buttonLabels[i] << std::endl;
            } else {
                std::cout << "Created button: " << buttonLabels[i] << " at (" << x << ", " << y << ") with ID: " << buttonIDs[i] << std::endl;
            }
        }
        
        // Create status bar
        statusBar = CreateWindow(
            STATUSCLASSNAME,
            nullptr,
            WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
            0, 0, 0, 0,
            mainWindow,
            reinterpret_cast<HMENU>(ID_STATUS),
            GetModuleHandle(nullptr),
            nullptr
        );
        
        // Initialize status bar
        UpdateStatus("Ready. Click 'Refresh List' to load windows. Buttons are at the bottom.");
    }
    
    void Run() {
        if (!CreateMainWindow()) {
            return;
        }
        
        // Initial window list load
        RefreshWindowList();
        
        std::cout << "=== Lab 4: Window Graphical Editor ===" << std::endl;
        std::cout << "GUI is now running. Buttons should be visible on the right side." << std::endl;
        std::cout << "If buttons are not visible, try resizing the window." << std::endl;
        std::cout << "Available operations:" << std::endl;
        std::cout << "- Select a window from the list" << std::endl;
        std::cout << "- Click buttons to manipulate windows" << std::endl;
        std::cout << "- Use 'Create New' to test window creation" << std::endl;
        
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
};

// Static member definition
HWND WindowGraphicalEditor::createdWindow = nullptr;

int main() {
    std::cout << "Starting Lab 4 - Window Graphical Editor..." << std::endl;
    
    // Initialize common controls
    InitCommonControls();
    
    WindowGraphicalEditor editor;
    editor.Run();
    
    return 0;
}
