#pragma once
#include "ComplianceCheck.h"

class FirewallCheck : public ComplianceCheck {
public:
    ComplianceResult performCheck() override;
    std::string getModuleName() const override { return "Firewall Status"; }
};

