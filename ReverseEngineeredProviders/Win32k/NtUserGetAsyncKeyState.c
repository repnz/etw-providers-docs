/*

    An anomaly is defined as: 

    A process called GetAsyncKeyState() more than 1 time per millisecond between 2 times some key is pressed.
    Every time GetAsyncKeyState() is called 
*/

NT_STATUS EtwInitializeAsyncKeyMonitor();

void EtwTraceGetAsyncKeyState();


struct CAsyncKeyEventMonitor {

    //
    // GetTickCount() is called each time GetAsyncKeyState is called
    // The result is saved here.
    //
    ULONG LastKeyStateRequestTickCount;

    //
    // This is the cached index of the last process in the TrackedProcesses array.
    //
    ULONG LastProcessIndex;

    struct CAsyncKeyEventMonitorProcess TrackedProcesses[10];

    //
    // If this flag is set when a key is pressed,
    // The tracked processes are checked to see if one of them called GetAsyncKeyState more than 1 per MS
    //
    BOOLEAN KeyStateRequested;

    EX_PUSH_LOCK MonitorLock;

    void OnKeyStateRequested(ULONG ProcessId);
};



struct CAsyncKeyEventMonitorProcess {
    
    //
    // The ID of the process that is tracked.
    //
    ULONG ProcessId;

    //
    // The number of times the process called GetAsyncKeyState() until a key was pressed
    //
    ULONG BackgroundCallCount;
}


CAsyncKeyEventMonitor* gpAsyncKeyEventMonitor;


//
// This is called from NtUserGetAsyncKeyState
//
void EtwTraceGetAsyncKeyState()
{
    NT_STATUS Status;

    if (W32kEtwEnabledKeyword & AUDIT_API_CALLS  && McGenLevelKeywordEnabled(&EtwTraceGetAsyncKeyState)) {
    
        if (!gpAsyncKeyEventMonitor) { 

            Status = EtwInitializeAsyncKeyMonitor();

            if (!NT_SUCCESS(Status)) { 
                return;
            }
        }

        OnKeyStateRequested(eventMonitor->ProcessInfo->ProcessId);
    }
}


// 
// Called from NtuserGetAsyncKeyState
//
void CAsyncKeyEventMonitor::OnKeyStateRequested(ULONG ProcessId)
{
    ULONG LastProcessIndex; 
    ULONG CurrentIndex; 

    ExAcquirePushLockExclusiveEx(&gpAsyncKeyEventMonitor->MonitorLock, FALSE);

    LastProcessIndex = gpAsyncKeyEventMonitor->LastProcessIndex;

    gpAsyncKeyEventMonitor->KeyStateRequested = TRUE;

    if ( gpAsyncKeyEventMonitor->TrackedProcesses[LastProcessIndex].ProcessId == ProcessId )// Cache of the last index
    {
        gpAsyncKeyEventMonitor->TrackedProcesses[LastProcessIndex].BackgroundCallCount++;
        goto Clean;
    }
    
    CurrentIndex = 0;

    if ( !gpAsyncKeyEventMonitor->LastKeyStateRequestTickCount ) { 
        gpAsyncKeyEventMonitor->LastKeyStateRequestTickCount = GetTickCount();
    }

    CurrentIndex = 0;

    while (TRUE) { 
        CAsyncKeyEventMonitorProcess* CurrentProcess = gpAsyncKeyEventMonitor->TrackedProcesses[CurrentIndex];


        if (CurrentProcess->ProcessId == ProcessId) {
            //
            // Found! Increase key count
            //
            CurrentProcess.BackgroundCallCount++;
            break;
        }

        if (CurrentProcess->ProcessId == -1){
            //
            // Reached an empty process!
            // Set this process to be the current process
            //
            CurrentProcess.ProcessId = ProcessId;
            CurrentProcess.BackgroundCallCount = 1;
            break;
        }


        CurrentIndex++;

        if (CurrentIndex >= 10) { 
            goto Clean;
        }
    }

    gpAsyncKeyEventMonitor->LastProcessIndex = CurrentIndex;


Clean:
    ExReleasePushLockExclusiveEx(&localAsyncKeyEventMonitor->MonitorLock, FALSE);

}

