# Lightweight in-memory logging

# How to use it.

Dll
The project can be refereneced or the WinRT component can be linked. The Test project demonstrates how to use the dll.

Debugger script.
Run following commands in the windbg
.scriptload <path>\InMemoryLogger.js
!Logs

# Details

Logger should be lightweight and fast.

1. Logger would use 64KB in memory-based events in a round robin manner.
2. The size of each event will be limited to 256 bytes. It will allow 256 events in the memory.
3. Allocated memory will be contagious so that traversal can be index based.
4. Circular logging will support multithread logging but it will be lockless and contention free.

Logger will expose two kinds of memory storage for the logs.

1. Circular log buffer.
  1. This buffer will be overwritten once its full.
2. Persistent log buffer.
  1. This will not be overwritten. Once its full the calls to log more data will fail.


# Log Message

The event will contain two parts

1. Tick count – It will help in log stitching and ordering.
2. Formatted log message.

# Memory dump support.

This logger will register a memory stream with a crash handler to push the data in the memory dumps.

# Debugger extension

Debugger extension will print the logs in the debugger

# Basic design

![](RackMultipart20220830-1-9ilt6a_html_85e5f3e3372dab62.png)

### Persistent storage

64KB buffer that applications can use to store the information that they want to remain available in the crash dumps. For example: Applications like notepad can store the information about application setting and file information that user is trying to open.

### Circular buffer

64KB buffer that applications can use to store the logs. The logs will be overwritten in a circular way such that once the buffer is full the first entry will be overwritten in a circular fashion.

# Logger instances

Logger will support multiple instances. Multiple binaries can choose to either share or instantiate their own logger instance for logging.

For example this logger can be used by a library such as Apps on demand for its own internal logging and the application that using this library can also use another instance of the logger for application specific logging.

All the log instances are changed to each-other. They all will be registered with Windows error reporting to be collected in the memory dumps. The debugger extension will walk the chain of the loggers and present readable and clickable view of the logger.

## In memory view

### Class instance in memory

The following diagram represents the instance view in memory.

1. The log buffer is accessed indirectly by using an array of pointers. It simplifies the calculations to reach the actual buffer.
2. The count is incremented using interlocked operations that keeps the access to the buffer lock free.

![](RackMultipart20220830-1-9ilt6a_html_8764214dd8e1ab4d.gif)

### Log changing in memory

The log instances are chained in memory. It allows multiple binaries to use the logger at the same time and log information based on their requirements.

![](RackMultipart20220830-1-9ilt6a_html_4ca3f3d4ca6e1e5d.gif)

# ABI definition

The logger will support a simple ABI with nominal overhead to the consumers.

- Logger(String name);
  - Constructor, takes a name for the logger instance.
- HRESULT LogCircular(String message);
  - Logs into the circular buffer.
- HRESULT LogPersistent(String message);
  - Logs into the persistent buffer.
- HRESULT ResetPersistentLogs();
  - Clears the persistent buffer.

# Debugger extension

A debugger extension will extract the logs from the memory dumps and print it to the user in readable format.

The extension will search the crashdump memory for the logger's instance and print all the logs.

Debugger extension functions

- !Logs
  - Searches for all the loggers in the memory and presents a DML format where users can click and navigate through logs.
- !printLogs
  - Takes the address of the logs and prints them.
- !getLoggers
  - Searches for the list of loggers and prints them
- !getInstance
  - Gets the details of one instance of the logger.

# Distribution

This WinRT component will be distributed as a nuget package

Nuget package contents

1. The binaries.
2. Debugger script

