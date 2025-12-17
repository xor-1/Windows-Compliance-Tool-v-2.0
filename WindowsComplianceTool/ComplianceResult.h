#pragma once
#include <string>
#include <vector>

// Severity levels for compliance issues
enum class Severity {
    Low = 20,
    Medium = 40,
    High = 60,
    Critical = 80
};

// Status of a compliance check
enum class CheckStatus {
    Pass,
    Fail,
    Warning,
    NotApplicable
};

// Structure to hold individual compliance check results
struct ComplianceResult {
    std::string moduleName;
    std::string description;
    CheckStatus status;
    Severity severity;
    std::string recommendation;
    std::vector<std::string> details;
    int score; // 0-100 for this module

    ComplianceResult() : status(CheckStatus::NotApplicable), severity(Severity::Low), score(0) {}
    
    ComplianceResult(const std::string& name, const std::string& desc, 
                    CheckStatus stat, Severity sev, int sc = 0)
        : moduleName(name), description(desc), status(stat), severity(sev), score(sc) {}
};

