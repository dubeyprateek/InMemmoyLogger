function invokeScript()
{
    
}

function initializeScript()
{

}

function GetLoggersList()
{
    host.diagnostics.debugLog("Function:: DoSomeThing \n");
    let x = host.getModuleSymbol("InMemmoyLogger", "gLoggerInstanceList");
    if (x == null)
    {
        throw new Error("gLoggerInstanceList not found\n");
    }
}