//
// This is called each time a key is pressed / released.
//
void OnKeyStateRequested::OnKeyEvent()
{
    ExAcquirePushLockExclusiveEx(&gpAsyncKeyEventMonitor->MonitorLock, FALSE);

    //
    // Check that NtUserGetAsyncKeyState was called.
    //
    if ( gpAsyncKeyEventMonitor->LastKeyStateRequestTickCount && gpAsyncKeyEventMonitor->KeyStateRequested) { 
        //
        //  Go over the process list and find anomaly..
        //
        ReportGetAsyncKeyStateAnomaly(gpAsyncKeyEventMonitor, GetTickCount() - gpAsyncKeyEventMonitor->LastKeyStateRequestTickCount);
    
    }
    
    gpAsyncKeyEventMonitor->KeyStateRequested = FALSE;
    gpAsyncKeyEventMonitor->LastKeyStateRequestTickCount = GetTickCount();
    
    ExReleasePushLockExclusiveEx(&gpAsyncKeyEventMonitor->MonitorLock, FALSE);
}


void CAsyncKeyEventMonitor::ReportGetAsyncKeyStateAnomaly(CAsyncKeyEventMonitor* this, unsigned int MsSinceLastKeyEvent) {

    //
    // This function enumerates over the tracked processes and tries to find an anomaly.
    //

    for ( ULONG CurrentIndex = 0; CurrentIndex < 10; ++CurrentIndex )  {
        CAsyncKeyEventMonitorProcess* TrackerProcess = this->TrackerProcesses[CurrentIndex];

        if (TrackerProcess->ProcessId == -1 ) // Process not found.
          break;

        ULONG BackgroundCallCount = TrackerProcess->BackgroundCallCount;

        //
        // This function should be called at least 2 times..
        //

        if (BackgroundCallCount >= 2) {            
            
            //
            // Check that it's more than 1 call in a millisecond
            //

            if (MsSinceLastKeyEvent) { 
                CallsInMillisecond = BackgroundCallCount / MsSinceLastKeyEvent;

            } else {
                CallsInMillisecond = BackgroundCallCount;
            }

            if (CallsInMillisecond >= 1) {
                EventWrite_GET_ASYNC_KEY_STATE(
                    TrackerProcess->ProcessId, // PID
                    MsSinceLastKeyEvent,  // MsSinceLastKeyEvent
                    BackgroundCallCount   // 
                );
            }
        }

        TrackerProcess->ProcessId = -1;        // Reset
        TrackerProcess->BackgroundCallCount = -1;
    }
}


//
// Called if
//
NT_STATUS EtwInitializeAsyncKeyMonitor()
{
    CAsyncKeyEventMonitor* EventMonitor; 

    EventMonitor = (CAsyncKeyEventMonitor*)Win32AllocPoolZInit(sizeof(CAsyncKeyEventMonitor));
    
    if (!EventMonitor) {
        return STATUS_NO_MEMORY;
    }

    EventMonitor->LastKeyStateRequestTickCount = 0;
    EventMonitor->LastProcessIndex = 0;
    ExInitializePushLock(&EventMonitor->MonitorLock);
    EventMonitor->KeyStateRequested = FALSE;

    //
    // Initialize to -1 to denote it's empty
    //
    Rtl(EventMonitor->TrackedProcesses, 0xFFFFFFFF, sizeof(EventMonitor->TrackedProcesses));
  
    if ( _InterlockedCompareExchange64((volatile signed __int64 *)&gpAsyncKeyEventMonitor, (signed __int64)EventMonitor, 0)) {
        Win32FreePool(EventMonitor);
    }
  
    return STATUS_SUCCESS;
}