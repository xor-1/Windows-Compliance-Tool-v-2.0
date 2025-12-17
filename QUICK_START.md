# Quick Start Guide - Windows Compliance Tool v2.0

## Getting Started in 5 Minutes

### Step 1: Open the Project
1. Launch **Visual Studio 2022**
2. File → Open → Project/Solution
3. Select `WindowsComplianceTool.sln`

### Step 2: Build the Project
1. Select **x64** platform (required)
2. Select **Debug** or **Release** configuration
3. Press `Ctrl+Shift+B` or Build → Build Solution

### Step 3: Run the Application
1. Right-click the project → **Set as StartUp Project**
2. Press `F5` to run (or `Ctrl+F5` to run without debugging)
3. **Important**: Run as Administrator for full functionality

### Step 4: Perform a Scan
1. Click **"Start Full Scan"** button
2. Wait for scan to complete (30-60 seconds)
3. View results in the results area
4. Check your **Compliance Score** at the top

### Step 5: Export Reports
1. Click **"Export Text Report"** for detailed text report
2. Click **"Export JSON Report"** for JSON format
3. Choose save location and filename

## Key Features

### Dashboard Tab
- View overall compliance score
- See scan status and progress
- Quick overview of system security

### Module Selection
- Check/uncheck modules to customize scan
- Use **"Select All Modules"** for convenience
- Each module can be scanned independently

### Results Display
- Detailed results for each module
- Status (Pass/Fail/Warning)
- Severity levels (Low/Medium/High/Critical)
- Actionable recommendations

### Report Export
- **Text Report**: Human-readable with DSA demonstrations
- **JSON Report**: Machine-readable for integration

## Compliance Modules

| Module | What It Checks |
|--------|----------------|
| Firewall Status | Windows Firewall enabled for all profiles |
| Antivirus Status | Antivirus installed, enabled, and updated |
| Password Policy | Password length, complexity, expiration |
| Windows Update | Automatic updates enabled and configured |
| User Account Review | Local accounts and admin privileges |
| BitLocker | Disk encryption status (Pro/Enterprise only) |
| System Logging | Event logging and audit policies |
| Installed Software | Applications and potential risks |
| Network Shares | Shared folders and permissions |

## Understanding Results

### Compliance Score
- **90-100%**: Excellent compliance
- **70-89%**: Good, minor improvements needed
- **50-69%**: Fair, several issues to address
- **0-49%**: Poor, significant security gaps

### Status Indicators
- **Pass** ✅: Configuration meets standards
- **Warning** ⚠️: Needs attention but not critical
- **Fail** ❌: Does not meet security standards
- **Not Applicable** ℹ️: Check not relevant to this system

### Severity Levels
- **Low**: Minor issue, low risk
- **Medium**: Moderate issue, should be addressed
- **High**: Significant issue, security risk
- **Critical**: Major security vulnerability

## Tips for Best Results

1. **Run as Administrator**: Required for accurate results
2. **Full Scan First**: Run full scan to see overall status
3. **Review Recommendations**: Each module provides specific guidance
4. **Export Reports**: Save reports for documentation and tracking
5. **Regular Scans**: Run scans periodically to track compliance

## Troubleshooting

### "Access Denied" Errors
→ Run the application as Administrator

### Some Checks Show "Not Applicable"
→ May require specific Windows edition (e.g., BitLocker needs Pro/Enterprise)

### Scan Takes Too Long
→ Normal for full scan (30-60 seconds). Software audit can be slower on systems with many applications.

### No Results Displayed
→ Ensure at least one module is selected before scanning

## Next Steps

1. Review the **README.md** for detailed documentation
2. Check **BUILD_INSTRUCTIONS.md** for build troubleshooting
3. Read **PROJECT_SUMMARY.md** for architecture details
4. Export reports and review recommendations
5. Address identified security gaps

## Support

For issues or questions:
1. Check error messages in the status label
2. Review build output in Visual Studio
3. Verify all prerequisites are installed
4. Ensure Windows SDK is properly configured

---

**Ready to start?** Open the solution in Visual Studio and press F5!

