#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

class CommandFileCreator {
private:
    std::string windowsScript;
    std::string linuxScript;
    
public:
    CommandFileCreator() {
        CreateWindowsScript();
        CreateLinuxScript();
    }
    
    void CreateWindowsScript() {
        windowsScript = R"(@echo off
REM Lab 6 - Variant 3: Command Files Creation for Windows
REM Author: Student
REM Date: 2025

echo Starting Lab 6 - Windows Command Script Execution...
echo.

REM 1. Creating folder tree structure
echo Creating folder structure...
if not exist "main1" mkdir main1

REM Creating subdirectories according to diagram
if not exist "main1\main2" mkdir main1\main2
if not exist "main1\main3" mkdir main1\main3
if not exist "main1\main2\1" mkdir main1\main2\1
if not exist "main1\main3\4" mkdir main1\main3\4
if not exist "main1\main3\7" mkdir main1\main3\7
if not exist "main1\main2\1\2" mkdir main1\main2\1\2
if not exist "main1\main2\1\10" mkdir main1\main2\1\10
if not exist "main1\main2\1\2\3" mkdir main1\main2\1\2\3
if not exist "main1\main3\4\5" mkdir main1\main3\4\5
if not exist "main1\main3\4\6" mkdir main1\main3\4\6
if not exist "main1\main3\7\8" mkdir main1\main3\7\8
if not exist "main1\main3\7\9" mkdir main1\main3\7\9

REM Creating bin directory for copying files
if not exist "bin" mkdir bin

echo Folder structure created successfully!
echo.

REM 2. Create some sample .exe files for demonstration
echo Creating sample .exe files in main1 folder...
copy nul main1\sample1.exe >nul 2>&1
copy nul main1\sample2.exe >nul 2>&1
copy nul main1\test.exe >nul 2>&1
echo Sample .exe files created in main1 folder.
echo.

REM 3. Loop for outputting numbers from 20 to 85 with step 5
echo Outputting numbers from 20 to 85 with step 5:
for /L %%i in (20,5,85) do (
    echo Number: %%i
)
echo.

REM 4. Display folder tree structure
echo Current folder structure:
tree /F
echo.

REM 5. Copying .exe files from main1 to bin folder
echo Copying .exe files from main1 to bin folder...
if exist "main1\*.exe" (
    copy main1\*.exe bin\ >nul 2>&1
    echo .exe files copied successfully from main1 to bin!
) else (
    echo No .exe files found in main1 folder.
)
echo.

REM 6. List files in bin folder
echo Files in bin folder:
if exist "bin\*.*" (
    dir bin /B
) else (
    echo No files found in bin folder.
)
echo.

REM 7. Opening .exe files on drive C (simulation - listing instead of opening all)
echo Searching for .exe files on drive C: (showing first 10 for safety)
dir C:\*.exe /S /B 2>nul | findstr /R ".*" | more +1 | head -10
echo.
echo Note: For safety, only listing .exe files instead of opening them.
echo To actually open files, uncomment the following lines:
REM for /R C:\ %%f in (*.exe) do (
REM     echo Opening: %%f
REM     start "" "%%f"
REM )

echo.
echo Lab 6 Windows script execution completed!
echo Press any key to continue...
pause >nul
)";
    }
    
    void CreateLinuxScript() {
        linuxScript = R"DELIMITER(#!/bin/bash
# Lab 6 - Variant 3: Command Files Creation for Linux
# Author: Student
# Date: 2025

echo "Starting Lab 6 - Linux Shell Script Execution..."
echo

# 1. Creating folder tree structure
echo "Creating folder structure..."
# Creating subdirectories according to diagram
mkdir -p main1/main2/1/2/3
mkdir -p main1/main2/1/10
mkdir -p main1/main3/4/5
mkdir -p main1/main3/4/6
mkdir -p main1/main3/7/8
mkdir -p main1/main3/7/9

# Creating bin directory for copying files
mkdir -p bin

echo "Folder structure created successfully!"
echo

# 2. Create some sample executable files for demonstration
echo "Creating sample executable files in main1 folder..."
touch main1/sample1.exe
touch main1/sample2.exe
touch main1/test.exe
chmod +x main1/*.exe
echo "Sample executable files created in main1 folder."
echo

# 3. Loop for outputting numbers from 20 to 85 with step 5
echo "Outputting numbers from 20 to 85 with step 5:"
for i in $(seq 20 5 85); do
    echo "Number: $i"
done
echo

# 4. Display folder tree structure
echo "Current folder structure:"
if command -v tree &> /dev/null; then
    tree
else
    find . -type d | sed -e "s/[^-][^\/]*\//  |/g" -e "s/|\([^ ]\)/|-\1/"
fi
echo

# 5. Copying .exe files from main1 to bin folder
echo "Copying .exe files from main1 to bin folder..."
if ls main1/*.exe 1> /dev/null 2>&1; then
    cp main1/*.exe bin/
    echo ".exe files copied successfully from main1 to bin!"
else
    echo "No .exe files found in main1 folder."
fi
echo

# 6. List files in bin folder
echo "Files in bin folder:"
if [ -n "$(ls -A bin 2>/dev/null)" ]; then
    ls -la bin/
else
    echo "No files found in bin folder."
fi
echo

# 7. Searching for executable files (simulation - safer than opening all)
echo "Searching for executable files in /usr/bin (showing first 10):"
find /usr/bin -name "*.exe" -o -name "*" -type f -executable 2>/dev/null | head -10
echo
echo "Note: For safety, only listing executable files instead of opening them."
echo "To actually open files with default applications, uncomment the following lines:"
# find /usr/bin -name "*.exe" -type f 2>/dev/null | while read file; do
#     echo "Opening: $file"
#     xdg-open "$file" 2>/dev/null &
# done

echo
echo "Lab 6 Linux script execution completed!"
read -p "Press Enter to continue..."
)DELIMITER";
    }
    
    void SaveWindowsScript(const std::string& filename = "create_structure.cmd") {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << windowsScript;
            file.close();
            std::cout << "Windows script saved as: " << filename << std::endl;
        } else {
            std::cerr << "Error: Could not create Windows script file!" << std::endl;
        }
    }
    
    void SaveLinuxScript(const std::string& filename = "create_structure.sh") {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << linuxScript;
            file.close();
            
            // Make the script executable on Unix-like systems
            std::filesystem::permissions(filename, 
                std::filesystem::perms::owner_all | 
                std::filesystem::perms::group_read | 
                std::filesystem::perms::group_exec |
                std::filesystem::perms::others_read | 
                std::filesystem::perms::others_exec);
                
            std::cout << "Linux script saved as: " << filename << " (made executable)" << std::endl;
        } else {
            std::cerr << "Error: Could not create Linux script file!" << std::endl;
        }
    }
    
    void DisplayScriptContents() {
        std::cout << "\n=== WINDOWS SCRIPT CONTENT ===" << std::endl;
        std::cout << windowsScript << std::endl;
        
        std::cout << "\n=== LINUX SCRIPT CONTENT ===" << std::endl;
        std::cout << linuxScript << std::endl;
    }
    
    void CreateFolderStructure() {
        try {
            // Create the folder structure as shown in variant 3 diagram
            // main1 → main2, main3
            // main2 → 1 → 2,10
            // main3 → 4,7
            // 2 → 3
            // 4 → 5,6
            // 7 → 8,9
            std::filesystem::create_directories("main1/main2/1/2/3");
            std::filesystem::create_directories("main1/main2/1/10");
            std::filesystem::create_directories("main1/main3/4/5");
            std::filesystem::create_directories("main1/main3/4/6");
            std::filesystem::create_directories("main1/main3/7/8");
            std::filesystem::create_directories("main1/main3/7/9");
            std::filesystem::create_directories("bin");
            
            std::cout << "C++ implementation: Folder structure created successfully!" << std::endl;
            std::cout << "Structure: main1 → main2/main3 → numbered subdirectories" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error creating folder structure: " << e.what() << std::endl;
        }
    }
    
    void NumberLoop() {
        std::cout << "\nC++ implementation: Numbers from 20 to 85 with step 5:" << std::endl;
        for (int i = 20; i <= 85; i += 5) {
            std::cout << "Number: " << i << std::endl;
        }
    }
};

void ShowMenu() {
    std::cout << "\n=== Lab 6 - Command Files Creation (Variant 3) ===" << std::endl;
    std::cout << "1. Create Windows script (.cmd)" << std::endl;
    std::cout << "2. Create Linux script (.sh)" << std::endl;
    std::cout << "3. Create both scripts" << std::endl;
    std::cout << "4. Display script contents" << std::endl;
    std::cout << "5. Test C++ folder creation" << std::endl;
    std::cout << "6. Test C++ number loop" << std::endl;
    std::cout << "7. Exit" << std::endl;
    std::cout << "Enter your choice (1-7): ";
}

int main() {
    CommandFileCreator creator;
    int choice;
    
    std::cout << "Lab 6 - Creating Command Files for Windows and Linux" << std::endl;
    std::cout << "Variant 3 Implementation" << std::endl;
    std::cout << "========================================" << std::endl;
    
    while (true) {
        ShowMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                creator.SaveWindowsScript();
                std::cout << "\nTo run the Windows script:" << std::endl;
                std::cout << "  create_structure.cmd" << std::endl;
                break;
                
            case 2:
                creator.SaveLinuxScript();
                std::cout << "\nTo run the Linux script:" << std::endl;
                std::cout << "  ./create_structure.sh" << std::endl;
                break;
                
            case 3:
                creator.SaveWindowsScript();
                creator.SaveLinuxScript();
                std::cout << "\nBoth scripts created successfully!" << std::endl;
                std::cout << "\nTo run:" << std::endl;
                std::cout << "  Windows: create_structure.cmd" << std::endl;
                std::cout << "  Linux:   ./create_structure.sh" << std::endl;
                break;
                
            case 4:
                creator.DisplayScriptContents();
                break;
                
            case 5:
                creator.CreateFolderStructure();
                break;
                
            case 6:
                creator.NumberLoop();
                break;
                
            case 7:
                std::cout << "Exiting Lab 6 program. Goodbye!" << std::endl;
                return 0;
                
            default:
                std::cout << "Invalid choice! Please enter 1-7." << std::endl;
                break;
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
    
    return 0;
}
