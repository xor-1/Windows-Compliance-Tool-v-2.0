#pragma once
#include "ComplianceCheck.h"

class InstalledSoftwareCheck : public ComplianceCheck {
public:
    ComplianceResult performCheck() override;
    std::string getModuleName() const override { return "Installed Software Audit"; }
};

