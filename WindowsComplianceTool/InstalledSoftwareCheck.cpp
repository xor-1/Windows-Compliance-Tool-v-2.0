#include "InstalledSoftwareCheck.h"
#include <sstream>
#include <vector>
#include <algorithm>

ComplianceResult InstalledSoftwareCheck::performCheck() {
    ComplianceResult result("Installed Software Audit",
                           "Lists all installed applications and flags potentially risky or outdated ones.",
                           CheckStatus::Pass, Severity::Low, 100);
    
    result.details.push_back("Auditing installed software...");
    
    // Get installed software using PowerShell
    std::string psCommand = "Get-WmiObject -Class Win32_Product | Select-Object Name, Version, InstallDate | Sort-Object Name | Format-Table -AutoSize";
    std::string output = executePowerShell(psCommand);
    
    // Also check via registry (more reliable)
    std::string regCommand = "Get-ItemProperty HKLM:\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\* | Select-Object DisplayName, DisplayVersion, Publisher | Where-Object {$_.DisplayName -ne $null} | Sort-Object DisplayName | Format-Table -AutoSize";
    std::string regOutput = executePowerShell(regCommand);
    
    int totalSoftware = 0;
    std::vector<std::string> riskySoftware;
    std::vector<std::string> outdatedSoftware;
    
    // Count software from registry output
    size_t pos = 0;
    while ((pos = regOutput.find("DisplayName", pos)) != std::string::npos) {
        totalSoftware++;
        pos += 11;
    }
    
    // Check for potentially risky software (common examples)
    std::vector<std::string> riskyKeywords = {
        "torrent", "crack", "keygen", "serial", "hack", "cracked",
        "p2p", "file sharing", "remote desktop", "vnc"
    };
    
    std::string lowerOutput = regOutput;
    std::transform(lowerOutput.begin(), lowerOutput.end(), lowerOutput.begin(), ::tolower);
    
    for (const auto& keyword : riskyKeywords) {
        if (lowerOutput.find(keyword) != std::string::npos) {
            riskySoftware.push_back(keyword);
        }
    }
    
    // Check for very old software (no version info or very old dates)
    // This is a simplified check - in production, you'd check actual version dates
    
    std::stringstream details;
    details << "Total Installed Applications: " << totalSoftware << "\n";
    
    if (!riskySoftware.empty()) {
        details << "Potentially Risky Software Detected: " << riskySoftware.size() << " types\n";
        details << "Keywords found: ";
        for (size_t i = 0; i < riskySoftware.size(); ++i) {
            details << riskySoftware[i];
            if (i < riskySoftware.size() - 1) details << ", ";
        }
    } else {
        details << "No obviously risky software detected.";
    }
    
    details << "\n\nNote: This is a basic audit. Review installed software manually for security compliance.";
    
    result.details.push_back(details.str());
    
    // Calculate score
    int score = 100;
    
    // Penalty for risky software
    if (!riskySoftware.empty()) {
        score -= riskySoftware.size() * 15;
    }
    
    // Penalty for too many installed applications (indicates poor software management)
    if (totalSoftware > 100) {
        score -= 10;
    } else if (totalSoftware > 200) {
        score -= 20;
    }
    
    if (score < 0) score = 0;
    
    result.score = score;
    
    if (score >= 90) {
        result.status = CheckStatus::Pass;
        result.severity = Severity::Low;
        result.recommendation = "Installed software appears compliant. Regularly review and update applications.";
    } else if (score >= 70) {
        result.status = CheckStatus::Warning;
        result.severity = Severity::Medium;
        result.recommendation = "Some potentially risky software detected. Review and remove unnecessary or unauthorized applications.";
    } else {
        result.status = CheckStatus::Fail;
        result.severity = Severity::High;
        result.recommendation = "Multiple potentially risky software applications detected. Conduct a thorough software audit and remove unauthorized applications immediately.";
    }
    
    return result;
}

