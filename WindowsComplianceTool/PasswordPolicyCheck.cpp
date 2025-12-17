#include "PasswordPolicyCheck.h"
#include <sstream>

ComplianceResult PasswordPolicyCheck::performCheck() {
    ComplianceResult result("Password Policy Review",
                           "Evaluates password length, complexity, and expiration settings.",
                           CheckStatus::Pass, Severity::Low, 100);
    
    result.details.push_back("Checking password policy settings...");
    
    // Check password policy using registry
    int minLength = 0;
    bool complexityRequired = false;
    int maxAge = 0;
    int minAge = 0;
    int historySize = 0;
    
    // Minimum password length
    std::string minLengthStr = readRegistryValue(
        "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters",
        "MinimumPasswordLength");
    if (!minLengthStr.empty()) {
        minLength = std::stoi(minLengthStr);
    } else {
        // Try alternative location
        minLengthStr = readRegistryValue(
            "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
            "MinimumPasswordLength");
        if (!minLengthStr.empty()) {
            minLength = std::stoi(minLengthStr);
        }
    }
    
    // Password complexity
    std::string complexityStr = readRegistryValue(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
        "PasswordComplexity");
    complexityRequired = (complexityStr == "1");
    
    // Maximum password age
    std::string maxAgeStr = readRegistryValue(
        "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters",
        "MaximumPasswordAge");
    if (!maxAgeStr.empty()) {
        maxAge = std::stoi(maxAgeStr);
    }
    
    // Minimum password age
    std::string minAgeStr = readRegistryValue(
        "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters",
        "MinimumPasswordAge");
    if (!minAgeStr.empty()) {
        minAge = std::stoi(minAgeStr);
    }
    
    // Password history
    std::string historyStr = readRegistryValue(
        "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters",
        "PasswordHistoryLength");
    if (!historyStr.empty()) {
        historySize = std::stoi(historyStr);
    }
    
    // Use PowerShell as fallback for more accurate results
    std::string psOutput = executePowerShell("Get-ADDefaultDomainPasswordPolicy -ErrorAction SilentlyContinue | Select-Object MinPasswordLength, PasswordHistoryCount, MaxPasswordAge, MinPasswordAge, ComplexityEnabled | Format-List");
    
    // If PowerShell worked, try to parse it
    if (psOutput.find("MinPasswordLength") != std::string::npos) {
        // Parse PowerShell output if available
        // For now, we'll use registry values
    }
    
    // If registry values are not available, use PowerShell net accounts command
    if (minLength == 0) {
        std::string netAccounts = executePowerShell("net accounts");
        // Parse net accounts output
        if (netAccounts.find("Minimum password length") != std::string::npos) {
            // Extract minimum length from output
            size_t pos = netAccounts.find("Minimum password length");
            if (pos != std::string::npos) {
                size_t start = netAccounts.find(":", pos) + 1;
                size_t end = netAccounts.find("\n", start);
                if (end == std::string::npos) end = netAccounts.length();
                std::string lenStr = netAccounts.substr(start, end - start);
                // Remove whitespace
                lenStr.erase(0, lenStr.find_first_not_of(" \t"));
                lenStr.erase(lenStr.find_last_not_of(" \t") + 1);
                if (!lenStr.empty()) {
                    minLength = std::stoi(lenStr);
                }
            }
        }
    }
    
    std::stringstream details;
    details << "Minimum Password Length: " << (minLength > 0 ? std::to_string(minLength) : "Not configured") << "\n";
    details << "Complexity Required: " << (complexityRequired ? "Yes" : "No") << "\n";
    details << "Maximum Password Age: " << (maxAge > 0 ? std::to_string(maxAge) + " days" : "Not configured") << "\n";
    details << "Minimum Password Age: " << (minAge > 0 ? std::to_string(minAge) + " days" : "Not configured") << "\n";
    details << "Password History: " << (historySize > 0 ? std::to_string(historySize) + " passwords" : "Not configured");
    
    result.details.push_back(details.str());
    
    // Calculate score based on NIST/ISO recommendations
    // Recommended: min length >= 8, complexity enabled, max age <= 90 days, history >= 12
    int score = 0;
    int issues = 0;
    
    if (minLength >= 8) score += 25;
    else if (minLength >= 6) score += 15;
    else issues++;
    
    if (complexityRequired) score += 25;
    else issues++;
    
    if (maxAge > 0 && maxAge <= 90) score += 25;
    else if (maxAge > 90) issues++;
    else score += 15; // Not configured is better than too long
    
    if (historySize >= 12) score += 25;
    else if (historySize > 0) score += 15;
    else issues++;
    
    result.score = score;
    
    if (score >= 90) {
        result.status = CheckStatus::Pass;
        result.severity = Severity::Low;
        result.recommendation = "Password policy meets security standards. System is compliant.";
    } else if (score >= 70) {
        result.status = CheckStatus::Warning;
        result.severity = Severity::Medium;
        result.recommendation = "Password policy needs improvement. Consider increasing minimum length to 8+ characters, enabling complexity, and setting appropriate age limits.";
    } else {
        result.status = CheckStatus::Fail;
        result.severity = (issues >= 3) ? Severity::High : Severity::Medium;
        result.recommendation = "Password policy does not meet security standards. Configure minimum length (8+), enable complexity requirements, and set password expiration (max 90 days).";
    }
    
    return result;
}

