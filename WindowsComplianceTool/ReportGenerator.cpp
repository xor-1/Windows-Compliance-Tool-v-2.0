#include "ReportGenerator.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

std::string ReportGenerator::escapeJSON(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

std::string ReportGenerator::statusToString(CheckStatus status) {
    switch (status) {
        case CheckStatus::Pass: return "Pass";
        case CheckStatus::Fail: return "Fail";
        case CheckStatus::Warning: return "Warning";
        case CheckStatus::NotApplicable: return "Not Applicable";
        default: return "Unknown";
    }
}

std::string ReportGenerator::severityToString(Severity severity) {
    switch (severity) {
        case Severity::Low: return "Low";
        case Severity::Medium: return "Medium";
        case Severity::High: return "High";
        case Severity::Critical: return "Critical";
        default: return "Unknown";
    }
}

bool ReportGenerator::generateTextReport(const ComplianceEngine& engine, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "========================================\n";
    file << "Windows Compliance Tool v2.0\n";
    file << "Compliance Audit Report\n";
    file << "========================================\n\n";
    
    file << "Overall Compliance Score: " << engine.getOverallComplianceScore() << "%\n\n";
    
    const auto& results = engine.getResults();
    
    file << "Module Results:\n";
    file << "----------------------------------------\n";
    
    for (const auto& result : results) {
        file << "\nModule: " << result.moduleName << "\n";
        file << "Status: " << statusToString(result.status) << "\n";
        file << "Score: " << result.score << "%\n";
        file << "Severity: " << severityToString(result.severity) << "\n";
        file << "Description: " << result.description << "\n";
        file << "Recommendation: " << result.recommendation << "\n";
        
        if (!result.details.empty()) {
            file << "Details:\n";
            for (const auto& detail : result.details) {
                file << "  " << detail << "\n";
            }
        }
        file << "\n";
    }
    
    file.close();
    return true;
}

bool ReportGenerator::generateJSONReport(const ComplianceEngine& engine, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{\n";
    file << "  \"reportVersion\": \"2.0\",\n";
    file << "  \"overallScore\": " << engine.getOverallComplianceScore() << ",\n";
    file << "  \"modules\": [\n";
    
    const auto& results = engine.getResults();
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& result = results[i];
        
        file << "    {\n";
        file << "      \"moduleName\": \"" << escapeJSON(result.moduleName) << "\",\n";
        file << "      \"description\": \"" << escapeJSON(result.description) << "\",\n";
        file << "      \"status\": \"" << statusToString(result.status) << "\",\n";
        file << "      \"severity\": \"" << severityToString(result.severity) << "\",\n";
        file << "      \"score\": " << result.score << ",\n";
        file << "      \"recommendation\": \"" << escapeJSON(result.recommendation) << "\",\n";
        file << "      \"details\": [\n";
        
        for (size_t j = 0; j < result.details.size(); ++j) {
            file << "        \"" << escapeJSON(result.details[j]) << "\"";
            if (j < result.details.size() - 1) file << ",";
            file << "\n";
        }
        
        file << "      ]\n";
        file << "    }";
        if (i < results.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    return true;
}

bool ReportGenerator::generateDetailedTextReport(const ComplianceEngine& engine, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "========================================\n";
    file << "Windows Compliance Tool v2.0\n";
    file << "Detailed Compliance Audit Report\n";
    file << "========================================\n\n";
    
    file << "Overall Compliance Score: " << engine.getOverallComplianceScore() << "%\n\n";
    
    // DSA Demo: Priority Queue of Issues (most severe first)
    file << "-- DSA Demo: Priority Queue of Issues (most severe first) --\n";
    file << "Top issues by severity:\n\n";
    
    auto topIssues = engine.getTopIssues();
    int issueNum = 1;
    auto topIssuesCopy = topIssues; // Copy because priority_queue doesn't support iteration
    
    while (!topIssuesCopy.empty()) {
        auto issue = topIssuesCopy.top();
        topIssuesCopy.pop();
        file << issueNum << ") Module: " << issue.moduleName 
             << " -- Severity: " << static_cast<int>(issue.severity) << "\n";
        issueNum++;
    }
    
    file << "\n\n";
    
    // DSA Demo: Bucket issues into severity groups using map/list
    file << "-- DSA Demo: Bucket issues into severity groups using map/list --\n";
    auto severityMap = engine.getResultsBySeverity();
    
    for (auto it = severityMap.rbegin(); it != severityMap.rend(); ++it) {
        file << "Severity " << it->first << ": ";
        bool first = true;
        for (const auto& result : it->second) {
            if (!first) file << ", ";
            file << result.moduleName;
            first = false;
        }
        file << ",\n";
    }
    
    file << "\n\n";
    
    // DSA Demo: Linked-list operations on results
    file << "-- DSA Demo: Linked-list operations on results (std::list) --\n";
    auto resultList = engine.getResultsAsList();
    
    file << "Original order (as scanned):\n";
    for (const auto& result : resultList) {
        file << "- " << result.moduleName << " (" << result.score << ")\n";
    }
    
    // Move worst issue to front
    if (!resultList.empty()) {
        auto worstIt = resultList.begin();
        int worstScore = worstIt->score;
        
        for (auto it = resultList.begin(); it != resultList.end(); ++it) {
            if (it->score < worstScore) {
                worstScore = it->score;
                worstIt = it;
            }
        }
        
        if (worstIt != resultList.begin()) {
            resultList.splice(resultList.begin(), resultList, worstIt);
        }
        
        file << "\nAfter moving worst issue to front:\n";
        for (const auto& result : resultList) {
            file << "- " << result.moduleName << " (" << result.score << ")\n";
        }
    }
    
    file << "\n\n";
    
    // Regular module results
    file << "========================================\n";
    file << "Detailed Module Results:\n";
    file << "========================================\n\n";
    
    const auto& results = engine.getResults();
    for (const auto& result : results) {
        file << "Module: " << result.moduleName << "\n";
        file << "Status: " << statusToString(result.status) << "\n";
        file << "Score: " << result.score << "%\n";
        file << "Severity: " << severityToString(result.severity) << "\n";
        file << "Description: " << result.description << "\n";
        file << "Recommendation: " << result.recommendation << "\n";
        
        if (!result.details.empty()) {
            file << "Details:\n";
            for (const auto& detail : result.details) {
                file << "  " << detail << "\n";
            }
        }
        file << "\n";
    }
    
    file.close();
    return true;
}

