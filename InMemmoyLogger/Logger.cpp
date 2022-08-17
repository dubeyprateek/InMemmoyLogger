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
            for (int i = 0; i < MAXBUFFER_COUNT; ++i)
            {
                circularLogIndex[i] = (PBYTE)circularLogBuffer + i * BUFFER_SIZE;
                persistentLogIndex[i] = (PBYTE)persistentLogBuffer + i * BUFFER_SIZE;
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
        countCircularBuffer(LONG_MAX),
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
        _snwprintf_s(outBuffer, bufferSize, _TRUNCATE, L"[%llu] %s", GetTickCount64(), message);
    }

    VOID Logger::PrintMessagesInTheDebugger(const WCHAR* message)
    {
        if (IsDebuggerPresent())
        {
            OutputDebugStringW(message);
            YieldProcessor();
        }
    }
    HRESULT Logger::LogInternal(LogType logType, hstring const& message)
    {
        HRESULT result = S_OK;

        switch (logType)
        {
        case winrt::InMemmoyLogger::implementation::LogType::LOGTYPE_CIRCULAR:
        {
            volatile LONG currentIndex = InterlockedIncrement(&countCircularBuffer);
            if (currentIndex < 0)
            {
                // 2147483647L = LONG_MAX
                // -2147483648L = LONG_MAX+1
                currentIndex = currentIndex + LONG_MAX + 1 ;
                auto distance = LONG_MAX % (MAXBUFFER_COUNT);
                currentIndex = (currentIndex + distance) % (MAXBUFFER_COUNT);
            }
            else {
                currentIndex = currentIndex % (MAXBUFFER_COUNT);
            }
            Write(currentIndex, message.c_str(), logType);
        }
        break;
        case winrt::InMemmoyLogger::implementation::LogType::LOGTYPE_PERSITENT:
        {
            EnterCriticalSection(&csProtectInstance);
            volatile LONG maxIndex = (maxAllocationSize / BUFFER_SIZE) -1;
            volatile LONG currentIndex = InterlockedIncrement(&countPersistentBuffer);
            currentIndex %= MAXBUFFER_COUNT;
            if (indexPersistentBuffer < maxIndex)
            {
                Write(currentIndex, message.c_str(), logType);
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

    void Logger::Write(LONG index, const WCHAR*  messageBuffer, LogType logType)
    {
        WCHAR  message[BUFFER_SIZE] = L"";
        PVOID currentMemoryLocation = NULL;
        FormatLogMessage(message, BUFFER_SIZE, messageBuffer, index);
        
        switch (logType)
        {
        case winrt::InMemmoyLogger::implementation::LogType::LOGTYPE_CIRCULAR:
            currentMemoryLocation  = circularLogIndex[index];
            break;
        case winrt::InMemmoyLogger::implementation::LogType::LOGTYPE_PERSITENT:
            currentMemoryLocation = persistentLogIndex[index];
            break;
        default:
            break;
        }

        if (currentMemoryLocation) 
        {
            ZeroMemory(currentMemoryLocation, BUFFER_SIZE);
            wcsncpy_s((TCHAR*)currentMemoryLocation,
                BUFFER_SIZE,
                message,
                BUFFER_SIZE);
        }
    }
}
