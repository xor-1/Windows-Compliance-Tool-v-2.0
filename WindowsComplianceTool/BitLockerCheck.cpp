#include "BitLockerCheck.h"
#include <sstream>

ComplianceResult BitLockerCheck::performCheck() {
    ComplianceResult result("Disk Encryption (BitLocker)",
                           "Verifies encryption status for system drives.",
                           CheckStatus::Pass, Severity::Low, 100);
    
    result.details.push_back("Checking BitLocker encryption status...");
    
    // Check BitLocker status using PowerShell
    std::string psCommand = "Get-BitLockerVolume | Select-Object MountPoint, VolumeType, EncryptionPercentage, VolumeStatus, ProtectionStatus | Format-List";
    std::string output = executePowerShell(psCommand);
    
    bool bitLockerAvailable = false;
    bool systemDriveEncrypted = false;
    bool allDrivesEncrypted = true;
    int encryptedDrives = 0;
    int totalDrives = 0;
    
    // Check if BitLocker is available (Windows Pro/Enterprise/Education)
    if (output.find("MountPoint") != std::string::npos || output.find("Get-BitLockerVolume") == std::string::npos) {
        bitLockerAvailable = true;
        
        // Parse output to check encryption status
        size_t pos = 0;
        while ((pos = output.find("MountPoint", pos)) != std::string::npos) {
            totalDrives++;
            
            // Find the mount point value
            size_t mountStart = output.find(":", pos) + 1;
            size_t mountEnd = output.find("\n", mountStart);
            std::string mountPoint = output.substr(mountStart, mountEnd - mountStart);
            mountPoint.erase(0, mountPoint.find_first_not_of(" \t"));
            mountPoint.erase(mountPoint.find_last_not_of(" \t") + 1);
            
            // Check if this is the system drive (usually C:)
            bool isSystemDrive = (mountPoint == "C:" || mountPoint.find("System") != std::string::npos);
            
            // Check encryption percentage
            size_t encPos = output.find("EncryptionPercentage", pos);
            if (encPos != std::string::npos && encPos < pos + 500) {
                size_t encStart = output.find(":", encPos) + 1;
                size_t encEnd = output.find("\n", encStart);
                std::string encStr = output.substr(encStart, encEnd - encStart);
                encStr.erase(0, encStr.find_first_not_of(" \t"));
                encStr.erase(encStr.find_last_not_of(" \t") + 1);
                
                if (!encStr.empty()) {
                    try {
                        int encPercent = std::stoi(encStr);
                        if (encPercent == 100) {
                            encryptedDrives++;
                            if (isSystemDrive) {
                                systemDriveEncrypted = true;
                            }
                        } else {
                            allDrivesEncrypted = false;
                        }
                    } catch (...) {
                        allDrivesEncrypted = false;
                    }
                } else {
                    allDrivesEncrypted = false;
                }
            } else {
                allDrivesEncrypted = false;
            }
            
            pos += 10;
        }
    }
    
    std::stringstream details;
    if (!bitLockerAvailable) {
        details << "BitLocker is not available on this system.\n";
        details << "BitLocker requires Windows Pro, Enterprise, or Education edition.";
    } else {
        details << "Total Drives Checked: " << totalDrives << "\n";
        details << "Fully Encrypted Drives: " << encryptedDrives << "\n";
        details << "System Drive (C:) Encrypted: " << (systemDriveEncrypted ? "Yes" : "No");
    }
    
    result.details.push_back(details.str());
    
    // Calculate score
    int score = 0;
    
    if (!bitLockerAvailable) {
        score = 0;
        result.status = CheckStatus::NotApplicable;
        result.severity = Severity::Medium;
        result.recommendation = "BitLocker is not available on this Windows edition. Consider upgrading to Windows Pro/Enterprise/Education or using third-party encryption software.";
    } else if (systemDriveEncrypted && allDrivesEncrypted) {
        score = 100;
        result.status = CheckStatus::Pass;
        result.severity = Severity::Low;
        result.recommendation = "All drives are encrypted with BitLocker. System is compliant.";
    } else if (systemDriveEncrypted) {
        score = 70;
        result.status = CheckStatus::Warning;
        result.severity = Severity::Medium;
        result.recommendation = "System drive is encrypted but other drives are not. Enable BitLocker on all drives for complete protection.";
    } else {
        score = 20;
        result.status = CheckStatus::Fail;
        result.severity = Severity::High;
        result.recommendation = "System drive is not encrypted. Enable BitLocker encryption immediately to protect sensitive data.";
    }
    
    result.score = score;
    
    return result;
}

