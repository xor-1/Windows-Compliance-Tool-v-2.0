#pragma once
#include "ComplianceResult.h"
#include "ComplianceEngine.h"
#include <string>
#include <fstream>

class ReportGenerator {
public:
    // Generate text report
    static bool generateTextReport(const ComplianceEngine& engine, const std::string& filename);
    
    // Generate JSON report
    static bool generateJSONReport(const ComplianceEngine& engine, const std::string& filename);
    
    // Generate formatted text report with DSA demonstrations
    static bool generateDetailedTextReport(const ComplianceEngine& engine, const std::string& filename);

private:
    static std::string escapeJSON(const std::string& str);
    static std::string statusToString(CheckStatus status);
    static std::string severityToString(Severity severity);
};

