#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <iomanip>
#include <psapi.h>
#include <thread>
#include <conio.h>

// Global variables
std::vector<DWORD> processIds;
HANDLE currentProcessHandle = NULL;
bool autoRefreshRunning = false;

// Function to display WinAPI error
void DisplayError(const std::string& message) {
    DWORD error = GetLastError();
    std::cout << message << " (Error code: " << error << ")" << std::endl;
}

// 1. Function to create a new process
void CreateNewProcess(const std::string& processPath) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create a copy of the path for use in CreateProcess
    char* processPathCopy = new char[processPath.length() + 1];
    strcpy_s(processPathCopy, processPath.length() + 1, processPath.c_str());

    std::cout << "Starting process: " << processPath << std::endl;

    // Create the process
    if (!CreateProcess(
        NULL,           // Executable name (NULL because we specify it in the command line)
        processPathCopy, // Command line
        NULL,           // Process security attributes
        NULL,           // Thread security attributes
        FALSE,          // Handle inheritance
        0,              // Creation flags
        NULL,           // Parent process environment
        NULL,           // Current directory
        &si,            // Startup information
        &pi             // Process information
    )) {
        DisplayError("Error creating process");
        delete[] processPathCopy;
        return;
    }

    std::cout << "Process created successfully!" << std::endl;
    std::cout << "Process ID: " << pi.dwProcessId << std::endl;
    std::cout << "Primary thread ID: " << pi.dwThreadId << std::endl;

    // Close handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    delete[] processPathCopy;
}

// 2. Function to list all processes
void ListAllProcesses() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        DisplayError("Failed to create process snapshot");
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Get the first process
    if (!Process32First(hSnapshot, &pe32)) {
        DisplayError("Failed to get information about the first process");
        CloseHandle(hSnapshot);
        return;
    }

    // Clear the process list
    processIds.clear();

    // Print header
    std::cout << std::left << std::setw(6) << "#"
              << std::setw(10) << "PID"
              << std::setw(40) << "Process Name"
              << std::setw(15) << "Thread Count" << std::endl;
    std::cout << std::string(71, '-') << std::endl;

    int index = 0;
    // Iterate through all processes
    do {
        processIds.push_back(pe32.th32ProcessID);
        std::cout << std::left << std::setw(6) << index
                  << std::setw(10) << pe32.th32ProcessID
                  << std::setw(40) << pe32.szExeFile
                  << std::setw(15) << pe32.cntThreads << std::endl;
        index++;
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
}

// Function to automatically refresh the process list at regular intervals
void AutoRefreshProcesses() {
    std::cout << "Starting automatic refresh of process list. Press any key to stop." << std::endl;
    autoRefreshRunning = true;

    // Create a separate thread to check for key presses
    std::thread keyCheckThread([]() {
        while (autoRefreshRunning) {
            if (_kbhit()) { // Check if a key was pressed
                _getch(); // Consume the key
                autoRefreshRunning = false;
                break;
            }
            Sleep(100); // Short sleep to avoid high CPU usage
        }
    });
    keyCheckThread.detach(); // Detach the thread

    while (autoRefreshRunning) {
        system("cls"); // Clear screen
        std::cout << "Automatic process list refresh (press any key to stop)" << std::endl;
        ListAllProcesses(); // List all processes
        Sleep(2000); // Wait 2 seconds before next refresh
    }

    std::cout << "Automatic refresh stopped." << std::endl;
}

// 3. Function to terminate a selected process
void TerminateSelectedProcess(int processId) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);

    std::cout << "after OpenProcess" << std::endl;

    if (hProcess == NULL) {
        DisplayError("Failed to open process");
        return;
    }

    std::cout << "hProcess is not null" << std::endl;

    if (!TerminateProcess(hProcess, 0)) {
        DisplayError("Failed to terminate process");
        CloseHandle(hProcess);
        return;
    }

    std::cout << "Process with PID " << processId << " successfully terminated." << std::endl;
    CloseHandle(hProcess);
}

