#include "rxn_game_optimizer.h"

RXNGameOptimizer::RXNGameOptimizer() = default;

bool RXNGameOptimizer::SetTargetByWindow(HWND hwnd) {
    if (!IsWindow(hwnd)) {
        return false;
    }

    // Get the Process ID from the window handle
    GetWindowThreadProcessId(hwnd, &m_targetProcessId);
    if (m_targetProcessId == 0) {
        return false;
    }

    // Open a handle to the process with the necessary rights
    m_targetProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION, FALSE, m_targetProcessId);
    if (m_targetProcessHandle == NULL) {
        return false;
    }

    // Store the original priority so we can revert it later
    m_originalPriorityClass = GetPriorityClass(m_targetProcessHandle);
    if (m_originalPriorityClass == 0) {
        // Failed to get priority, clean up and return
        CloseHandle(m_targetProcessHandle);
        m_targetProcessHandle = NULL;
        return false;
    }

    return true;
}

bool RXNGameOptimizer::ApplyHighPriority() {
    if (m_targetProcessHandle == NULL) {
        return false;
    }

    // Set the process priority to HIGH
    if (!SetPriorityClass(m_targetProcessHandle, HIGH_PRIORITY_CLASS)) {
        return false;
    }

    // Future enhancement: This is where you could also iterate through the process's
    // main threads and use SetThreadPriority and SetThreadAffinityMask for more
    // granular control, effectively creating a "performance core" for the game.
    // For now, process-level priority is a robust first step.

    return true;
}

bool RXNGameOptimizer::RestoreOriginalPriority() {
    if (m_targetProcessHandle == NULL || m_originalPriorityClass == 0) {
        return false;
    }

    // Revert the process priority to its original value
    bool success = SetPriorityClass(m_targetProcessHandle, m_originalPriorityClass);

    // Clean up the handle
    CloseHandle(m_targetProcessHandle);
    m_targetProcessHandle = NULL;
    m_originalPriorityClass = 0;
    m_targetProcessId = 0;

    return success;
}
