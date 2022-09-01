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
        Logger() = default                                              ;
        Logger(hstring const& name)                                     ;

        winrt::hresult LogCircular(hstring const& message)              ;
        winrt::hresult LogPersistent(hstring const& message)            ;
        winrt::hresult ResetPersistentLogs()                            ;

    private:
        HRESULT RegisterAddressesWithWER()                              ;
        BOOL IsInstanceIntialized()                                     ;
        VOID FormatLogMessage(CHAR* outBuffer, DWORD bufferSize,
            hstring const& message)                                     ;
        VOID PrintMessagesInTheDebugger(const CHAR* message)            ;
        HRESULT LogInternal(LogType logType, hstring const& message)    ;
        VOID Write(LONG Index, hstring const& message, LogType logType) ;
        VOID InitializeLogMemory(hstring const& name)                   ;

    private:
        volatile LONG       countCircularBuffer                         ;
        volatile LONG       countPersistentBuffer                       ;
        volatile LONG       indexPersistentBuffer                       ;
        PVOID               persistentLogBuffer                         ;
        PVOID               circularLogBuffer                           ;
        int                 maxAllocationSize                           ;
        BOOL                isInitialized                               ;
        PVOID               circularLogIndex[MAXBUFFER_COUNT]           ;
        PVOID               persistentLogIndex[MAXBUFFER_COUNT]         ;
        CRITICAL_SECTION    csProtectInstance                           ;
    };
}
namespace winrt::InMemmoyLogger::factory_implementation
{
    struct Logger : LoggerT<Logger, implementation::Logger>
    {
    };
}
