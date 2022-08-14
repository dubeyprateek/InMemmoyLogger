#include "pch.h"
#include "Logger.h"
#include "Logger.g.cpp"
#include <WerApi.h>
#include "CommonMacros.h"
#include "List.h"

extern LOGGER_LIST gLoggerInstanceList;
namespace winrt::InMemmoyLogger::implementation
{
    winrt::hresult Logger::LogCircular(hstring const& message)
    {
        HRESULT result = S_OK;
        if (IsInstanceIntialized()) 
        {
            result = LogInternal(LogType::LOGTYPE_CIRCULAR, message);
        }
        else 
        {
            result = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        }
        return result;
    }

    winrt::hresult Logger::LogPersistent(hstring const& message)
    {
        HRESULT result = S_OK;
        if (IsInstanceIntialized())
        {
            result = LogInternal(LogType::LOGTYPE_PERSITENT, message);
        }
        else
        {
            result = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        }
        return result;
    }

    HRESULT Logger::RegisterAddressesWithWER()
    {
        // We don't care about the failure of these functions. 
        // If they are succesful the memory dump will contain information to search the logs.
        WerRegisterMemoryBlock(circularLogBuffer, maxAllocationSize);
        WerRegisterMemoryBlock(persistentLogBuffer, maxAllocationSize);
        WerRegisterMemoryBlock(&gLoggerInstanceList, sizeof(LOGGER_LIST));
        return S_OK;
    }

    VOID Logger::InitializeLogMemory(hstring const& name)
    {
        DWORD allocationFlags = MEM_COMMIT;
        HRESULT result = S_OK;
        if (!IsInstanceIntialized()) 
        {
            LOGGER_INSTANCE inMemoryLoggerInstance = {};
            persistentLogBuffer = NULL;
            circularLogBuffer = NULL;
            maxAllocationSize = MIN_ALLOCATIION_SIZE;
            
            wcsncpy_s(inMemoryLoggerInstance.loggerInstanceName, name.c_str(), LOGINSTANCENAMELENGTH);
            inMemoryLoggerInstance.persistantBufferAddress = 
                persistentLogBuffer = VirtualAllocFromApp(NULL, maxAllocationSize, allocationFlags, PAGE_READWRITE);
            if (!persistentLogBuffer)
            {
                result = HRESULT_FROM_WIN32(GetLastError());
                goto CleanUp;
            }
            
            inMemoryLoggerInstance.curcularBufferAddress = 
                circularLogBuffer = VirtualAllocFromApp(NULL, maxAllocationSize, allocationFlags, PAGE_READWRITE);
            if (!circularLogBuffer)
            {
                result = HRESULT_FROM_WIN32(GetLastError());
                goto CleanUp;
            }
            gLoggerInstanceList.AddItemInTheList(&inMemoryLoggerInstance);
            for (int i = 1; i <= NUM_BUFFER; ++i)
            {
                circularLogIndex[ i -1 ] = (PBYTE)circularLogBuffer + i * BUFFER_SIZE;
                persistentLogIndex[i - 1] = (PBYTE)persistentLogBuffer + i * BUFFER_SIZE;
            }
            RegisterAddressesWithWER();
        }

    CleanUp:
        if (SUCCEEDED(result)) 
        {
            isInitialized = TRUE;
        }
        else 
        {
            if (persistentLogBuffer)
            {
                VirtualFree(persistentLogBuffer, 0, MEM_RELEASE);
                persistentLogBuffer = NULL;
            }
            if (circularLogBuffer)
            {
                VirtualFree(circularLogBuffer, 0, MEM_RELEASE);
                circularLogBuffer = NULL;
            }
        }
        return;
    }

    winrt::hresult Logger::ResetPersistentLogs()
    {
        if (IsInstanceIntialized()) 
        {
            EnterCriticalSection(&csProtectInstance);
            countPersistentBuffer = -1;
            indexPersistentBuffer = 0;
            ZeroMemory(persistentLogBuffer, maxAllocationSize);
            LeaveCriticalSection(&csProtectInstance);
        }
        return S_OK;
    }

    Logger::Logger(hstring const& name) : isInitialized(FALSE),
        persistentLogBuffer(NULL),
        circularLogBuffer(NULL),
        maxAllocationSize(0),
        countCircularBuffer(MAXLONG),
        countPersistentBuffer(-1),
        indexPersistentBuffer(0),
        circularLogIndex{NULL},
        persistentLogIndex{NULL}
    {
        InitializeCriticalSection(&csProtectInstance);
        InitializeLogMemory(name);
    }

    BOOL Logger::IsInstanceIntialized()
    {
        return isInitialized;
    }

    VOID Logger::FormatLogMessage(WCHAR outBuffer[BUFFER_SIZE], DWORD bufferSize, const WCHAR* message, LONG messageindex)
    {
        _snwprintf_s(outBuffer, bufferSize, _TRUNCATE, L"[%d] %s", messageindex, message);
    }

    VOID Logger::PrintMessagesInTheDebugger(const WCHAR* message)
    {
        if (IsDebuggerPresent())
        {
            OutputDebugStringW(message);
            YieldProcessor();
            OutputDebugStringW(L"\n");
        }
    }
    HRESULT Logger::LogInternal(LogType logType, hstring const& message)
    {
        HRESULT result = S_OK;

        switch (logType)
        {
        case winrt::InMemmoyLogger::implementation::LogType::LOGTYPE_CIRCULAR:
        {
            //volatile LONG currentIndex = InterlockedIncrement(&countCircularBuffer);
            volatile LONG currentIndex = countCircularBuffer++;
            if (currentIndex < 0)
            {
                //2147483647L
                //-2147483648L
                currentIndex = currentIndex + LONG_MAX + 2 ;
                auto distance = LONG_MAX % (NUM_BUFFER - 1);
                currentIndex = (currentIndex + distance) % (NUM_BUFFER - 1);
            }
            else {
                currentIndex = currentIndex % (NUM_BUFFER - 1);
            }

            PVOID currentMemoryLocation = circularLogIndex[currentIndex];
            ZeroMemory(currentMemoryLocation, BUFFER_SIZE-1);
            YieldProcessor();
            Write(currentIndex, currentMemoryLocation, message.c_str());
        }
        break;
        case winrt::InMemmoyLogger::implementation::LogType::LOGTYPE_PERSITENT:
        {
            EnterCriticalSection(&csProtectInstance);
            volatile LONG maxIndex = maxAllocationSize / BUFFER_SIZE;
            volatile LONG currentIndex = InterlockedIncrement(&countPersistentBuffer);
            currentIndex %= NUM_BUFFER - 1;
            if (indexPersistentBuffer < maxIndex)
            {
                PVOID currentMemoryLocation = persistentLogIndex[currentIndex];
                ZeroMemory(currentMemoryLocation, BUFFER_SIZE-1);
                Write(currentIndex, currentMemoryLocation, message.c_str());
                indexPersistentBuffer++;
            }
            else
            {
                result = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            }
            LeaveCriticalSection(&csProtectInstance);
        }
        break;
        default:
            result = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            break;
        }

        return result;
    }

    void Logger::Write(LONG index, const PVOID& currentMemoryLocation, const WCHAR*  messageBuffer)
    {
        WCHAR  message[BUFFER_SIZE] = L"";
        FormatLogMessage(message, BUFFER_SIZE, messageBuffer, index);
        PrintMessagesInTheDebugger(message);
        wcsncpy_s((TCHAR*)currentMemoryLocation,
            BUFFER_SIZE - 1,
            message,
            BUFFER_SIZE - 1);
    }
}
