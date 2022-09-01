#include "pch.h"
#include "Class.h"
#include "Class.g.cpp"
#include "pch.h"
#include <Windows.h>
#include<Memoryapi.h>
#include <sstream>
#include"winrt/InMemmoyLogger.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace std;

#define LOOP_LIMIT 100

namespace winrt::TestDll::implementation
{
    int32_t Class::MyProperty()
    {
        throw hresult_not_implemented();
    }
    void Class::MyProperty(int32_t value)
    {
        UNREFERENCED_PARAMETER(value);
        throw hresult_not_implemented();
    }
    void Class::TestLogger()
    {
        std::wostringstream wostringstream;
        wostringstream << L"TestDLL-ThreadID [" << GetCurrentThreadId() << L"]" << endl;
        InMemmoyLogger::Logger logger(wostringstream.str());

        for (ULONGLONG i = 0; i < LOOP_LIMIT; ++i)
        {
            std::wostringstream wostringstreamloop;
            wostringstreamloop << L"TestDLLThreadID [" << GetCurrentThreadId() << L"] Circular Loop count [" << i << L"]" << endl;
            logger.LogCircular(wostringstreamloop.str());
            logger.LogPersistent(wostringstreamloop.str());
        }
    }
}
