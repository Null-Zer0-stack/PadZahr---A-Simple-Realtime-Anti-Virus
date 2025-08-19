#include <vector>
#include <string>
#include <utility>
#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <cctype>

#include <AppCore/App.h>
#include <AppCore/Window.h>
#include <AppCore/Overlay.h>
#include <AppCore/JSHelpers.h>
#include <Ultralight/View.h>
#include <Ultralight/String.h>
#include <Ultralight/Listener.h>

#include "BlackList.h"
#include "KillProcess.h"
#include "ProcessList.h"
#include "ConvertToString.h"

// === Global State for Monitoring Thread === //
std::atomic<bool> g_monitoringActive(false);
std::thread g_monitorThread;






// Main Ultralight application class
class MyApp : public ultralight::AppListener,
    public ultralight::WindowListener,
    public ultralight::LoadListener {
private:
    ultralight::RefPtr<ultralight::App> app_;
    ultralight::RefPtr<ultralight::Window> window_;
    ultralight::RefPtr<ultralight::Overlay> overlay_;
    ultralight::RefPtr<ultralight::View> view_;
public:
    ultralight::RefPtr<ultralight::View> view() {
        return view_;
    }

    MyApp() {
        app_ = ultralight::App::Create();
        window_ = ultralight::Window::Create(app_->main_monitor(), 900, 768, false,
            ultralight::kWindowFlags_Titled | ultralight::kWindowFlags_Maximizable |ultralight::kWindowFlags_Resizable);
        window_->SetTitle("PadZahr");
        window_->set_listener(this);
        overlay_ = ultralight::Overlay::Create(window_, window_->width(), window_->height(), 0, 0);
        view_ = overlay_->view();
        view_->set_load_listener(this);
        view_->LoadURL("file:///front_page.html");
    }

    ~MyApp() {
        // Stop the monitoring thread in the destructor
        g_monitoringActive = false;
        if (g_monitorThread.joinable()) {
            g_monitorThread.join();
        }
    }

    void BindJSCallbacks(ultralight::View* caller) {
        ultralight::RefPtr<ultralight::JSContext> context = caller->LockJSContext();
        ultralight::SetJSContext(context->ctx());
        ultralight::JSObject global_obj = ultralight::JSGlobalObject();
        global_obj["killProcess"] = ultralight::JSCallbackWithRetval(std::bind(&MyApp::OnKillProcess, this, std::placeholders::_1, std::placeholders::_2));
        global_obj["startMonitoring"] = ultralight::JSCallbackWithRetval(std::bind(&MyApp::OnStartMonitoring, this, std::placeholders::_1, std::placeholders::_2));
        global_obj["stopMonitoring"] = ultralight::JSCallbackWithRetval(std::bind(&MyApp::OnStopMonitoring, this, std::placeholders::_1, std::placeholders::_2));
    }

    ultralight::JSValue OnKillProcess(const ultralight::JSObject& this_object, const ultralight::JSArgs& args) {
        if (args.size() == 1 && args[0].IsNumber()) {
            DWORD pid = (DWORD)args[0].ToNumber();

            // Check to prevent self-termination 
            if (pid == GetCurrentProcessId()) {
                std::string result_str = "Warning: Attempted to kill self (PID: " + std::to_string(pid) + "). Operation aborted.";
                return ultralight::JSValue(ultralight::String(result_str.c_str()));
            }

            if (killProcess(pid)) {
                std::string result_str = "Process with PID " + std::to_string(pid) + " killed successfully.";
                return ultralight::JSValue(ultralight::String(result_str.c_str()));
            }
            else {
                std::string result_str = "Failed to kill process with PID " + std::to_string(pid) + ". Check permissions.";
                return ultralight::JSValue(ultralight::String(result_str.c_str()));
            }
        }
        return ultralight::JSValue(ultralight::String("Invalid arguments."));
    }

    ultralight::JSValue OnStartMonitoring(const ultralight::JSObject& this_object, const ultralight::JSArgs& args) {
        if (!g_monitoringActive) {
            g_monitoringActive = true;
            g_monitorThread = std::thread(&MyApp::monitorProcessesInternal, this);
            return ultralight::JSValue(ultralight::String("Monitoring started."));
        }
        return ultralight::JSValue(ultralight::String("Monitoring is already active."));
    }

    ultralight::JSValue OnStopMonitoring(const ultralight::JSObject& this_object, const ultralight::JSArgs& args) {
        if (g_monitoringActive) {
            g_monitoringActive = false;
            if (g_monitorThread.joinable()) {
                g_monitorThread.join();
            }
            return ultralight::JSValue(ultralight::String("Monitoring stopped."));
        }
        return ultralight::JSValue(ultralight::String("Monitoring is not active."));
    }


    virtual void OnDOMReady(ultralight::View* caller,
        uint64_t frame_id,
        bool is_main_frame,
        const ultralight::String& url) override {
        if (!is_main_frame) return;

        BindJSCallbacks(caller);
        auto processes = getProcessList();
        for (const auto& proc : processes) {
            std::string procName = WStringToString(proc.first);
            size_t pos = 0;
            while ((pos = procName.find('\'', pos)) != std::string::npos) {
                procName.replace(pos, 1, "\\'");
                pos += 2;
            }
            std::string script = "addProcess('" + procName + "', " + std::to_string(proc.second) + ");";
            caller->EvaluateScript(ultralight::String(script.c_str()));
        }
        caller->EvaluateScript(ultralight::String("addLog('Initial process list loaded. Click 'Start Monitoring' to activate real-time detection.');"));
    }

    virtual void OnClose(ultralight::Window* window) override {
        // Stop the monitoring thread in the OnClose handler as well
        g_monitoringActive = false;
        if (g_monitorThread.joinable()) {
            g_monitorThread.join();
        }
        app_->Quit();
    }

    void Run() {
        app_->Run();
    }

private:
    // New member function to run the monitoring logic
    void monitorProcessesInternal() {
        while (g_monitoringActive) {
            std::vector<std::pair<std::wstring, DWORD>> currentProcesses = getProcessList();

            for (const auto& proc : currentProcesses) {
                std::string procNameStr = WStringToString(proc.first);
                std::transform(procNameStr.begin(), procNameStr.end(), procNameStr.begin(), ::tolower);

                bool isBlacklisted = false;
                for (const auto& blacklisted : g_blacklistedProcesses) {
                    if (procNameStr.find(blacklisted) != std::string::npos) {
                        isBlacklisted = true;
                        break;
                    }
                }

                if (isBlacklisted) {
                    // Check to prevent self-termination in the monitoring thread 
                    if (proc.second == GetCurrentProcessId()) {
                        std::string log_message = "addLog('Warning: Attempted to kill self (PID: " + std::to_string(proc.second) + "). Skipped.');";
                        this->view()->EvaluateScript(ultralight::String(log_message.c_str()));
                        continue;
                    }

                    if (killProcess(proc.second)) {
                        size_t pos = 0;
                        while ((pos = procNameStr.find('\'', pos)) != std::string::npos) {
                            procNameStr.replace(pos, 1, "\\'");
                            pos += 2;
                        }
                        std::string log_message = "addLog('Detected and killed malicious process: " + procNameStr + " (PID: " + std::to_string(proc.second) + ").');";
                        this->view()->EvaluateScript(ultralight::String(log_message.c_str()));
                    }
                    else {
                        size_t pos = 0;
                        while ((pos = procNameStr.find('\'', pos)) != std::string::npos) {
                            procNameStr.replace(pos, 1, "\\'");
                            pos += 2;
                        }
                        std::string log_message = "addLog('Failed to kill malicious process: " + procNameStr + " (PID: " + std::to_string(proc.second) + "). Check permissions.');";
                        this->view()->EvaluateScript(ultralight::String(log_message.c_str()));
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
};

int main() {
    MyApp my_app;
    my_app.Run();
    return 0;
}