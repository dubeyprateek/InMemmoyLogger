#pragma once
#include "Logger.g.h"
#include <concurrent_unordered_map.h>
#include "DebugSupport.h"
#include "CommonMacros.h"

using namespace concurrency;
using namespace std;



namespace winrt::InMemmoyLogger::implementation
{
    enum class LogType
    {
        LOGTYPE_CIRCULAR,
        LOGTYPE_PERSITENT
    };

    struct Logger : LoggerT<Logger>
    {
        Logger() = default;
        Logger(hstring const& name);

        winrt::hresult LogCircular(hstring const& message);
        winrt::hresult LogPersistent(hstring const& message);
        winrt::hresult ResetPersistentLogs();

    private:
        HRESULT RegisterAddressesWithWER();
        BOOL IsInstanceIntialized();
        VOID FormatLogMessage(WCHAR outBuffer[BUFFER_SIZE], DWORD bufferSize,
            const WCHAR* messageBuffer, LONG messageindex);
        VOID PrintMessagesInTheDebugger(const WCHAR* message);
        HRESULT LogInternal(LogType logType, hstring const& message);
        VOID Write(LONG Index, const WCHAR*  messageBuffer, LogType logType);
        VOID InitializeLogMemory(hstring const& name);

    private:
        volatile LONG       countCircularBuffer                         ;
        volatile LONG       countPersistentBuffer                       ;
        volatile LONG       indexPersistentBuffer                       ;
        PVOID               persistentLogBuffer                         ;
        PVOID               circularLogBuffer                           ;
        int                 maxAllocationSize                           ;
        BOOL                isInitialized                               ;
        PVOID               circularLogIndex[NUM_BUFFER-1]              ;
        PVOID               persistentLogIndex[NUM_BUFFER-1]            ;
        CRITICAL_SECTION    csProtectInstance                           ;
    };
}
namespace winrt::InMemmoyLogger::factory_implementation
{
    struct Logger : LoggerT<Logger, implementation::Logger>
    {
    };
}
