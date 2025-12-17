#pragma once
#include "ComplianceCheck.h"

class PasswordPolicyCheck : public ComplianceCheck {
public:
    ComplianceResult performCheck() override;
    std::string getModuleName() const override { return "Password Policy Review"; }
};

