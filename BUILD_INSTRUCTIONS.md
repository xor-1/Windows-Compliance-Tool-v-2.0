# Build Instructions for Windows Compliance Tool v2.0

## Prerequisites

1. **Visual Studio 2022** (Community, Professional, or Enterprise edition)
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Ensure the following workloads are installed:
     - Desktop development with C++
     - Windows 10/11 SDK (latest version)

2. **Windows 10/11** operating system

3. **Administrator privileges** (required for running the application and some compliance checks)

## Building the Project

### Method 1: Using Visual Studio IDE

1. **Open the Solution**
   - Launch Visual Studio 2022
   - File → Open → Project/Solution
   - Navigate to the project directory and select `WindowsComplianceTool.sln`

2. **Configure Build Settings**
   - Select configuration: **Debug** or **Release**
   - Select platform: **x64** (required)
   - Build → Configuration Manager → Ensure "x64" is selected for all projects

3. **Build the Solution**
   - Build → Build Solution (or press `Ctrl+Shift+B`)
   - Wait for compilation to complete
   - Check Output window for any errors

4. **Run the Application**
   - Debug → Start Debugging (or press `F5`)
   - Or: Debug → Start Without Debugging (or press `Ctrl+F5`)

### Method 2: Using Command Line (MSBuild)

1. **Open Developer Command Prompt for VS 2022**
   - Start menu → Visual Studio 2022 → Developer Command Prompt for VS 2022

2. **Navigate to Project Directory**
   ```cmd
   cd path\to\WindowsComplianceTool
   ```

3. **Build the Solution**
   ```cmd
   msbuild WindowsComplianceTool.sln /p:Configuration=Release /p:Platform=x64
   ```

4. **Run the Executable**
   ```cmd
   WindowsComplianceTool\x64\Release\WindowsComplianceTool.exe
   ```

## Troubleshooting Build Issues

### Issue: "Cannot open include file: 'windows.h'"
**Solution**: Install Windows SDK via Visual Studio Installer
- Tools → Get Tools and Features
- Individual components → Windows 10/11 SDK

### Issue: "Unresolved external symbol" errors
**Solution**: Ensure all required libraries are linked:
- `comctl32.lib` (Common Controls)
- `wbemuuid.lib` (WMI)
- `comdlg32.lib` (Common Dialogs)
- `shell32.lib` (Shell functions)

These should be automatically linked via `#pragma comment(lib, ...)` directives in the code.

### Issue: "CLR support" errors
**Solution**: The project uses native C++ with Win32 API, not C++/CLI. Ensure:
- Project properties → C/C++ → General → Common Language Runtime Support = **No Common Language Runtime Support**

### Issue: Linker errors related to WMI
**Solution**: 
- Ensure `wbemuuid.lib` is linked (check `ComplianceCheck.cpp`)
- Verify Windows Management Instrumentation service is running:
  ```cmd
  sc query winmgmt
  ```

### Issue: "Access Denied" when running
**Solution**: 
- Right-click executable → Run as Administrator
- Some compliance checks require elevated privileges

## Project Configuration

### Required Project Settings

1. **C++ Language Standard**: C++17 or later
   - Project Properties → C/C++ → Language → C++ Language Standard = **ISO C++17**

2. **Character Set**: Unicode
   - Project Properties → General → Character Set = **Use Unicode Character Set**

3. **Platform**: x64
   - Project Properties → General → Platform Toolset = **v143** (VS 2022)

## Output Location

After successful build:
- **Debug**: `WindowsComplianceTool\x64\Debug\WindowsComplianceTool.exe`
- **Release**: `WindowsComplianceTool\x64\Release\WindowsComplianceTool.exe`

## Testing the Build

1. **Run the application** with administrator privileges
2. **Click "Start Full Scan"** to test all compliance modules
3. **Verify results** are displayed in the results area
4. **Test report export** by clicking "Export Text Report" or "Export JSON Report"

## Additional Notes

- The application requires **.NET Framework** is NOT required (pure native C++)
- Some checks may fail on Windows Home editions (e.g., BitLocker requires Pro/Enterprise/Education)
- PowerShell execution policy may need adjustment for some checks:
  ```powershell
  Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
  ```

## Support

For build issues:
1. Check Visual Studio Output window for detailed error messages
2. Verify all prerequisites are installed
3. Ensure project files are not corrupted
4. Try cleaning and rebuilding:
   - Build → Clean Solution
   - Build → Rebuild Solution

