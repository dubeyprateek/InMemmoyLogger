﻿"use strict";

function initializeScript()
{
    //
    // Return an array of registration objects to modify the object model of the debugger
    // See the following for more details:
    //
    //     https://aka.ms/JsDbgExt
    //
    return [new host.apiVersionSupport(1, 7)];
}


let logln = function (e) {
    host.diagnostics.debugLog(e + '\n');
}

function read_u64(addr) {
    return host.memory.readMemoryValues(addr, 1, 8)[0];
}

function invokeScript() {
    //let Regs = host.currentThread.Registers.User;
    //let a = read_u64(Regs.rsp);
    //logln(a.toString(16));
    //let WideStr = host.currentProcess.Environment.EnvironmentBlock.ProcessParameters.ImagePathName.Buffer;
    //logln(host.memory.readWideString(WideStr));
    //let WideStrAddress = WideStr.address;
    //logln(host.memory.readWideString(WideStrAddress));
     GetLogger();
}

function GetLogger()
{
    loggerList = host.getModuleSymbol("InMemmoyLogger","gLoggerInstanceList");
    logln(loggerList);
}
