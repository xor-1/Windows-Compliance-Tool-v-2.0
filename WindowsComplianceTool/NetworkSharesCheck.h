#pragma once
#include "ComplianceCheck.h"

class NetworkSharesCheck : public ComplianceCheck {
public:
    ComplianceResult performCheck() override;
    std::string getModuleName() const override { return "Network Shares Check"; }
};
