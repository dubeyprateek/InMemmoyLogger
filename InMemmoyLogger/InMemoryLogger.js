"use strict";

function log(log) {
    host.diagnostics.debugLog(log);
    host.diagnostics.debugLog("\n");
}

function hostModule() {
    return "InMemmoyLogger";
}

function readString(obj) {
    var logString = null;
    var array = [];
    log("readString::");
    var pointer = host.createPointerObject(obj,"ntdll","WCHAR*");
    var address = pointer.address;
    for(var i=0; i<256; ++i) 
    {
        logString = host.memory.readWideString(address);
        log(logString);
        array.push(logString);
        address = address+256;
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

function iterateLLST(gLoggerInstanceList, targetType) {
    var result = [];
    try {

        var moduleName = hostModule();
        let head = gLoggerInstanceList.loggerInstanceListEntry;
        let instance = head.Flink;
        for(var i=0; i<gLoggerInstanceList.nodeCount; ++i)
        {
            let loggerInstance = host.createTypedObject(instance.address, moduleName, "LOGGER_INSTANCE");
            result.push(loggerInstance);
            instance = loggerInstance.loggerInstnace.Flink;
        }
    }
    catch (e) {
        host.diagnostics.debugLog(e);
    }
    return result;
}

function showLog() {
    var gLoggerInstanceList = findSymbol("gLoggerInstanceList");
    var loggers = iterateLLST(gLoggerInstanceList, "_LOGGER_INSTANCE *");
    return loggers;
}

function getLogger() {
    return findSymbol("gLoggerInstanceList");
}

function initializeScript() {
    return [
        new host.apiVersionSupport(1, 3),
        new host.functionAlias(
            showLog,
            'showLog'
        ),
        new host.functionAlias(
            readString,
            'readLog'
        ),
        new host.functionAlias(
            getLogger,
            'getLogger'
        )
    ];
}