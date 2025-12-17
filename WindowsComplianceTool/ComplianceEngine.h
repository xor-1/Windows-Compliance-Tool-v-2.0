#pragma once
#include "ComplianceResult.h"
#include "ComplianceCheck.h"
#include "FirewallCheck.h"
#include "AntivirusCheck.h"
#include "PasswordPolicyCheck.h"
#include "WindowsUpdateCheck.h"
#include "UserAccountCheck.h"
#include "BitLockerCheck.h"
#include "SystemLoggingCheck.h"
#include "InstalledSoftwareCheck.h"
#include "NetworkSharesCheck.h"
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <memory>

// Priority queue comparator for severity-based sorting
struct SeverityComparator {
    bool operator()(const ComplianceResult& a, const ComplianceResult& b) {
        // Higher severity (higher value) should come first
        return static_cast<int>(a.severity) < static_cast<int>(b.severity);
    }
};

class ComplianceEngine {
public:
    ComplianceEngine();
    ~ComplianceEngine();
    
    // Register all compliance check modules
    void initializeChecks();
    
    // Perform all enabled compliance checks
    void performFullScan();
    
    // Perform selected compliance checks
    void performCustomScan(const std::vector<std::string>& selectedModules);
    
    // Get overall compliance score (0-100)
    int getOverallComplianceScore() const;
    
    // Get all results
    const std::vector<ComplianceResult>& getResults() const { return results; }
    
    // Get results organized by severity (using map)
    std::map<int, std::list<ComplianceResult>> getResultsBySeverity() const;
    
    // Get top issues by severity (using priority queue)
    std::priority_queue<ComplianceResult, std::vector<ComplianceResult>, SeverityComparator> getTopIssues() const;
    
    // Get results as linked list (for DSA demonstration)
    std::list<ComplianceResult> getResultsAsList() const;
    
    // Enable/disable specific check modules
    void setModuleEnabled(const std::string& moduleName, bool enabled);
    
    // Get all available module names
    std::vector<std::string> getAvailableModules() const;

private:
    std::vector<std::unique_ptr<ComplianceCheck>> checks;
    std::vector<ComplianceResult> results;
    
    void calculateOverallScore();
    int overallScore;
};

