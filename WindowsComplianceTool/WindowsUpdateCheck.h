#pragma once
#include "ComplianceCheck.h"

class WindowsUpdateCheck : public ComplianceCheck {
public:
    ComplianceResult performCheck() override;
    std::string getModuleName() const override { return "Windows Update Status"; }
};

