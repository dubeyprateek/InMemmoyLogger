﻿"use strict";
delete Object.prototype.toString;
delete Array.prototype.toString;

function log(log) {
    host.diagnostics.debugLog(log);
}

function hostModule() {
    return "InMemmoyLogger";
}

function readString(obj) {
    var logString = null;
    var array = [];
    var pointer = host.createPointerObject(obj.address,"ntdll","CHAR*");
    var address = pointer.address;
    for(var i=0; i<256; ++i) 
    {
        logString = host.memory.readString(address);
        array.push(logString);
        address = address+256;
    }
    return array;
}


function printLogs(obj) {
    var logString = null;
    var array = [];
    var pointer = host.createPointerObject(obj, "ntdll", "CHAR*");
    var address = pointer.address;
    for (var i = 0; i < 256; ++i) {
        logString = host.memory.readString(address);
        log(logString);
        address = address + 256;
    }
}

function findSymbol(name, allowUndefined) {
    var moduleName = hostModule();
    var moduleSymbol = host.getModuleSymbol(moduleName, "gLoggerInstanceList");
    if (!allowUndefined && (moduleSymbol == null || moduleSymbol == undefined)) {
        host.diagnostics.debugLog("failed to locate symbol: " + name + " ensure symbols are correctly loaded for " + moduleName);
        return moduleSymbol;
    }
    else {
        return moduleSymbol;
    }
}

class synloggerInstance {
    constructor(instance)
    {
        this.CircularLogs = [];
        this.PersistaneLogs = [];
        var moduleName = hostModule();
        var loggerInstance = instance;
        this.LoggerName = host.memory.readWideString(loggerInstance.loggerInstanceName.address);
        this.CircularLogs = readString(loggerInstance.curcularBufferAddress);
        this.PersistaneLogs = readString(loggerInstance.persistantBufferAddress);
        this.Address = instance.address;
        this.Type = moduleName+"!"+instance.targetType;
    }
    toString()
    {
        return this.LoggerName;
    }
};

function iterateLLST(gLoggerInstanceList) {
    let result = [];
    try {
        let head = gLoggerInstanceList.loggerInstanceListEntry;
        let instance = head.Flink.address;
        for(var i=0; i<gLoggerInstanceList.nodeCount; ++i)
        {
            let loggerInstance = getInstance(instance);
            var syninstance = new synloggerInstance(loggerInstance);
            result.push(syninstance);
            instance = loggerInstance.loggerInstnace.Flink.address;
        }
    }
    catch (e) {
        host.diagnostics.debugLog(e);
    }
    return result;
}

function showLog() {
    var gLoggerInstanceList = findSymbol("gLoggerInstanceList");
    var loggers = iterateLLST(gLoggerInstanceList);
    return loggers;
}

function getLogger() {
    return findSymbol("gLoggerInstanceList");
}

function getInstance(instance)
{
    var moduleName = hostModule();
    return host.createTypedObject(instance, moduleName, "LOGGER_INSTANCE");
}

function initializeScript() {
    return [
        new host.apiVersionSupport(1, 3),
        new host.functionAlias(
            showLog,
            'Logs'
        ),
        new host.functionAlias(
            printLogs,
            'printLogs'
        ),
        new host.functionAlias(
            getLogger,
            'getLoggers'
        ),
        new host.functionAlias(
            getInstance,
            'getInstance'
        )
    ];
}