#ifndef PROCESSREADER_H
#define PROCESSREADER_H

#include <windows.h>
#include <tchar.h>
#include <vector>

class ProcessReader
{
public:
    ProcessReader();
    std::vector<int> update();
private:
    void getBaseAddress();
    std::vector<int> getHand(int size);
    DWORD_PTR baseAddress;
    DWORD pid;
    HANDLE windowHandle;
};

#endif // PROCESSREADER_H
