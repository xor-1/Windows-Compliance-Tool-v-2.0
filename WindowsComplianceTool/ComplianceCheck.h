#pragma once
#include "ComplianceResult.h"
#include <string>

// Base class for all compliance checks
class ComplianceCheck {
public:
    virtual ~ComplianceCheck() = default;
    
    // Perform the compliance check
    virtual ComplianceResult performCheck() = 0;
    
    // Get the name of this check module
    virtual std::string getModuleName() const = 0;
    
    // Check if this module is enabled
    bool isEnabled() const { return enabled; }
    void setEnabled(bool value) { enabled = value; }

protected:
    bool enabled = true;
    
    // Helper method to execute PowerShell commands
    std::string executePowerShell(const std::string& command);
    
    // Helper method to read registry values
    std::string readRegistryValue(const std::string& keyPath, const std::string& valueName);
    
    // Helper method to check if a registry value exists
    bool registryValueExists(const std::string& keyPath, const std::string& valueName);
};

