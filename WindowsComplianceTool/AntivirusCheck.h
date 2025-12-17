#pragma once
#include "ComplianceCheck.h"

class AntivirusCheck : public ComplianceCheck {
public:
    ComplianceResult performCheck() override;
    std::string getModuleName() const override { return "Antivirus Status"; }
};

