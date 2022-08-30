# InMemmoyLogger

##This project demostrates a WINRT based logging component.

It keeps the data in the memmory which can later be retirved from memory dump. It also have a script that can search and print the logs.

ABI definition
The logger supports a simple ABI with nominal overhead to the consumers.

•	Logger(String name); o	Constructor, takes a name for the logger instance.

•	HRESULT LogCircular(String message); o	Logs into the circular buffer.

•	HRESULT LogPersistent(String message); o	Logs into the persistent buffer.

•	HRESULT ResetPersistentLogs(); o	Clears the persistent buffer.


Debugger extension functions
•	!Logs o	Searches for all the loggers in the memory and presents a DML format where users can click and navigate through logs.
•	!printLogs o	Takes the address of the logs and prints them.
•	!getLoggers o	Searches for the list of loggers and prints them
•	!getInstance o	Gets the details of one instance of the logger.


