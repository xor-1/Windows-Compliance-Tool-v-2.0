#include "WindowsUpdateCheck.h"
#include <sstream>

ComplianceResult WindowsUpdateCheck::performCheck() {
    ComplianceResult result("Windows Update Status",
                           "Checks if automatic updates and patch management are enabled.",
                           CheckStatus::Pass, Severity::Low, 100);
    
    result.details.push_back("Checking Windows Update configuration...");
    
    bool autoUpdateEnabled = false;
    int updateLevel = 0; // 0 = disabled, 1 = notify, 2 = download, 3 = install, 4 = auto install
    
    // Check Windows Update settings via registry
    std::string auOptions = readRegistryValue(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update",
        "AUOptions");
    
    if (!auOptions.empty()) {
        updateLevel = std::stoi(auOptions);
        autoUpdateEnabled = (updateLevel >= 2);
    }
    
    // Check if Windows Update service is running
    std::string serviceStatus = executePowerShell("Get-Service -Name wuauserv | Select-Object -ExpandProperty Status");
    bool serviceRunning = (serviceStatus.find("Running") != std::string::npos);
    
    // Check for pending updates
    std::string pendingUpdates = executePowerShell("(New-Object -ComObject Microsoft.Update.Session).CreateUpdateSearcher().Search('IsInstalled=0').Updates.Count");
    int pendingCount = 0;
    if (!pendingUpdates.empty() && pendingUpdates.find_first_of("0123456789") != std::string::npos) {
        try {
            pendingCount = std::stoi(pendingUpdates);
        } catch (...) {
            pendingCount = 0;
        }
    }
    
    std::stringstream details;
    details << "Automatic Updates: " << (autoUpdateEnabled ? "Enabled" : "Disabled") << "\n";
    details << "Update Level: " << updateLevel << " (0=Disabled, 2=Download, 3=Install, 4=Auto Install)\n";
    details << "Windows Update Service: " << (serviceRunning ? "Running" : "Stopped") << "\n";
    details << "Pending Updates: " << pendingCount;
    
    result.details.push_back(details.str());
    
    // Calculate score
    int score = 0;
    
    if (autoUpdateEnabled && serviceRunning) {
        score = 100;
        if (pendingCount > 0) {
            score -= (pendingCount > 10 ? 20 : pendingCount * 2); // Penalty for pending updates
            if (score < 70) score = 70; // Minimum score if auto-update is enabled
        }
    } else if (serviceRunning) {
        score = 50; // Service running but auto-update not fully configured
    } else {
        score = 0; // Service not running
    }
    
    result.score = score;
    
    if (score >= 90) {
        result.status = CheckStatus::Pass;
        result.severity = Severity::Low;
        result.recommendation = "Windows Update is properly configured. Consider installing pending updates if any.";
    } else if (score >= 70) {
        result.status = CheckStatus::Warning;
        result.severity = Severity::Medium;
        result.recommendation = "Windows Update is enabled but has pending updates. Install updates to maintain security.";
    } else if (score >= 50) {
        result.status = CheckStatus::Warning;
        result.severity = Severity::Medium;
        result.recommendation = "Windows Update service is running but automatic updates are not fully configured. Enable automatic installation of updates.";
    } else {
        result.status = CheckStatus::Fail;
        result.severity = Severity::High;
        result.recommendation = "Windows Update is disabled or not functioning. Enable automatic updates immediately to ensure system security.";
    }
    
    return result;
}

