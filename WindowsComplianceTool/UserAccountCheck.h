#pragma once
#include "ComplianceCheck.h"

class UserAccountCheck : public ComplianceCheck {
public:
    ComplianceResult performCheck() override;
    std::string getModuleName() const override { return "User Account Review"; }
};

