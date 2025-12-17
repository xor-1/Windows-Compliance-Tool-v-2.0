#include "ComplianceEngine.h"
#include <algorithm>
#include <numeric>

ComplianceEngine::ComplianceEngine() : overallScore(0) {
    initializeChecks();
}

ComplianceEngine::~ComplianceEngine() {
}

void ComplianceEngine::initializeChecks() {
    checks.clear();

    checks.push_back(std::make_unique<FirewallCheck>());
    checks.push_back(std::make_unique<AntivirusCheck>());
    checks.push_back(std::make_unique<PasswordPolicyCheck>());
    checks.push_back(std::make_unique<WindowsUpdateCheck>());
    checks.push_back(std::make_unique<UserAccountCheck>());
    checks.push_back(std::make_unique<BitLockerCheck>());
    checks.push_back(std::make_unique<SystemLoggingCheck>());
    checks.push_back(std::make_unique<InstalledSoftwareCheck>());
    checks.push_back(std::make_unique<NetworkSharesCheck>());
}

void ComplianceEngine::performFullScan() {
    results.clear();

    for (const auto& check : checks) {
        if (check->isEnabled()) {
            ComplianceResult result = check->performCheck();
            results.push_back(result);
        }
    }

    calculateOverallScore();
}

void ComplianceEngine::performCustomScan(const std::vector<std::string>& selectedModules) {
    results.clear();

    for (const auto& check : checks) {
        std::string moduleName = check->getModuleName();

        // Check if this module is in the selected list
        bool selected = std::find(selectedModules.begin(), selectedModules.end(), moduleName) != selectedModules.end();

        if (selected && check->isEnabled()) {
            ComplianceResult result = check->performCheck();
            results.push_back(result);
        }
    }

    calculateOverallScore();
}

void ComplianceEngine::calculateOverallScore() {
    if (results.empty()) {
        overallScore = 0;
        return;
    }

    // Weighted average based on severity
    int totalWeightedScore = 0;
    int totalWeight = 0;

    for (const auto& result : results) {
        int weight = 1;

        // Higher severity gets more weight
        switch (result.severity) {
        case Severity::Critical:
            weight = 4;
            break;
        case Severity::High:
            weight = 3;
            break;
        case Severity::Medium:
            weight = 2;
            break;
        case Severity::Low:
            weight = 1;
            break;
        }

        totalWeightedScore += result.score * weight;
        totalWeight += weight;
    }

    overallScore = totalWeight > 0 ? totalWeightedScore / totalWeight : 0;
}

int ComplianceEngine::getOverallComplianceScore() const {
    return overallScore;
}

std::map<int, std::list<ComplianceResult>> ComplianceEngine::getResultsBySeverity() const {
    std::map<int, std::list<ComplianceResult>> severityMap;

    for (const auto& result : results) {
        int severityLevel = static_cast<int>(result.severity);
        severityMap[severityLevel].push_back(result);
    }

    return severityMap;
}

std::priority_queue<ComplianceResult, std::vector<ComplianceResult>, SeverityComparator>
ComplianceEngine::getTopIssues() const {
    std::priority_queue<ComplianceResult, std::vector<ComplianceResult>, SeverityComparator> pq;

    for (const auto& result : results) {
        // Only add failed or warning results
        if (result.status == CheckStatus::Fail || result.status == CheckStatus::Warning) {
            pq.push(result);
        }
    }

    return pq;
}

std::list<ComplianceResult> ComplianceEngine::getResultsAsList() const {
    std::list<ComplianceResult> resultList;

    for (const auto& result : results) {
        resultList.push_back(result);
    }

    return resultList;
}

void ComplianceEngine::setModuleEnabled(const std::string& moduleName, bool enabled) {
    for (auto& check : checks) {
        if (check->getModuleName() == moduleName) {
            check->setEnabled(enabled);
            break;
        }
    }
}

std::vector<std::string> ComplianceEngine::getAvailableModules() const {
    std::vector<std::string> moduleNames;

    for (const auto& check : checks) {
        moduleNames.push_back(check->getModuleName());
    }

    return moduleNames;
}