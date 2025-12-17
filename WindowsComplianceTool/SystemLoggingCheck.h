#pragma once
#include "ComplianceCheck.h"

class SystemLoggingCheck : public ComplianceCheck {
public:
    ComplianceResult performCheck() override;
    std::string getModuleName() const override { return "System Logging Verification"; }
};