// 4. Function to list information about all threads of a selected process (Group 1 task)
void ListProcessThreads(int index) {
    if (index < 0 || index >= processIds.size()) {
        std::cout << "Invalid process index." << std::endl;
        return;
    }

    DWORD processId = processIds[index];
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        DisplayError("Failed to create thread snapshot");
        return;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    // Get the first thread
    if (!Thread32First(hSnapshot, &te32)) {
        DisplayError("Failed to get information about the first thread");
        CloseHandle(hSnapshot);
        return;
    }

    // Open the process to get detailed information
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (hProcess == NULL) {
        DisplayError("Failed to open process to get thread information");
        CloseHandle(hSnapshot);
        return;
    }

    // Save the current process handle for later use
    currentProcessHandle = hProcess;

    // Print header
    std::cout << "Threads of process with PID " << processId << ":" << std::endl;
    std::cout << std::left << std::setw(15) << "TID"
              << std::setw(15) << "Base Priority"
              << std::setw(20) << "Status" << std::endl;
    std::cout << std::string(50, '-') << std::endl;

    // Iterate through all threads and display those belonging to the selected process
    do {
        if (te32.th32OwnerProcessID == processId) {
            // Get additional information about the thread
            HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
            std::string status = "Unknown";

            if (hThread != NULL) {
                // Get thread status
                DWORD exitCode;
                if (GetExitCodeThread(hThread, &exitCode)) {
                    status = (exitCode == STILL_ACTIVE) ? "Active" : "Terminated";
                }

                std::cout << std::left << std::setw(15) << te32.th32ThreadID
                          << std::setw(15) << te32.tpBasePri
                          << std::setw(20) << status << std::endl;

                CloseHandle(hThread);
            }
            else {
                std::cout << std::left << std::setw(15) << te32.th32ThreadID
                          << std::setw(15) << te32.tpBasePri
                          << std::setw(20) << "No access" << std::endl;
            }
        }
    } while (Thread32Next(hSnapshot, &te32));

    CloseHandle(hSnapshot);
}

// 5. Function to list information about all modules of a selected process
void ListProcessModules(int index) {
    if (index < 0 || index >= processIds.size()) {
        std::cout << "Invalid process index." << std::endl;
        return;
    }

    DWORD processId = processIds[index];
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);

    if (hProcess == NULL) {
        DisplayError("Failed to open process to get module information");
        return;
    }

    HMODULE hModules[1024];
    DWORD cbNeeded;

    // Get the list of process modules
    if (!EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded)) {
        DisplayError("Failed to get module list");
        CloseHandle(hProcess);
        return;
    }

    // Print header
    std::cout << "Modules of process with PID " << processId << ":" << std::endl;
    std::cout << std::left << std::setw(50) << "Module Name"
              << std::setw(20) << "Base Address" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    // Iterate through all modules
    for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
        char szModName[MAX_PATH];

        // Get the full path to the module
        if (GetModuleFileNameEx(hProcess, hModules[i], szModName, sizeof(szModName))) {
            std::cout << std::left << std::setw(50) << szModName
                      << std::setw(20) << std::hex << std::showbase
                      << reinterpret_cast<uintptr_t>(hModules[i]) << std::dec << std::endl;
        }
    }

    CloseHandle(hProcess);
}

// 6. Function to launch a new process with parameters
void StartProcessWithParameters(const std::string& processPath, const std::string& parameters) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create command line from process and parameters
    std::string commandLine = processPath + " " + parameters;
    char* commandLineCopy = new char[commandLine.length() + 1];
    strcpy_s(commandLineCopy, commandLine.length() + 1, commandLine.c_str());

    std::cout << "Starting process with parameters: " << commandLine << std::endl;

    // Create the process
    if (!CreateProcess(
        NULL,           // Executable name
        commandLineCopy, // Command line
        NULL,           // Process security attributes
        NULL,           // Thread security attributes
        FALSE,          // Handle inheritance
        0,              // Creation flags
        NULL,           // Parent process environment
        NULL,           // Current directory
        &si,            // Startup information
        &pi             // Process information
    )) {
        DisplayError("Error creating process with parameters");
        delete[] commandLineCopy;
        return;
    }

    std::cout << "Process created successfully!" << std::endl;
    std::cout << "Process ID: " << pi.dwProcessId << std::endl;

    // Close handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    delete[] commandLineCopy;
}

