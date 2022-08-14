#pragma once
#include <Windows.h>
#include <WerApi.h>
#include "List.h"
#define LOGINSTANCENAMELENGTH 64
typedef struct _LOGGER_INSTANCE
{
	LIST_ENTRY loggerInstnace;
	WCHAR loggerInstanceName[LOGINSTANCENAMELENGTH];
	VOID* curcularBufferAddress;
	VOID* persistantBufferAddress;
	_LOGGER_INSTANCE():curcularBufferAddress(NULL), 
		persistantBufferAddress(NULL), 
		loggerInstanceName()
	{
		InitializeListHead(&loggerInstnace);
	}
	_LOGGER_INSTANCE(const _LOGGER_INSTANCE &instance)
	{
		InitializeListHead(&loggerInstnace);
		wcsncpy_s(loggerInstanceName, instance.loggerInstanceName, LOGINSTANCENAMELENGTH);
		curcularBufferAddress = instance.curcularBufferAddress;
		persistantBufferAddress = instance.persistantBufferAddress;
	}
	void operator = (const _LOGGER_INSTANCE &instance)
	{
		InitializeListHead(&loggerInstnace);
		wcsncpy_s(loggerInstanceName, instance.loggerInstanceName, LOGINSTANCENAMELENGTH);
		curcularBufferAddress = instance.curcularBufferAddress;
		persistantBufferAddress = instance.persistantBufferAddress;
	}

}LOGGER_INSTANCE, *PLOGGER_INSTANCE;


typedef struct _LOGGER_LIST {
	LIST_ENTRY loggerInstanceListEntry;
	CRITICAL_SECTION csLoggerListLock;
	int nodeCount;
	_LOGGER_LIST()
	{
		InitializeListHead(&loggerInstanceListEntry);
		InitializeCriticalSection(&csLoggerListLock);
		nodeCount = 0;
	}
	void AddItemInTheList(const PLOGGER_INSTANCE pItem)
	{
		PLOGGER_INSTANCE newItem = new LOGGER_INSTANCE(*pItem);
		EnterCriticalSection(&csLoggerListLock);
		InsertHeadList(&loggerInstanceListEntry, &newItem->loggerInstnace);
		nodeCount++;
		WerRegisterMemoryBlock(&newItem->loggerInstnace, sizeof(PLOGGER_INSTANCE));
		LeaveCriticalSection(& csLoggerListLock);
	}
} LOGGER_LIST, *PLOGGER_LIST;

LOGGER_LIST gLoggerInstanceList;