#include "processreader.h"

#include <tlhelp32.h>


DWORD_PTR ADDRESSES[5] = {0x1F40AC,0x240,0x340,0x3C,0x1A8};
DWORD_PTR HANDSIZE[7] = {0x0,0xAC,0x14,0x20,0x2c,0xc,0xc};
DWORD_PTR DECKLIST[7] = {0x0,0xAC,0x14,0x20,0x2c,0xc,0x8};
DWORD_PTR FIRSTCARD = 0x10;
DWORD_PTR CARDNAME[3] = {0x30,0x8,0x30};

DWORD_PTR FindPointer(int offset, HANDLE pHandle,DWORD_PTR baseaddr, DWORD_PTR offsets[]);
DWORD_PTR dwGetModuleBaseAddress(DWORD dwProcID, const char *szModuleName);

ProcessReader::ProcessReader()
{
    baseAddress = 0;
}

std::vector<int> ProcessReader::update()
{
    std::vector<int> ret;

    // do stuff here
    if (baseAddress)
    {
        unsigned int handSize = FindPointer(7,windowHandle, baseAddress, HANDSIZE);

        //if hand size is valid
        if (handSize < 10)
        {
            ret = getHand(handSize);
        }
    }
    else
        getBaseAddress();

    return ret;
}

std::vector<int> ProcessReader::getHand(int size)
{
    std::vector<int> cardsInHand;

    DWORD deckAddr = FindPointer(7,windowHandle, baseAddress, DECKLIST);

    for (int i = 0; i < size; i++)
    {
        DWORD_PTR card[1] = {FIRSTCARD + 0x4 * i};
        DWORD currentCard = FindPointer(1,windowHandle,deckAddr, card);
        cardsInHand.push_back((int)FindPointer(3,windowHandle, currentCard,CARDNAME ));
    }

    return cardsInHand;
}

void ProcessReader::getBaseAddress()
{
    HWND window = FindWindowA(NULL, "Shadowverse");
    GetWindowThreadProcessId(window, &pid);

    windowHandle = OpenProcess(PROCESS_VM_READ, false, pid);

    std::string windowName = std::string("mono.dll") ;
    DWORD_PTR mono = dwGetModuleBaseAddress(pid, windowName.c_str());
    baseAddress = FindPointer(5,windowHandle, mono, ADDRESSES);
}

DWORD_PTR dwGetModuleBaseAddress(DWORD dwProcID, const char *szModuleName)
{
    DWORD_PTR dwModuleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcID);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 ModuleEntry32;
        ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hSnapshot, &ModuleEntry32))
        {
            do
            {
                std::wstring wide(ModuleEntry32.szModule);
                std::string cstr(wide.begin(),wide.end());
                if (_tcsicmp(cstr.c_str(), szModuleName) == 0)
                {
                    dwModuleBaseAddress = (DWORD_PTR)ModuleEntry32.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnapshot, &ModuleEntry32));
        }
        CloseHandle(hSnapshot);
    }
    return dwModuleBaseAddress;
}

DWORD_PTR FindPointer(int offset, HANDLE pHandle, DWORD_PTR baseaddr, DWORD_PTR offsets[])
{
   DWORD_PTR Address = baseaddr;
   int total = offset;
   for (int i = 0; i < total; i++) //Loop trough the offsets
   {
      Address+=offsets[i];
      if(!ReadProcessMemory(pHandle, (LPCVOID)Address, &Address , 4, NULL))
      {
          Address = 0;
          break;
      }
   }
   return Address;
}
