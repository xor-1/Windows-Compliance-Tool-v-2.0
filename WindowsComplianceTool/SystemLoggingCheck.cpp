#include "SystemLoggingCheck.h"
#include <sstream>

ComplianceResult SystemLoggingCheck::performCheck() {
    ComplianceResult result("System Logging Verification",
                           "Checks if Windows Event Logging is active for security and audit tracking.",
                           CheckStatus::Pass, Severity::Low, 100);
    
    result.details.push_back("Checking Windows Event Logging configuration...");
    
    // Check if Event Log service is running
    std::string serviceStatus = executePowerShell("Get-Service -Name EventLog | Select-Object -ExpandProperty Status");
    bool serviceRunning = (serviceStatus.find("Running") != std::string::npos);
    
    // Check audit policy settings
    std::string auditPolicy = executePowerShell("auditpol /get /category:*");
    
    bool logonAuditing = false;
    bool objectAccessAuditing = false;
    bool policyChangeAuditing = false;
    bool accountManagementAuditing = false;
    
    // Parse audit policy output
    if (auditPolicy.find("Logon") != std::string::npos) {
        if (auditPolicy.find("Success and Failure", auditPolicy.find("Logon")) != std::string::npos ||
            auditPolicy.find("Success", auditPolicy.find("Logon")) != std::string::npos) {
            logonAuditing = true;
        }
    }
    
    if (auditPolicy.find("Object Access") != std::string::npos) {
        if (auditPolicy.find("Success and Failure", auditPolicy.find("Object Access")) != std::string::npos ||
            auditPolicy.find("Success", auditPolicy.find("Object Access")) != std::string::npos) {
            objectAccessAuditing = true;
        }
    }
    
    if (auditPolicy.find("Policy Change") != std::string::npos) {
        if (auditPolicy.find("Success and Failure", auditPolicy.find("Policy Change")) != std::string::npos ||
            auditPolicy.find("Success", auditPolicy.find("Policy Change")) != std::string::npos) {
            policyChangeAuditing = true;
        }
    }
    
    if (auditPolicy.find("Account Management") != std::string::npos) {
        if (auditPolicy.find("Success and Failure", auditPolicy.find("Account Management")) != std::string::npos ||
            auditPolicy.find("Success", auditPolicy.find("Account Management")) != std::string::npos) {
            accountManagementAuditing = true;
        }
    }
    
    // Check event log sizes
    std::string securityLogSize = executePowerShell("(Get-EventLog -LogName Security -ErrorAction SilentlyContinue).MaximumKilobytes");
    std::string applicationLogSize = executePowerShell("(Get-EventLog -LogName Application -ErrorAction SilentlyContinue).MaximumKilobytes");
    std::string systemLogSize = executePowerShell("(Get-EventLog -LogName System -ErrorAction SilentlyContinue).MaximumKilobytes");
    
    std::stringstream details;
    details << "Event Log Service: " << (serviceRunning ? "Running" : "Stopped") << "\n";
    details << "Logon Auditing: " << (logonAuditing ? "Enabled" : "Disabled") << "\n";
    details << "Object Access Auditing: " << (objectAccessAuditing ? "Enabled" : "Disabled") << "\n";
    details << "Policy Change Auditing: " << (policyChangeAuditing ? "Enabled" : "Disabled") << "\n";
    details << "Account Management Auditing: " << (accountManagementAuditing ? "Enabled" : "Disabled");
    
    if (!securityLogSize.empty() && securityLogSize.find_first_of("0123456789") != std::string::npos) {
        details << "\nSecurity Log Size: " << securityLogSize << " KB";
    }
    
    result.details.push_back(details.str());
    
    // Calculate score
    int score = 0;
    
    if (!serviceRunning) {
        score = 0;
    } else {
        score = 20; // Base score for service running
        
        if (logonAuditing) score += 20;
        if (objectAccessAuditing) score += 20;
        if (policyChangeAuditing) score += 20;
        if (accountManagementAuditing) score += 20;
    }
    
    result.score = score;
    
    if (score >= 90) {
        result.status = CheckStatus::Pass;
        result.severity = Severity::Low;
        result.recommendation = "Event logging is properly configured. System is compliant.";
    } else if (score >= 60) {
        result.status = CheckStatus::Warning;
        result.severity = Severity::Medium;
        result.recommendation = "Event logging is partially configured. Enable all recommended audit policies for comprehensive security monitoring.";
    } else if (score >= 20) {
        result.status = CheckStatus::Warning;
        result.severity = Severity::Medium;
        result.recommendation = "Event logging service is running but audit policies need improvement. Enable logon, object access, policy change, and account management auditing.";
    } else {
        result.status = CheckStatus::Fail;
        result.severity = Severity::High;
        result.recommendation = "Event logging is not properly configured. Enable the Event Log service and configure audit policies for security monitoring.";
    }
    
    return result;
}

