#include "pch.h"
#include <Windows.h>
#include<Memoryapi.h>
#include <sstream>
#include"winrt/InMemmoyLogger.h"
#include"winrt/TestDll.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace std;


DWORD WINAPI MyThreadFunction(LPVOID lpParam);
DWORD WINAPI MyThreadFunction2(LPVOID lpParam);

#define NUM_THREADS 50
#define LOOP_LIMIT MAXULONGLONG
int main()
{
    init_apartment();
    Uri uri(L"http://aka.ms/cppwinrt");
    printf("Hello, %ls!\n", uri.AbsoluteUri().c_str());

    TestDll::Class objClass;
    InMemmoyLogger::Logger logger(L"FirstInstance");
    HANDLE threadHandles[NUM_THREADS];
    DWORD dwThreadIdArray[NUM_THREADS];

    objClass.TestLogger();

    for (int i = 0; i < NUM_THREADS/2; ++i)
    {
        threadHandles[i] = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            MyThreadFunction,       // thread function name
            &logger,                // argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray[i]);   // returns the thread identifier 
    }

    for (int i = NUM_THREADS/2; i < NUM_THREADS; ++i)
    {
        threadHandles[i] = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            MyThreadFunction2,      // thread function name
            NULL,                   // argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray[i]);   // returns the thread identifier 
    }

    WaitForMultipleObjects(NUM_THREADS, threadHandles, true, INFINITE);
    printf("Logging Complete\n");
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam)
{
    InMemmoyLogger::Logger* logger = (InMemmoyLogger::Logger*)lpParam;
    for (ULONGLONG i = 0; i < LOOP_LIMIT; ++i)
    {
        std::wostringstream wostringstream;
        wostringstream << L"ThreadID [" << GetCurrentThreadId() << L"] Circular Loop count [" << i << L"]" << endl;
        logger->LogCircular(wostringstream.str());

        wostringstream << L"ThreadID [" << GetCurrentThreadId() << L"] Persistent Loop count [" << i << L"]" << endl;
        logger->LogPersistent(wostringstream.str());
    }
    return 0;
}

DWORD WINAPI MyThreadFunction2(LPVOID lpParam)
{
    UNREFERENCED_PARAMETER(lpParam);
    std::wostringstream wostringstream;
    wostringstream << L"ThreadID [" << GetCurrentThreadId() << L"]" << endl;
    InMemmoyLogger::Logger logger(wostringstream.str());
    
    for (ULONGLONG i = 0; i < LOOP_LIMIT; ++i)
    {
        std::wostringstream wostringstreamloop ;
        wostringstreamloop << L"ThreadID [" << GetCurrentThreadId() << L"] Circular Loop count [" << i << L"]" << endl;
        logger.LogCircular(wostringstreamloop.str());
        logger.LogPersistent(wostringstreamloop.str());
    }
    return 0;
}