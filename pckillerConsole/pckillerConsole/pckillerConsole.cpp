#include <Windows.h>
#include <iostream>
#include <iomanip>
#include <cmath>

#define PCKILLER_DEVICE_TYPE 0x8000
#define IOCTL_SHUTDOWN CTL_CODE(PCKILLER_DEVICE_TYPE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CANCEL CTL_CODE(PCKILLER_DEVICE_TYPE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REMAINING CTL_CODE(PCKILLER_DEVICE_TYPE, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)




int wmain(int argc, wchar_t* argv[]) {
    if (argc < 2) {
        std::wcout << L"Usage: pckillerConsole.exe -shutdown <n> | -cancel | -remaining" << std::endl;
        return 1;
    }

    HANDLE hDevice = CreateFile(
        L"\\\\.\\PCKiller",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hDevice == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Failed to open device: " << GetLastError() << std::endl;
        return 1;
    }

    DWORD bytesReturned;
    if (wcscmp(argv[1], L"-shutdown") == 0 && argc == 3) {
        ULONG n = _wtoi(argv[2]);
        if (!DeviceIoControl(hDevice, IOCTL_SHUTDOWN, &n, sizeof(n), NULL, 0, &bytesReturned, NULL)) {
            std::wcerr << L"Failed to send shutdown command: " << GetLastError() << std::endl;
        }

        else {
            std::wcout << L"Shutdown scheduled in " << n << L" seconds!" << std::endl;
        }
    }


    else if (wcscmp(argv[1], L"-cancel") == 0) {
        if (!DeviceIoControl(hDevice, IOCTL_CANCEL, NULL, 0, NULL, 0, &bytesReturned, NULL)) {
            std::wcerr << L"Failed to send cancel command: " << GetLastError() << std::endl;
        }

        else {
            std::wcout << L"Shutdown canceled!" << std::endl;
        }
    }


    else if (wcscmp(argv[1], L"-remaining") == 0) {
        LARGE_INTEGER remainingTime;
        if (!DeviceIoControl(hDevice, IOCTL_REMAINING, NULL, 0, &remainingTime, sizeof(remainingTime), &bytesReturned, NULL)) {
            std::wcerr << L"Failed to get remaining time: " << GetLastError() << std::endl;
        }

        else {
            if (remainingTime.QuadPart == -1) {
                std::wcout << L"No shutdown scheduled!" << std::endl;
            }

            else {
                double secondsRemaining = std::abs(remainingTime.QuadPart / -10000000.0);
                std::wcout << L"Time remaining: " << std::fixed << std::setprecision(3) << secondsRemaining << L" seconds." << std::endl;
            }
        }
    }


    else {
        std::wcout << L"Usage: pckillerConsole.exe -shutdown <n> | -cancel | -remaining" << std::endl;
    }

    CloseHandle(hDevice);
    return 0;
}