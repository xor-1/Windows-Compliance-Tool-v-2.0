#include "UserAccountCheck.h"
#include <sstream>
#include <vector>

ComplianceResult UserAccountCheck::performCheck() {
    ComplianceResult result("User Account Review",
                           "Lists local accounts and identifies unnecessary or admin-level users.",
                           CheckStatus::Pass, Severity::Low, 100);
    
    result.details.push_back("Reviewing local user accounts...");
    
    // Get local user accounts using PowerShell
    std::string psCommand = "Get-LocalUser | Select-Object Name, Enabled, Description, PrincipalSource | Format-List";
    std::string output = executePowerShell(psCommand);
    
    // Also get admin accounts
    std::string adminCommand = "Get-LocalGroupMember -Group 'Administrators' | Select-Object Name, PrincipalSource | Format-List";
    std::string adminOutput = executePowerShell(adminCommand);
    
    int totalUsers = 0;
    int enabledUsers = 0;
    int adminUsers = 0;
    int disabledUsers = 0;
    std::vector<std::string> adminAccountNames;
    
    // Count users from output
    size_t pos = 0;
    while ((pos = output.find("Name", pos)) != std::string::npos) {
        totalUsers++;
        pos += 4;
        
        // Check if enabled
        size_t enabledPos = output.find("Enabled", pos);
        if (enabledPos != std::string::npos && enabledPos < pos + 200) {
            if (output.find("True", enabledPos) != std::string::npos) {
                enabledUsers++;
            } else {
                disabledUsers++;
            }
        }
    }
    
    // Count admin users
    pos = 0;
    while ((pos = adminOutput.find("Name", pos)) != std::string::npos) {
        adminUsers++;
        size_t nameStart = adminOutput.find(":", pos) + 1;
        size_t nameEnd = adminOutput.find("\n", nameStart);
        if (nameEnd != std::string::npos) {
            std::string adminName = adminOutput.substr(nameStart, nameEnd - nameStart);
            // Remove whitespace
            adminName.erase(0, adminName.find_first_not_of(" \t"));
            adminName.erase(adminName.find_last_not_of(" \t") + 1);
            if (!adminName.empty() && adminName.find("Administrator") == std::string::npos) {
                adminAccountNames.push_back(adminName);
            }
        }
        pos += 4;
    }
    
    std::stringstream details;
    details << "Total Local Users: " << totalUsers << "\n";
    details << "Enabled Users: " << enabledUsers << "\n";
    details << "Disabled Users: " << disabledUsers << "\n";
    details << "Administrator Accounts: " << adminUsers;
    
    if (!adminAccountNames.empty()) {
        details << "\n\nAdmin Accounts (excluding built-in Administrator):\n";
        for (const auto& name : adminAccountNames) {
            details << "  - " << name << "\n";
        }
    }
    
    result.details.push_back(details.str());
    
    // Calculate score
    int score = 100;
    int issues = 0;
    
    // Penalty for too many admin accounts
    if (adminUsers > 2) {
        score -= (adminUsers - 2) * 10;
        issues++;
    }
    
    // Penalty for disabled accounts (potential security risk if not needed)
    if (disabledUsers > 0) {
        score -= 10;
    }
    
    // Penalty for too many total users (indicates poor account management)
    if (totalUsers > 10) {
        score -= 10;
        issues++;
    }
    
    if (score < 0) score = 0;
    
    result.score = score;
    
    if (score >= 90) {
        result.status = CheckStatus::Pass;
        result.severity = Severity::Low;
        result.recommendation = "User account configuration is appropriate. System is compliant.";
    } else if (score >= 70) {
        result.status = CheckStatus::Warning;
        result.severity = Severity::Medium;
        result.recommendation = "Review user accounts. Consider removing unnecessary accounts and limiting administrator access.";
    } else {
        result.status = CheckStatus::Fail;
        result.severity = Severity::High;
        result.recommendation = "User account configuration needs attention. Reduce the number of administrator accounts and remove unused accounts.";
    }
    
    return result;
}

