#include "AntivirusCheck.h"
#include <sstream>
#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>

#pragma comment(lib, "wbemuuid.lib")

ComplianceResult AntivirusCheck::performCheck() {
    ComplianceResult result("Antivirus Status",
                           "Detects if an antivirus or endpoint protection software is active and updated.",
                           CheckStatus::Pass, Severity::Low, 100);
    
    result.details.push_back("Checking antivirus status...");
    
    bool antivirusFound = false;
    bool antivirusEnabled = false;
    bool antivirusUpdated = false;
    std::string antivirusName = "Unknown";
    
    // Check Windows Defender status using WMI
    HRESULT hres;
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (SUCCEEDED(hres)) {
        hres = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_NONE,
                                   RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
        
        if (SUCCEEDED(hres)) {
            hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                                   IID_IWbemLocator, (LPVOID*)&pLoc);
            
            if (SUCCEEDED(hres)) {
                hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\SecurityCenter2"), nullptr, nullptr, 0, NULL, 0, 0, &pSvc);
                
                if (SUCCEEDED(hres)) {
                    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
                                            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
                    
                    if (SUCCEEDED(hres)) {
                        IEnumWbemClassObject* pEnumerator = nullptr;
                        hres = pSvc->ExecQuery(bstr_t("WQL"),
                                              bstr_t("SELECT * FROM AntiVirusProduct"),
                                              WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                              nullptr, &pEnumerator);
                        
                        if (SUCCEEDED(hres)) {
                            IWbemClassObject* pclsObj = nullptr;
                            ULONG uReturn = 0;
                            
                            while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == WBEM_S_NO_ERROR) {
                                VARIANT vtProp;
                                
                                antivirusFound = true;
                                
                                // Get product name
                                hres = pclsObj->Get(L"displayName", 0, &vtProp, 0, 0);
                                if (SUCCEEDED(hres) && vtProp.vt == VT_BSTR) {
                                    _bstr_t bstrName(vtProp.bstrVal);
                                    antivirusName = std::string((char*)bstrName);
                                }
                                VariantClear(&vtProp);
                                
                                // Get product state (0x10 = enabled, 0x00 = disabled)
                                hres = pclsObj->Get(L"productState", 0, &vtProp, 0, 0);
                                if (SUCCEEDED(hres) && vtProp.vt == VT_I4) {
                                    DWORD state = vtProp.lVal;
                                    antivirusEnabled = (state & 0x1000) != 0; // Bit 12 indicates enabled
                                    antivirusUpdated = (state & 0x10) != 0; // Bit 4 indicates up-to-date
                                }
                                VariantClear(&vtProp);
                                
                                pclsObj->Release();
                                break; // Check first antivirus found
                            }
                            
                            pEnumerator->Release();
                        }
                    }
                }
            }
        }
    }
    
    // Cleanup
    if (pSvc) pSvc->Release();
    if (pLoc) pLoc->Release();
    CoUninitialize();
    
    std::stringstream details;
    if (antivirusFound) {
        details << "Antivirus Product: " << antivirusName << "\n";
        details << "Status: " << (antivirusEnabled ? "Enabled" : "Disabled") << "\n";
        details << "Up-to-date: " << (antivirusUpdated ? "Yes" : "No");
    } else {
        details << "No antivirus product detected in Security Center.";
    }
    
    result.details.push_back(details.str());
    
    // Calculate score
    if (antivirusFound && antivirusEnabled && antivirusUpdated) {
        result.status = CheckStatus::Pass;
        result.score = 100;
        result.severity = Severity::Low;
        result.recommendation = "Antivirus is installed, enabled, and up-to-date. System is compliant.";
    } else if (antivirusFound && antivirusEnabled) {
        result.status = CheckStatus::Warning;
        result.score = 70;
        result.severity = Severity::Medium;
        result.recommendation = "Antivirus is installed and enabled but may not be up-to-date. Update your antivirus definitions.";
    } else if (antivirusFound) {
        result.status = CheckStatus::Fail;
        result.score = 40;
        result.severity = Severity::High;
        result.recommendation = "Antivirus is installed but disabled. Enable your antivirus software immediately.";
    } else {
        result.status = CheckStatus::Fail;
        result.score = 0;
        result.severity = Severity::Critical;
        result.recommendation = "No antivirus software detected. Install and enable antivirus protection immediately.";
    }
    
    return result;
}

