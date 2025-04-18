
void Inject() {
    std::vector<std::uint8_t> Data = server.GetDllExport();

    PROCESSENTRY32W PE32{ 0 }; // Используем PROCESSENTRY32W для поддержки Unicode
    PE32.dwSize = sizeof(PE32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create snapshot!" << std::endl;
        return;
    }

    DWORD PID = 0;
    BOOL bRet = Process32FirstW(hSnap, &PE32); // Используем Process32FirstW для Unicode
    while (bRet) {
        // Сравниваем строки с помощью wcscmp
        if (wcscmp(L"RustClient.exe", PE32.szExeFile) == 0) {
            PID = PE32.th32ProcessID;
            break;
        }
        bRet = Process32NextW(hSnap, &PE32); // Используем Process32NextW для Unicode
    }

    CloseHandle(hSnap);

    if (PID == 0) {
        std::cerr << "Process 'RustClient.exe' not found!" << std::endl;
        return;
    }

    std::cout << "ProcessId - " << PID << std::endl;

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
    if (!hProc) {
        std::cerr << "Failed to open target process!" << std::endl;
        return;
    }

    std::cout << "hProcess - " << hProc << std::endl;

    if (!ManualMap(hProc, Data.data())) {
        std::cerr << "Failed to manually map DLL!" << std::endl;
    }

    std::cout << "Data Size - " << Data.size() << std::endl;

    CloseHandle(hProc);

    std::cout << "Handle closed" << std::endl;


    std::cout << "injected" << std::endl;

}