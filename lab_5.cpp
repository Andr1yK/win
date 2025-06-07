#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <conio.h>

using namespace std;

// Global variables
bool timerRunning = false;
bool alarmActive = false;
vector<string> exerciseNames = {"Push-ups", "Squats", "Plank", "Jumping Jacks", "Burpees"};

// Function to display error messages
void DisplayError(const string& message) {
    DWORD error = GetLastError();
    cout << message << " (Error code: " << error << ")" << endl;
}

// Function to get current system time
void DisplayCurrentTime() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    cout << "Current system time: "
         << setfill('0') << setw(2) << st.wDay << "/"
         << setfill('0') << setw(2) << st.wMonth << "/"
         << st.wYear << " "
         << setfill('0') << setw(2) << st.wHour << ":"
         << setfill('0') << setw(2) << st.wMinute << ":"
         << setfill('0') << setw(2) << st.wSecond << endl;
}

// Task 1: Exercise Timer with countdown
void ExerciseTimer() {
    int exerciseTime, restTime, totalExercises;

    cout << "\n===== EXERCISE TIMER =====\n";
    cout << "Enter exercise duration (seconds): ";
    cin >> exerciseTime;
    cout << "Enter rest time between exercises (seconds): ";
    cin >> restTime;
    cout << "Enter number of exercises: ";
    cin >> totalExercises;

    if (totalExercises > exerciseNames.size()) {
        totalExercises = exerciseNames.size();
        cout << "Limited to " << exerciseNames.size() << " exercises.\n";
    }

    cout << "\nStarting workout in 3 seconds...\n";
    Sleep(3000);

    timerRunning = true;

    for (int i = 0; i < totalExercises && timerRunning; i++) {
        cout << "\n=== Exercise " << (i + 1) << "/" << totalExercises
             << ": " << exerciseNames[i] << " ===\n";

        // Exercise countdown
        for (int time = exerciseTime; time > 0 && timerRunning; time--) {
            cout << "\rTime remaining: " << time << " seconds   ";
            cout.flush();

            // Check for ESC key to stop timer
            if (_kbhit() && _getch() == 27) { // ESC key
                timerRunning = false;
                cout << "\n\nTimer stopped by user!\n";
                return;
            }

            Sleep(1000);
        }

        if (!timerRunning) break;

        // Play completion sound
        Beep(1000, 300);
        cout << "\nExercise completed!\n";

        // Rest period (except after last exercise)
        if (i < totalExercises - 1) {
            cout << "\nRest time:\n";
            for (int time = restTime; time > 0 && timerRunning; time--) {
                cout << "\rRest: " << time << " seconds   ";
                cout.flush();

                if (_kbhit() && _getch() == 27) {
                    timerRunning = false;
                    cout << "\n\nTimer stopped by user!\n";
                    return;
                }

                Sleep(1000);
            }
            cout << "\nGet ready for next exercise!\n";
            Beep(800, 200);
        }
    }

    if (timerRunning) {
        cout << "\n\n🎉 WORKOUT COMPLETED! Great job! 🎉\n";
        // Play completion melody
        Beep(523, 300); // C
        Beep(659, 300); // E
        Beep(784, 300); // G
        Beep(1047, 600); // High C
    }

    timerRunning = false;
}

// Task 2: Morning Alarm with Snooze Function
void MorningAlarm() {
    int hour, minute, snoozeMinutes;

    cout << "\n===== MORNING ALARM =====\n";
    cout << "Set alarm time:\n";
    cout << "Hour (0-23): ";
    cin >> hour;
    cout << "Minute (0-59): ";
    cin >> minute;
    cout << "Snooze duration (minutes): ";
    cin >> snoozeMinutes;

    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        cout << "Invalid time format!\n";
        return;
    }

    cout << "Alarm set for " << setfill('0') << setw(2) << hour
         << ":" << setfill('0') << setw(2) << minute << "\n";
    cout << "Press ESC to cancel alarm.\n";

    alarmActive = true;

    while (alarmActive) {
        SYSTEMTIME st;
        GetLocalTime(&st);

        // Check if alarm time is reached
        if (st.wHour == hour && st.wMinute == minute && st.wSecond == 0) {
            cout << "\n🔔 WAKE UP! ALARM IS RINGING! 🔔\n";

            // Ring alarm for 30 seconds or until user responds
            auto startTime = chrono::steady_clock::now();
            bool snoozed = false;

            while (chrono::duration_cast<chrono::seconds>(
                chrono::steady_clock::now() - startTime).count() < 30) {

                // Alternate beep sound
                Beep(800, 500);
                Beep(1000, 500);

                cout << "\nPress 'S' for Snooze or 'O' to turn Off: ";

                if (_kbhit()) {
                    char choice = toupper(_getch());
                    if (choice == 'S') {
                        // Snooze alarm
                        minute += snoozeMinutes;
                        if (minute >= 60) {
                            hour += minute / 60;
                            minute = minute % 60;
                            if (hour >= 24) hour = hour % 24;
                        }

                        cout << "\nAlarm snoozed for " << snoozeMinutes
                             << " minutes. Next alarm: "
                             << setfill('0') << setw(2) << hour
                             << ":" << setfill('0') << setw(2) << minute << "\n";
                        snoozed = true;
                        break;
                    } else if (choice == 'O') {
                        cout << "\nAlarm turned off. Have a great day!\n";
                        alarmActive = false;
                        return;
                    }
                }
            }

            if (!snoozed && alarmActive) {
                cout << "\nAlarm automatically stopped after 30 seconds.\n";
                alarmActive = false;
                return;
            }
        }

        // Check for ESC key to cancel alarm
        if (_kbhit() && _getch() == 27) {
            cout << "\nAlarm cancelled by user.\n";
            alarmActive = false;
            return;
        }

        Sleep(1000); // Check every second
    }
}

