#pragma once
#include "ComplianceCheck.h"

class BitLockerCheck : public ComplianceCheck {
public:
    ComplianceResult performCheck() override;
    std::string getModuleName() const override { return "Disk Encryption (BitLocker)"; }
};

