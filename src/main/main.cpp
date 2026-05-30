#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "rxn_config.h"
#include "../telemetry/rxn_hardware_monitor.h"
#include "../ui/rxn_ui_manager.h"
#include "../graphics/rxn_graphics_engine.h"

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
) {
    // --- 1. System Initialization ---

    // Set a higher-than-normal process priority to ensure the dashboard remains
    // responsive, even when the system is under load. This is crucial for a
    // monitoring application.
    HANDLE hProcess = GetCurrentProcess();
    SetPriorityClass(hProcess, ABOVE_NORMAL_PRIORITY_CLASS);

    // --- 2. Instantiate All Core Components ---

    RXNConfig configManager;
    RXNHardwareMonitor hardwareMonitor;
    RXNUIManager uiManager;
    RXNGraphicsEngine graphicsEngine;

    // --- 3. Component Initialization and Thread Launch ---

    // Initialize the configuration manager first, as other components depend on it.
    configManager.Initialize(L"rxn_settings.json"); // CORRECTED: Pass a wide-string literal.

    // Initialize and start the UI Manager. This will create the main window
    // on a dedicated thread, keeping the UI responsive at all times.
    uiManager.Initialize(hInstance, &configManager);
    uiManager.Start();

    // Poll for the window handle. We must wait until the UI thread has finished
    // creating the main window before we can initialize other components that
    // require a window handle (like the graphics and capture engines).
    HWND hwnd = nullptr;
    while ((hwnd = uiManager.GetWindowHandle()) == nullptr) {
        Sleep(50); // Yield CPU while waiting.
    }

    // Now that the window exists, initialize the remaining engines.
    hardwareMonitor.Initialize(); // Does not require a window handle.
    graphicsEngine.Initialize(hwnd, &configManager);

    // Start the background processing threads.
    hardwareMonitor.Start();
    graphicsEngine.Start();

    // --- 4. Main Application Wait Loop ---

    // The main thread will now enter a low-power wait state. Its only job is
    // to wait until the user closes the main window. IsWindow() provides a
    // reliable and efficient way to detect this.
    while (IsWindow(hwnd)) {
        Sleep(250); // Check every quarter second.
    }

    // --- 5. Graceful Shutdown Sequence ---

    // Once the window is closed, the application begins its shutdown procedure.
    // We stop the engines in the reverse order of their dependency.
    // The graphics engine depends on the UI window, so it is stopped first.
    graphicsEngine.Stop();

    // The hardware monitor has no dependencies and can be stopped at any time.
    hardwareMonitor.Stop();

    // Finally, we signal the UI manager to clean up its thread.
    uiManager.Stop();

    // Restore the process priority to normal before exiting.
    SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);

    return 0;
}
