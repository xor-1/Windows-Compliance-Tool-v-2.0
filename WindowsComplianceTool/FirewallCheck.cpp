#include "FirewallCheck.h"
#include <sstream>

ComplianceResult FirewallCheck::performCheck() {
    ComplianceResult result("Firewall Status", 
                           "Verifies whether the Windows Firewall is enabled for Domain, Private, and Public profiles.",
                           CheckStatus::Pass, Severity::Low, 100);
    
    result.details.push_back("Checking Windows Firewall status...");
    
    // Check firewall status using PowerShell
    std::string psCommand = "Get-NetFirewallProfile | Select-Object Name, Enabled | Format-List";
    std::string output = executePowerShell(psCommand);
    
    bool domainEnabled = false;
    bool privateEnabled = false;
    bool publicEnabled = false;
    
    // Parse the output
    if (output.find("Domain") != std::string::npos) {
        size_t pos = output.find("Domain");
        if (output.find("True", pos) != std::string::npos) {
            domainEnabled = true;
        }
    }
    
    if (output.find("Private") != std::string::npos) {
        size_t pos = output.find("Private");
        if (output.find("True", pos) != std::string::npos) {
            privateEnabled = true;
        }
    }
    
    if (output.find("Public") != std::string::npos) {
        size_t pos = output.find("Public");
        if (output.find("True", pos) != std::string::npos) {
            publicEnabled = true;
        }
    }
    
    std::stringstream details;
    details << "Domain Profile: " << (domainEnabled ? "Enabled" : "Disabled") << "\n";
    details << "Private Profile: " << (privateEnabled ? "Enabled" : "Disabled") << "\n";
    details << "Public Profile: " << (publicEnabled ? "Enabled" : "Disabled");
    
    result.details.push_back(details.str());
    
    // Calculate score and status
    int enabledCount = (domainEnabled ? 1 : 0) + (privateEnabled ? 1 : 0) + (publicEnabled ? 1 : 0);
    
    if (enabledCount == 3) {
        result.status = CheckStatus::Pass;
        result.score = 100;
        result.severity = Severity::Low;
        result.recommendation = "All firewall profiles are enabled. System is compliant.";
    } else if (enabledCount == 2) {
        result.status = CheckStatus::Warning;
        result.score = 67;
        result.severity = Severity::Medium;
        result.recommendation = "One or more firewall profiles are disabled. Enable all profiles for maximum security.";
    } else if (enabledCount == 1) {
        result.status = CheckStatus::Fail;
        result.score = 33;
        result.severity = Severity::High;
        result.recommendation = "Multiple firewall profiles are disabled. This poses a significant security risk. Enable all profiles immediately.";
    } else {
        result.status = CheckStatus::Fail;
        result.score = 0;
        result.severity = Severity::Critical;
        result.recommendation = "Windows Firewall is completely disabled. This is a critical security risk. Enable the firewall immediately.";
    }
    
    return result;
}

