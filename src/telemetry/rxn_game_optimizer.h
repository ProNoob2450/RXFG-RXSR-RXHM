#pragma once

#include <Windows.h>

class RXNGameOptimizer {
public:
    RXNGameOptimizer();

    // Sets the target application window. The optimizer will find the process
    // associated with this window.
    bool SetTargetByWindow(HWND hwnd);

    // Elevates the target process to HIGH_PRIORITY_CLASS.
    bool ApplyHighPriority();

    // Reverts the target process priority to its original state.
    bool RestoreOriginalPriority();

private:
    // --- Member Variables ---
    HANDLE m_targetProcessHandle = NULL;
    DWORD m_originalPriorityClass = 0;
    DWORD m_targetProcessId = 0;
};
