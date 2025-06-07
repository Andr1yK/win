#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

// Resource IDs
#define IDC_TREE_REGISTRY      1001
#define IDC_LIST_VALUES        1002
#define IDC_EDIT_KEY_PATH      1003
#define IDC_EDIT_VALUE_NAME    1004
#define IDC_EDIT_VALUE_DATA    1005
#define IDC_BUTTON_CREATE_KEY  1006
#define IDC_BUTTON_DELETE_KEY  1007
#define IDC_BUTTON_SET_VALUE   1008
#define IDC_BUTTON_DELETE_VALUE 1009
#define IDC_BUTTON_REFRESH     1010
#define IDC_BUTTON_SAVE_TO_FILE 1011
#define IDC_BUTTON_LOAD_FROM_FILE 1012
#define IDC_STATUS_BAR         1013
#define IDC_COMBO_ROOT_KEY     1014
#define IDC_BUTTON_CHECK_KEY   1015

// Global variables
HWND hMainWindow;
HWND hTreeView;
HWND hListView;
HWND hEditKeyPath;
HWND hEditValueName;
HWND hEditValueData;
HWND hStatusBar;
HWND hComboRootKey;

// Registry root keys structure
struct RegistryRoot {
    HKEY hKey;
    const char* name;
    const char* displayName;
};

RegistryRoot rootKeys[] = {
    {HKEY_CURRENT_USER, "HKEY_CURRENT_USER", "HKEY_CURRENT_USER"},
    {HKEY_LOCAL_MACHINE, "HKEY_LOCAL_MACHINE", "HKEY_LOCAL_MACHINE"},
    {HKEY_CLASSES_ROOT, "HKEY_CLASSES_ROOT", "HKEY_CLASSES_ROOT"},
    {HKEY_USERS, "HKEY_USERS", "HKEY_USERS"},
    {HKEY_CURRENT_CONFIG, "HKEY_CURRENT_CONFIG", "HKEY_CURRENT_CONFIG"}
};

