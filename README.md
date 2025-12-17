# Windows Compliance Tool v2.0

A desktop-based compliance auditing application designed to analyze the security posture of a Windows operating system against selected parameters inspired by recognized cybersecurity standards, including NIST SP 800-53, ISO/IEC 27001:2022, and CIS Controls frameworks.

## Features

### Compliance Check Modules

1. **Firewall Status Check** - Verifies Windows Firewall is enabled for all profiles
2. **Antivirus Status Check** - Detects active and updated antivirus software
3. **Password Policy Review** - Evaluates password length, complexity, and expiration
4. **Windows Update Status** - Checks automatic updates and patch management
5. **User Account Review** - Lists local accounts and identifies admin-level users
6. **Disk Encryption (BitLocker)** - Verifies encryption status for system drives
7. **System Logging Verification** - Checks Windows Event Logging configuration
8. **Installed Software Audit** - Lists applications and flags potentially risky ones
9. **Network Shares Check** - Identifies open network shares and permissions

### Key Features

- **Automated Compliance Scanning** - Performs comprehensive system security checks
- **Compliance Score Calculation** - Generates overall score (0-100%) based on scan results
- **Data Structure Demonstrations** - Uses Priority Queue, Map, and Linked List for organizing results
- **Report Generation** - Exports detailed reports in text and JSON formats
- **Scan Customization** - Select specific modules to scan or perform full system check
- **Visual Dashboard** - Displays compliance score and categorized results

## Requirements

- **Operating System**: Windows 10/11
- **Development Environment**: Visual Studio 2022
- **Compiler**: MSVC with C++17 support
- **Prerequisites**: 
  - Windows SDK
  - Administrator privileges (for some checks)

## Building the Project

### Using Visual Studio 2022

1. Open `WindowsComplianceTool.sln` in Visual Studio 2022
2. Select the configuration (Debug or Release) and platform (x64)
3. Build the solution (Build > Build Solution or Ctrl+Shift+B)
4. Run the application (Debug > Start Debugging or F5)

### Project Structure

```
WindowsComplianceTool/
├── ComplianceCheck.h/cpp          # Base class for compliance checks
├── ComplianceResult.h/cpp          # Data structures for results
├── ComplianceEngine.h/cpp          # Orchestrates all checks
├── ReportGenerator.h/cpp           # Generates text and JSON reports
├── FirewallCheck.h/cpp             # Firewall compliance check
├── AntivirusCheck.h/cpp            # Antivirus compliance check
├── PasswordPolicyCheck.h/cpp       # Password policy check
├── WindowsUpdateCheck.h/cpp        # Windows Update check
├── UserAccountCheck.h/cpp          # User account review
├── BitLockerCheck.h/cpp            # BitLocker encryption check
├── SystemLoggingCheck.h/cpp        # Event logging check
├── InstalledSoftwareCheck.h/cpp    # Installed software audit
├── NetworkSharesCheck.h/cpp        # Network shares check
├── MainForm.h/cpp                  # Windows Forms GUI
└── Main.cpp                        # Application entry point
```

## Usage

### Running the Application

1. **Launch the application** - Run `WindowsComplianceTool.exe` (requires administrator privileges for some checks)
2. **Select modules** - Use checkboxes to select which compliance modules to scan
3. **Start scan** - Click "Start Full Scan" button
4. **View results** - Results are displayed in the results area with compliance score
5. **Export reports** - Use "Export Text Report" or "Export JSON Report" buttons to save results

### Understanding Results

- **Compliance Score**: Overall percentage (0-100%) indicating system compliance
- **Status**: Pass, Fail, Warning, or Not Applicable for each module
- **Severity**: Low, Medium, High, or Critical for identified issues
- **Recommendations**: Actionable guidance for improving compliance

### Report Formats

#### Text Report
- Detailed human-readable report with all scan results
- Includes DSA demonstrations (Priority Queue, Map, Linked List operations)
- Shows compliance score and recommendations

#### JSON Report
- Machine-readable format for integration with other tools
- Contains all module results with structured data
- Suitable for automated processing

## Data Structures Used

The project demonstrates the use of several data structures:

1. **Priority Queue** - Organizes issues by severity (most severe first)
2. **Map** - Buckets issues into severity groups
3. **Linked List** - Stores and manipulates scan results

These are demonstrated in the detailed text report output.

## Compliance Standards

The tool checks configurations against:

- **NIST SP 800-53** - Security and privacy controls
- **ISO/IEC 27001:2022** - Information security management
- **CIS Controls** - Critical security controls

## Limitations

- **Local System Only** - Scans only the local Windows system
- **Static Configuration Checks** - Does not perform active vulnerability scanning
- **No Automated Remediation** - Provides recommendations but does not fix issues automatically
- **Windows Edition Requirements** - Some checks (e.g., BitLocker) require Pro/Enterprise/Education editions

## Future Enhancements

Potential improvements for future versions:

1. Real-time background monitoring
2. CVE integration for vulnerability detection
3. Multi-OS support (Linux, macOS)
4. Cloud reporting and SIEM integration
5. Automated remediation capabilities

## Troubleshooting

### Common Issues

1. **"Access Denied" errors**: Run the application as Administrator
2. **Some checks return "Not Applicable"**: May require specific Windows editions or features
3. **PowerShell execution errors**: Ensure PowerShell execution policy allows script execution
4. **WMI access errors**: Ensure Windows Management Instrumentation service is running

### Build Issues

1. **Missing Windows SDK**: Install Windows 10/11 SDK via Visual Studio Installer
2. **Linker errors**: Ensure all required libraries are linked (comctl32.lib, wbemuuid.lib, etc.)
3. **CLR support errors**: Ensure project is configured for C++/CLI if using managed code

## License

This project is developed for educational purposes as part of a Data Structures and Algorithms course.

## Author

Developed as a semester project demonstrating DSA concepts in a cybersecurity compliance tool.

## Version

v2.0 - Windows Forms GUI version with enhanced compliance capabilities