// Task 3: System Time Backward Adjustment Timer
void SystemTimeBackwardTimer() {
    cout << "\n===== SYSTEM TIME BACKWARD TIMER =====\n";
    cout << "WARNING: This function requires administrator privileges!\n";
    cout << "Current system time will be adjusted backward.\n\n";

    DisplayCurrentTime();

    int minutesBack;
    cout << "\nEnter minutes to go back (1-60): ";
    cin >> minutesBack;

    if (minutesBack < 1 || minutesBack > 60) {
        cout << "Invalid input! Must be between 1 and 60 minutes.\n";
        return;
    }

    cout << "Are you sure you want to change system time "
         << minutesBack << " minutes back? (Y/N): ";
    char confirm;
    cin >> confirm;

    if (toupper(confirm) != 'Y') {
        cout << "Operation cancelled.\n";
        return;
    }

    // Get current system time
    SYSTEMTIME st;
    GetSystemTime(&st); // Get UTC time

    // Store original time for display
    SYSTEMTIME originalTime = st;

    // Calculate new time (subtract minutes)
    FILETIME ft, newFt;
    SystemTimeToFileTime(&st, &ft);

    // Convert to large integer for calculation
    LARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    // Subtract minutes (in 100-nanosecond intervals)
    // 1 minute = 60 seconds * 10,000,000 (100-nanosecond intervals)
    li.QuadPart -= (long long)minutesBack * 60 * 10000000;

    // Convert back to FILETIME and SYSTEMTIME
    newFt.dwLowDateTime = li.LowPart;
    newFt.dwHighDateTime = li.HighPart;

    SYSTEMTIME newSt;
    FileTimeToSystemTime(&newFt, &newSt);

    // Attempt to set new system time
    if (SetSystemTime(&newSt)) {
        cout << "\n✅ System time successfully changed!\n";
        cout << "Previous time (UTC): "
             << setfill('0') << setw(2) << originalTime.wDay << "/"
             << setfill('0') << setw(2) << originalTime.wMonth << "/"
             << originalTime.wYear << " "
             << setfill('0') << setw(2) << originalTime.wHour << ":"
             << setfill('0') << setw(2) << originalTime.wMinute << ":"
             << setfill('0') << setw(2) << originalTime.wSecond << "\n";

        cout << "New time (UTC): "
             << setfill('0') << setw(2) << newSt.wDay << "/"
             << setfill('0') << setw(2) << newSt.wMonth << "/"
             << newSt.wYear << " "
             << setfill('0') << setw(2) << newSt.wHour << ":"
             << setfill('0') << setw(2) << newSt.wMinute << ":"
             << setfill('0') << setw(2) << newSt.wSecond << "\n";

        cout << "\nCurrent local time:\n";
        DisplayCurrentTime();

        // Ask if user wants to restore original time
        cout << "\nDo you want to restore the original time? (Y/N): ";
        cin >> confirm;

        if (toupper(confirm) == 'Y') {
            if (SetSystemTime(&originalTime)) {
                cout << "✅ Original time restored successfully!\n";
                DisplayCurrentTime();
            } else {
                DisplayError("❌ Failed to restore original time");
            }
        }

    } else {
        DisplayError("❌ Failed to change system time");
        cout << "Make sure to run the program as Administrator!\n";
    }
}

// Function to display current time continuously
void DisplayTimeMonitor() {
    cout << "\n===== TIME MONITOR =====\n";
    cout << "Press any key to stop monitoring...\n\n";

    while (!_kbhit()) {
        cout << "\r";
        DisplayCurrentTime();
        Sleep(1000);
    }
    _getch(); // Consume the key press
    cout << "\nTime monitoring stopped.\n";
}

// Main menu
void ShowMenu() {
    cout << "\n===== LAB 5 - SYSTEM TIME & TIMER MANAGEMENT =====\n";
    cout << "Variant 3 Tasks:\n";
    cout << "1. Exercise Timer (Countdown for fitness exercises)\n";
    cout << "2. Morning Alarm (with snooze function)\n";
    cout << "3. System Time Backward Adjustment\n";
    cout << "4. Display Current Time\n";
    cout << "5. Time Monitor (real-time display)\n";
    cout << "0. Exit\n";
    cout << "\nEnter your choice: ";
}

int main() {
    cout << "=== Windows System Time and Timer Management ===\n";
    cout << "Lab 5 - Variant 3 Implementation\n";

    int choice;
    bool running = true;

    while (running) {
        ShowMenu();
        cin >> choice;

        switch (choice) {
            case 1:
                ExerciseTimer();
                break;
            case 2:
                MorningAlarm();
                break;
            case 3:
                SystemTimeBackwardTimer();
                break;
            case 4:
                cout << "\n";
                DisplayCurrentTime();
                break;
            case 5:
                DisplayTimeMonitor();
                break;
            case 0:
                running = false;
                cout << "Goodbye!\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }

        if (running && choice != 0) {
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
            system("cls");
        }
    }

    // Cleanup
    timerRunning = false;
    alarmActive = false;

    return 0;
}