// Function to create a new thread inside a selected process (Group 1 additional task)
DWORD WINAPI ThreadFunction(LPVOID lpParam) {
    // This thread just displays a message and exits
    MessageBoxA(NULL, "New thread created successfully!", "Information", MB_OK | MB_ICONINFORMATION);
    return 0;
}

void CreateThreadInProcess() {
    if (currentProcessHandle == NULL) {
        std::cout << "Please select a process first by using the list threads function." << std::endl;
        return;
    }

    // Allocate memory inside the selected process for thread code
    LPVOID remoteCode = VirtualAllocEx(
        currentProcessHandle,
        NULL,
        4096,  // Memory size
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );

    if (remoteCode == NULL) {
        DisplayError("Failed to allocate memory in remote process");
        return;
    }

    // Write thread function code to remote process
    // This example is simplified - in reality we would write the entire machine code
    // But for demonstration we'll use CreateRemoteThread with a function address in a system library

    // Create thread in remote process
    HANDLE hThread = CreateRemoteThread(
        currentProcessHandle,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "Sleep"),
        (LPVOID)10000,  // Argument - sleep for 10 seconds
        0,
        NULL
    );

    if (hThread == NULL) {
        DisplayError("Failed to create thread in remote process");
        VirtualFreeEx(currentProcessHandle, remoteCode, 0, MEM_RELEASE);
        return;
    }

    std::cout << "New thread successfully created in the selected process!" << std::endl;
    std::cout << "Thread ID: " << GetThreadId(hThread) << std::endl;

    // Close thread handle
    CloseHandle(hThread);
}

// Main program menu
void ShowMenu() {
    std::cout << "\n===== Windows Process Manager =====\n";
    std::cout << "1. Create a new process\n";
    std::cout << "2. Show all processes\n";
    std::cout << "3. Auto-refresh process list (real-time monitoring)\n";
    std::cout << "4. Terminate selected process\n";
    std::cout << "5. Show process threads (Group 1 task)\n";
    std::cout << "6. Show process modules\n";
    std::cout << "7. Launch process with parameters\n";
    std::cout << "8. Create a new thread in selected process (Group 1 additional task)\n";
    std::cout << "0. Exit\n";
    std::cout << "Enter your choice: ";
}

int main() {
    int choice;
    bool running = true;

    while (running) {
        ShowMenu();
        std::cin >> choice;
        std::cin.ignore(); // Clear input buffer

        switch (choice) {
            case 1: {
                std::string processPath;
                std::cout << "Enter path to executable file: ";
                std::getline(std::cin, processPath);
                CreateNewProcess(processPath);
                break;
            }
            case 2:
                ListAllProcesses();
                break;
            case 3:
                AutoRefreshProcesses();
                break;
            case 4: {
                int processId;
                std::cout << "Enter process number to terminate: ";
                std::cin >> processId;
                TerminateSelectedProcess(processId);
                break;
            }
            case 5: {
                int index;
                std::cout << "Enter process number to view threads: ";
                std::cin >> index;
                ListProcessThreads(index);
                break;
            }
            case 6: {
                int index;
                std::cout << "Enter process number to view modules: ";
                std::cin >> index;
                ListProcessModules(index);
                break;
            }
            case 7: {
                std::string processPath, parameters;
                std::cout << "Enter path to executable file: ";
                std::getline(std::cin, processPath);
                std::cout << "Enter launch parameters: ";
                std::getline(std::cin, parameters);
                StartProcessWithParameters(processPath, parameters);
                break;
            }
            case 8:
                CreateThreadInProcess();
                break;
            case 0:
                running = false;
                break;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }

        if (running) {
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            system("cls"); // Clear screen
        }
    }

    // Close process handle if it was opened
    if (currentProcessHandle != NULL) {
        CloseHandle(currentProcessHandle);
    }

    // Ensure auto-refresh is stopped if running
    autoRefreshRunning = false;

    return 0;
}
