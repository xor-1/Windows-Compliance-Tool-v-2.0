#include "NetworkSharesCheck.h"
#include <sstream>
#include <vector>

ComplianceResult NetworkSharesCheck::performCheck() {
    ComplianceResult result("Network Shares Check",
                           "Identifies open network shares and their permissions.",
                           CheckStatus::Pass, Severity::Low, 100);
    
    result.details.push_back("Checking network shares...");
    
    // Get network shares using PowerShell
    std::string psCommand = "Get-SmbShare | Select-Object Name, Path, Description, ShareType | Format-List";
    std::string output = executePowerShell(psCommand);
    
    // Also get share permissions
    std::string permCommand = "Get-SmbShare | ForEach-Object { $share = $_.Name; Get-SmbShareAccess -Name $share | Select-Object @{Name='Share';Expression={$share}}, AccountName, AccessRight, AccessControlType }";
    std::string permOutput = executePowerShell(permCommand);
    
    int totalShares = 0;
    int publicShares = 0;
    int adminOnlyShares = 0;
    std::vector<std::string> shareNames;
    
    // Count shares
    size_t pos = 0;
    while ((pos = output.find("Name", pos)) != std::string::npos) {
        totalShares++;
        
        // Extract share name
        size_t nameStart = output.find(":", pos) + 1;
        size_t nameEnd = output.find("\n", nameStart);
        if (nameEnd != std::string::npos) {
            std::string shareName = output.substr(nameStart, nameEnd - nameStart);
            shareName.erase(0, shareName.find_first_not_of(" \t"));
            shareName.erase(shareName.find_last_not_of(" \t") + 1);
            if (!shareName.empty() && shareName != "Name") {
                shareNames.push_back(shareName);
            }
        }
        
        pos += 4;
    }
    
    // Check for public access (Everyone or Authenticated Users with Full Control)
    if (permOutput.find("Everyone") != std::string::npos || 
        permOutput.find("Authenticated Users") != std::string::npos) {
        // Count how many shares have public access
        size_t permPos = 0;
        while ((permPos = permOutput.find("Everyone", permPos)) != std::string::npos ||
               (permPos = permOutput.find("Authenticated Users", permPos)) != std::string::npos) {
            publicShares++;
            if (permPos != std::string::npos) {
                permPos += 10;
            }
        }
    }
    
    // Check for admin-only shares (default shares like C$, ADMIN$, etc.)
    for (const auto& share : shareNames) {
        if (share.back() == '$') {
            adminOnlyShares++;
        }
    }
    
    std::stringstream details;
    details << "Total Network Shares: " << totalShares << "\n";
    details << "Admin Shares (hidden): " << adminOnlyShares << "\n";
    details << "Shares with Public Access: " << publicShares;
    
    if (!shareNames.empty()) {
        details << "\n\nShare Names:\n";
        for (const auto& name : shareNames) {
            details << "  - " << name << "\n";
        }
    }
    
    result.details.push_back(details.str());
    
    // Calculate score
    int score = 100;
    
    // Penalty for public shares
    if (publicShares > 0) {
        score -= publicShares * 20;
    }
    
    // Small penalty for many custom shares (indicates potential exposure)
    int customShares = totalShares - adminOnlyShares;
    if (customShares > 5) {
        score -= 10;
    }
    
    if (score < 0) score = 0;
    
    result.score = score;
    
    if (score >= 90) {
        result.status = CheckStatus::Pass;
        result.severity = Severity::Low;
        result.recommendation = "Network shares are properly configured. System is compliant.";
    } else if (score >= 70) {
        result.status = CheckStatus::Warning;
        result.severity = Severity::Medium;
        result.recommendation = "Some network shares have public access. Review share permissions and restrict access to authorized users only.";
    } else {
        result.status = CheckStatus::Fail;
        result.severity = Severity::High;
        result.recommendation = "Network shares have excessive public access. This poses a security risk. Immediately restrict share permissions to authorized users only.";
    }
    
    return result;
}