const int ROOT_KEYS_COUNT = sizeof(rootKeys) / sizeof(rootKeys[0]);

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeControls(HWND hwnd);
void PopulateTreeView();
void PopulateRootKey(HTREEITEM hParent, HKEY hKey, const std::string& keyPath, int depth = 0);
void OnTreeSelectionChanged();
void PopulateValuesList(HKEY hKey, const std::string& keyPath);
void CreateRegistryKey();
void DeleteRegistryKey();
void SetRegistryValue();
void DeleteRegistryValue();
void SaveRegistryToFile();
void LoadRegistryFromFile();
void UpdateStatusBar(const std::string& message);
HKEY GetSelectedRootKey();
std::string GetSelectedKeyPath();
std::string GetWindowText(HWND hwnd);
void SetWindowText(HWND hwnd, const std::string& text);
void CheckKeyExists();
bool KeyExists(HKEY rootKey, const std::string& keyPath);

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    // Register window class
    const char* className = "RegistryManagerWindow";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    RegisterClass(&wc);

    // Create main window
    hMainWindow = CreateWindowEx(
        0,
        className,
        "Windows Registry Manager - Lab 3 (Group 3)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1200, 800,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hMainWindow) {
        MessageBox(nullptr, "Failed to create window!", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    ShowWindow(hMainWindow, nCmdShow);
    UpdateWindow(hMainWindow);

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        InitializeControls(hwnd);
        PopulateTreeView();
        UpdateStatusBar("Registry Manager initialized successfully");
        break;

    case WM_SIZE: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        // Resize controls
        MoveWindow(hTreeView, 10, 100, 350, height - 150, TRUE);
        MoveWindow(hListView, 370, 100, width - 390, height - 150, TRUE);
        SendMessage(hStatusBar, WM_SIZE, 0, 0);
        break;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BUTTON_CREATE_KEY:
            CreateRegistryKey();
            break;
        case IDC_BUTTON_DELETE_KEY:
            DeleteRegistryKey();
            break;
        case IDC_BUTTON_SET_VALUE:
            SetRegistryValue();
            break;
        case IDC_BUTTON_DELETE_VALUE:
            DeleteRegistryValue();
            break;
        case IDC_BUTTON_REFRESH:
            PopulateTreeView();
            OnTreeSelectionChanged();
            break;
        case IDC_BUTTON_SAVE_TO_FILE:
            SaveRegistryToFile();
            break;
        case IDC_BUTTON_LOAD_FROM_FILE:
            LoadRegistryFromFile();
            break;
        case IDC_BUTTON_CHECK_KEY:
            CheckKeyExists();
            break;
        }
        break;

    case WM_NOTIFY: {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        if (pnmhdr->hwndFrom == hTreeView && pnmhdr->code == TVN_SELCHANGED) {
            OnTreeSelectionChanged();
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Initialize all GUI controls
void InitializeControls(HWND hwnd) {
    // Create combo box for root keys
    hComboRootKey = CreateWindow("COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        10, 10, 200, 200, hwnd, (HMENU)IDC_COMBO_ROOT_KEY, nullptr, nullptr);

    // Populate combo box
    for (int i = 0; i < ROOT_KEYS_COUNT; i++) {
        SendMessage(hComboRootKey, CB_ADDSTRING, 0, (LPARAM)rootKeys[i].displayName);
    }
    SendMessage(hComboRootKey, CB_SETCURSEL, 0, 0);

    // Create edit controls
    CreateWindow("STATIC", "Key Path:", WS_CHILD | WS_VISIBLE,
        220, 10, 80, 20, hwnd, nullptr, nullptr, nullptr);
    hEditKeyPath = CreateWindow("EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        300, 10, 300, 20, hwnd, (HMENU)IDC_EDIT_KEY_PATH, nullptr, nullptr);

    CreateWindow("STATIC", "Value Name:", WS_CHILD | WS_VISIBLE,
        10, 40, 80, 20, hwnd, nullptr, nullptr, nullptr);
    hEditValueName = CreateWindow("EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        100, 40, 150, 20, hwnd, (HMENU)IDC_EDIT_VALUE_NAME, nullptr, nullptr);

    CreateWindow("STATIC", "Value Data:", WS_CHILD | WS_VISIBLE,
        260, 40, 80, 20, hwnd, nullptr, nullptr, nullptr);
    hEditValueData = CreateWindow("EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        340, 40, 150, 20, hwnd, (HMENU)IDC_EDIT_VALUE_DATA, nullptr, nullptr);

    // Create buttons
    CreateWindow("BUTTON", "Create Key", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 70, 80, 25, hwnd, (HMENU)IDC_BUTTON_CREATE_KEY, nullptr, nullptr);

    CreateWindow("BUTTON", "Delete Key", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        100, 70, 80, 25, hwnd, (HMENU)IDC_BUTTON_DELETE_KEY, nullptr, nullptr);

    CreateWindow("BUTTON", "Set Value", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        190, 70, 80, 25, hwnd, (HMENU)IDC_BUTTON_SET_VALUE, nullptr, nullptr);

    CreateWindow("BUTTON", "Delete Value", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        280, 70, 80, 25, hwnd, (HMENU)IDC_BUTTON_DELETE_VALUE, nullptr, nullptr);

    CreateWindow("BUTTON", "Check Key", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        370, 70, 80, 25, hwnd, (HMENU)IDC_BUTTON_CHECK_KEY, nullptr, nullptr);

    CreateWindow("BUTTON", "Refresh", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        460, 70, 60, 25, hwnd, (HMENU)IDC_BUTTON_REFRESH, nullptr, nullptr);

    CreateWindow("BUTTON", "Save to File", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        530, 70, 80, 25, hwnd, (HMENU)IDC_BUTTON_SAVE_TO_FILE, nullptr, nullptr);

    CreateWindow("BUTTON", "Load from File", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        620, 70, 90, 25, hwnd, (HMENU)IDC_BUTTON_LOAD_FROM_FILE, nullptr, nullptr);

    // Create tree view
    hTreeView = CreateWindow(WC_TREEVIEW, "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT,
        10, 100, 350, 400, hwnd, (HMENU)IDC_TREE_REGISTRY, nullptr, nullptr);

    // Create list view for values
    hListView = CreateWindow(WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT,
        370, 100, 400, 400, hwnd, (HMENU)IDC_LIST_VALUES, nullptr, nullptr);

    // Set up list view columns
    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.cx = 150;
    lvc.pszText = "Name";
    ListView_InsertColumn(hListView, 0, &lvc);

    lvc.cx = 100;
    lvc.pszText = "Type";
    ListView_InsertColumn(hListView, 1, &lvc);

    lvc.cx = 200;
    lvc.pszText = "Data";
    ListView_InsertColumn(hListView, 2, &lvc);

    // Create status bar
    hStatusBar = CreateWindow(STATUSCLASSNAME, "",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd, (HMENU)IDC_STATUS_BAR, nullptr, nullptr);
}

// Populate tree view with registry structure
void PopulateTreeView() {
    TreeView_DeleteAllItems(hTreeView);

    for (int i = 0; i < ROOT_KEYS_COUNT; i++) {
        TVINSERTSTRUCT tvins;
        tvins.hParent = TVI_ROOT;
        tvins.hInsertAfter = TVI_LAST;
        tvins.item.mask = TVIF_TEXT | TVIF_PARAM;
        tvins.item.pszText = (char*)rootKeys[i].displayName;
        tvins.item.lParam = (LPARAM)rootKeys[i].hKey;

        HTREEITEM hRoot = TreeView_InsertItem(hTreeView, &tvins);
        PopulateRootKey(hRoot, rootKeys[i].hKey, "", 0);
    }
}

// Populate registry key recursively (limit depth to avoid performance issues)
void PopulateRootKey(HTREEITEM hParent, HKEY hKey, const std::string& keyPath, int depth) {
    if (depth > 2) return; // Limit depth to avoid performance issues

    HKEY hSubKey;
    if (RegOpenKeyEx(hKey, keyPath.c_str(), 0, KEY_READ, &hSubKey) != ERROR_SUCCESS) {
        return;
    }

    char subKeyName[256];
    DWORD subKeyNameSize = sizeof(subKeyName);
    DWORD index = 0;

    while (RegEnumKeyEx(hSubKey, index++, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        TVINSERTSTRUCT tvins;
        tvins.hParent = hParent;
        tvins.hInsertAfter = TVI_LAST;
        tvins.item.mask = TVIF_TEXT;
        tvins.item.pszText = subKeyName;

        HTREEITEM hItem = TreeView_InsertItem(hTreeView, &tvins);

        // Only expand one level further for main branches
        if (depth < 1) {
            std::string newPath = keyPath.empty() ? subKeyName : keyPath + "\\\\" + subKeyName;
            PopulateRootKey(hItem, hKey, newPath, depth + 1);
        }

        subKeyNameSize = sizeof(subKeyName);
    }

    RegCloseKey(hSubKey);
}

// Handle tree selection change
void OnTreeSelectionChanged() {
    HTREEITEM hItem = TreeView_GetSelection(hTreeView);
    if (!hItem) return;

    // Get the full path to the selected key
    std::vector<std::string> pathParts;
    HTREEITEM currentItem = hItem;

    while (currentItem) {
        char itemText[256];
        TVITEM item;
        item.mask = TVIF_TEXT;
        item.hItem = currentItem;
        item.pszText = itemText;
        item.cchTextMax = sizeof(itemText);

        if (TreeView_GetItem(hTreeView, &item)) {
            pathParts.insert(pathParts.begin(), std::string(itemText));
        }

        currentItem = TreeView_GetParent(hTreeView, currentItem);
    }

    if (pathParts.size() > 1) {
        std::string fullPath;
        for (size_t i = 1; i < pathParts.size(); i++) {
            if (i > 1) fullPath += "\\\\";
            fullPath += pathParts[i];
        }

        SetWindowText(hEditKeyPath, fullPath);

        // Find the root key
        HKEY rootKey = HKEY_CURRENT_USER;
        for (int i = 0; i < ROOT_KEYS_COUNT; i++) {
            if (pathParts[0] == rootKeys[i].displayName) {
                rootKey = rootKeys[i].hKey;
                break;
            }
        }

        PopulateValuesList(rootKey, fullPath);
    }
}

// Populate values list
void PopulateValuesList(HKEY hKey, const std::string& keyPath) {
    ListView_DeleteAllItems(hListView);

    HKEY hSubKey;
    if (RegOpenKeyEx(hKey, keyPath.c_str(), 0, KEY_READ, &hSubKey) != ERROR_SUCCESS) {
        return;
    }

    char valueName[256];
    DWORD valueNameSize = sizeof(valueName);
    DWORD valueType;
    BYTE valueData[1024];
    DWORD valueDataSize = sizeof(valueData);
    DWORD index = 0;

    while (RegEnumValue(hSubKey, index++, valueName, &valueNameSize, nullptr, &valueType, valueData, &valueDataSize) == ERROR_SUCCESS) {
        LVITEM lvi;
        lvi.mask = LVIF_TEXT;
        lvi.iItem = index - 1;
        lvi.iSubItem = 0;
        lvi.pszText = valueName;

        int itemIndex = ListView_InsertItem(hListView, &lvi);

        // Set type
        const char* typeStr = "Unknown";
        switch (valueType) {
        case REG_SZ: typeStr = "REG_SZ"; break;
        case REG_DWORD: typeStr = "REG_DWORD"; break;
        case REG_BINARY: typeStr = "REG_BINARY"; break;
        case REG_EXPAND_SZ: typeStr = "REG_EXPAND_SZ"; break;
        case REG_MULTI_SZ: typeStr = "REG_MULTI_SZ"; break;
        }

        ListView_SetItemText(hListView, itemIndex, 1, (char*)typeStr);

        // Set data
        std::string dataStr;
        if (valueType == REG_SZ || valueType == REG_EXPAND_SZ) {
            dataStr = (char*)valueData;
        } else if (valueType == REG_DWORD) {
            dataStr = std::to_string(*(DWORD*)valueData);
        } else {
            dataStr = "[Binary Data]";
        }

        ListView_SetItemText(hListView, itemIndex, 2, (char*)dataStr.c_str());

        valueNameSize = sizeof(valueName);
        valueDataSize = sizeof(valueData);
    }

    RegCloseKey(hSubKey);
}

// Create registry key
void CreateRegistryKey() {
    std::string keyPath = GetWindowText(hEditKeyPath);
    if (keyPath.empty()) {
        MessageBox(hMainWindow, "Please enter a key path!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    HKEY rootKey = GetSelectedRootKey();
    HKEY hKey;
    DWORD disposition;

    // Check if key already exists (Group 3 additional requirement)
    if (KeyExists(rootKey, keyPath)) {
        int result = MessageBox(hMainWindow,
            "Key already exists! Do you want to update it?",
            "Key Exists", MB_YESNO | MB_ICONQUESTION);
        if (result != IDYES) {
            return;
        }
    }

    LONG result = RegCreateKeyEx(rootKey, keyPath.c_str(), 0, nullptr,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, &disposition);

    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        if (disposition == REG_CREATED_NEW_KEY) {
            UpdateStatusBar("Registry key created successfully: " + keyPath);
        } else {
            UpdateStatusBar("Registry key opened for update: " + keyPath);
        }
        PopulateTreeView();
    } else {
        UpdateStatusBar("Failed to create registry key. Error: " + std::to_string(result));
        MessageBox(hMainWindow, "Failed to create registry key!", "Error", MB_OK | MB_ICONERROR);
    }
}

// Delete registry key
void DeleteRegistryKey() {
    std::string keyPath = GetWindowText(hEditKeyPath);
    if (keyPath.empty()) {
        MessageBox(hMainWindow, "Please enter a key path!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    int result = MessageBox(hMainWindow,
        ("Are you sure you want to delete the key: " + keyPath + "?").c_str(),
        "Confirm Delete", MB_YESNO | MB_ICONWARNING);

    if (result != IDYES) return;

    HKEY rootKey = GetSelectedRootKey();
    LONG regResult = RegDeleteKey(rootKey, keyPath.c_str());

    if (regResult == ERROR_SUCCESS) {
        UpdateStatusBar("Registry key deleted successfully: " + keyPath);
        PopulateTreeView();
        ListView_DeleteAllItems(hListView);
    } else {
        UpdateStatusBar("Failed to delete registry key. Error: " + std::to_string(regResult));
        MessageBox(hMainWindow, "Failed to delete registry key!", "Error", MB_OK | MB_ICONERROR);
    }
}

// Set registry value
void SetRegistryValue() {
    std::string keyPath = GetWindowText(hEditKeyPath);
    std::string valueName = GetWindowText(hEditValueName);
    std::string valueData = GetWindowText(hEditValueData);

    if (keyPath.empty() || valueName.empty()) {
        MessageBox(hMainWindow, "Please enter key path and value name!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    HKEY rootKey = GetSelectedRootKey();
    HKEY hKey;

    LONG result = RegOpenKeyEx(rootKey, keyPath.c_str(), 0, KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS) {
        UpdateStatusBar("Failed to open registry key for writing. Error: " + std::to_string(result));
        MessageBox(hMainWindow, "Failed to open registry key!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    result = RegSetValueEx(hKey, valueName.c_str(), 0, REG_SZ,
        (const BYTE*)valueData.c_str(), valueData.length() + 1);

    RegCloseKey(hKey);

    if (result == ERROR_SUCCESS) {
        UpdateStatusBar("Registry value set successfully: " + valueName);
        OnTreeSelectionChanged(); // Refresh values list
    } else {
        UpdateStatusBar("Failed to set registry value. Error: " + std::to_string(result));
        MessageBox(hMainWindow, "Failed to set registry value!", "Error", MB_OK | MB_ICONERROR);
    }
}

// Delete registry value
void DeleteRegistryValue() {
    std::string keyPath = GetWindowText(hEditKeyPath);
    std::string valueName = GetWindowText(hEditValueName);

    if (keyPath.empty() || valueName.empty()) {
        MessageBox(hMainWindow, "Please enter key path and value name!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    int result = MessageBox(hMainWindow,
        ("Are you sure you want to delete the value: " + valueName + "?").c_str(),
        "Confirm Delete", MB_YESNO | MB_ICONWARNING);

    if (result != IDYES) return;

    HKEY rootKey = GetSelectedRootKey();
    HKEY hKey;

    LONG regResult = RegOpenKeyEx(rootKey, keyPath.c_str(), 0, KEY_WRITE, &hKey);
    if (regResult == ERROR_SUCCESS) {
        regResult = RegDeleteValue(hKey, valueName.c_str());
        RegCloseKey(hKey);

        if (regResult == ERROR_SUCCESS) {
            UpdateStatusBar("Registry value deleted successfully: " + valueName);
            OnTreeSelectionChanged(); // Refresh values list
        } else {
            UpdateStatusBar("Failed to delete registry value. Error: " + std::to_string(regResult));
            MessageBox(hMainWindow, "Failed to delete registry value!", "Error", MB_OK | MB_ICONERROR);
        }
    } else {
        UpdateStatusBar("Failed to open registry key. Error: " + std::to_string(regResult));
        MessageBox(hMainWindow, "Failed to open registry key!", "Error", MB_OK | MB_ICONERROR);
    }
}

// Save registry to file
void SaveRegistryToFile() {
    OPENFILENAME ofn;
    char szFile[260] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hMainWindow;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Registry Files\\0*.reg\\0Text Files\\0*.txt\\0All Files\\0*.*\\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        std::string keyPath = GetWindowText(hEditKeyPath);
        if (keyPath.empty()) {
            MessageBox(hMainWindow, "Please select a key to save!", "Error", MB_OK | MB_ICONERROR);
            return;
        }

        std::ofstream file(szFile);
        if (file.is_open()) {
            file << "Windows Registry Editor Version 5.00\
\
";

            HKEY rootKey = GetSelectedRootKey();
            std::string rootKeyName;
            for (int i = 0; i < ROOT_KEYS_COUNT; i++) {
                if (rootKeys[i].hKey == rootKey) {
                    rootKeyName = rootKeys[i].name;
                    break;
                }
            }

            file << "[" << rootKeyName << "\\\\" << keyPath << "]";

            // Save values
            HKEY hKey;
            if (RegOpenKeyEx(rootKey, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                char valueName[256];
                DWORD valueNameSize = sizeof(valueName);
                DWORD valueType;
                BYTE valueData[1024];
                DWORD valueDataSize = sizeof(valueData);
                DWORD index = 0;

                while (RegEnumValue(hKey, index++, valueName, &valueNameSize, nullptr, &valueType, valueData, &valueDataSize) == ERROR_SUCCESS) {
                    file << "\"" << valueName << "\"=";

                    if (valueType == REG_SZ) {
                        file << "\"" << (char*)valueData << "\"";
                    } else if (valueType == REG_DWORD) {
                        file << "dword:" << std::hex << *(DWORD*)valueData << "";
                    }

                    valueNameSize = sizeof(valueName);
                    valueDataSize = sizeof(valueData);
                }

                RegCloseKey(hKey);
            }

            file.close();
            UpdateStatusBar("Registry saved to file: " + std::string(szFile));
        } else {
            MessageBox(hMainWindow, "Failed to create file!", "Error", MB_OK | MB_ICONERROR);
        }
    }
}

// Load registry from file
void LoadRegistryFromFile() {
    OPENFILENAME ofn;
    char szFile[260] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hMainWindow;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Registry Files\\0*.reg\\0Text Files\\0*.txt\\0All Files\\0*.*\\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        std::ifstream file(szFile);
        if (file.is_open()) {
            std::string line;
            bool success = false;

            while (std::getline(file, line)) {
                if (line.find("Windows Registry Editor") != std::string::npos) {
                    success = true;
                    break;
                }
            }

            file.close();

            if (success) {
                UpdateStatusBar("Registry file loaded: " + std::string(szFile));
                MessageBox(hMainWindow, "Registry file loaded successfully!\
Note: This is a simplified implementation.", "Success", MB_OK | MB_ICONINFORMATION);
                PopulateTreeView();
            } else {
                MessageBox(hMainWindow, "Invalid registry file format!", "Error", MB_OK | MB_ICONERROR);
            }
        } else {
            MessageBox(hMainWindow, "Failed to open file!", "Error", MB_OK | MB_ICONERROR);
        }
    }
}

// Check if key exists (Group 3 additional feature)
void CheckKeyExists() {
    std::string keyPath = GetWindowText(hEditKeyPath);
    if (keyPath.empty()) {
        MessageBox(hMainWindow, "Please enter a key path!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    HKEY rootKey = GetSelectedRootKey();
    bool exists = KeyExists(rootKey, keyPath);

    std::string message = "Key '" + keyPath + "' ";
    message += exists ? "EXISTS" : "DOES NOT EXIST";

    MessageBox(hMainWindow, message.c_str(), "Key Check Result",
        MB_OK | (exists ? MB_ICONINFORMATION : MB_ICONWARNING));

    UpdateStatusBar(message);
}

    // Check if registry key exists
    bool KeyExists(HKEY rootKey, const std::string& keyPath) {
    HKEY hKey;
    LONG result = RegOpenKeyEx(rootKey, keyPath.c_str(), 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

    // Update status bar
    void UpdateStatusBar(const std::string& message) {
    SetWindowText(hStatusBar, message.c_str());
}

    // Get selected root key from combo box
    HKEY GetSelectedRootKey() {
    int index = SendMessage(hComboRootKey, CB_GETCURSEL, 0, 0);
    if (index >= 0 && index < ROOT_KEYS_COUNT) {
        return rootKeys[index].hKey;
    }
    return HKEY_CURRENT_USER;
}

    // Get selected key path from edit control
    std::string GetSelectedKeyPath() {
    return GetWindowText(hEditKeyPath);
}

    // Helper function to get text from window
    std::string GetWindowText(HWND hwnd) {
    int length = ::GetWindowTextLength(hwnd);
    if (length == 0) return "";

    std::vector<char> buffer(length + 1);
    ::GetWindowText(hwnd, buffer.data(), length + 1);
    return std::string(buffer.data());
}

    // Helper function to set text to window
    void SetWindowText(HWND hwnd, const std::string& text) {
    ::SetWindowText(hwnd, text.c_str());
}