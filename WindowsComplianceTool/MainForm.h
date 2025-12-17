#pragma once
#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "ComplianceEngine.h"
#include "ReportGenerator.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// Modern color scheme
struct ModernColors {
    static const COLORREF BG_DARK = RGB(18, 18, 18);
    static const COLORREF BG_CARD = RGB(30, 30, 30);
    static const COLORREF BG_CARD_HOVER = RGB(40, 40, 40);
    static const COLORREF TEXT_PRIMARY = RGB(255, 255, 255);
    static const COLORREF TEXT_SECONDARY = RGB(180, 180, 180);
    static const COLORREF ACCENT_BLUE = RGB(0, 120, 212);
    static const COLORREF ACCENT_GREEN = RGB(16, 124, 16);
    static const COLORREF ACCENT_ORANGE = RGB(247, 99, 12);
    static const COLORREF ACCENT_RED = RGB(232, 17, 35);
    static const COLORREF BORDER = RGB(60, 60, 60);
};

// Card data structure for module display
struct ModuleCard {
    std::string name;
    int score;
    CheckStatus status;
    Severity severity;
    RECT bounds;
    bool hovered;
    bool selected;
};

class MainForm {
public:
    MainForm();
    ~MainForm();

    bool Create(HINSTANCE hInstance, int nCmdShow);
    int MessageLoop();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static MainForm* GetThisFromWindow(HWND hwnd);

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void OnCreate(HWND hwnd);
    void OnCommand(WPARAM wParam);
    void OnPaint();
    void OnSize();
    void OnDestroy();
    void OnNotify(WPARAM wParam, LPARAM lParam);
    void OnMouseMove(WPARAM wParam, LPARAM lParam);
    void OnLButtonDown(WPARAM wParam, LPARAM lParam);
    void OnTimer(UINT_PTR timerID);
    void OnDrawItem(WPARAM wParam, LPARAM lParam);  // ADDED: Missing declaration

    void InitializeControls();
    void InitializeGDIPlus();
    void CleanupGDIPlus();

    // Drawing functions
    void DrawDashboard(Graphics* g);
    void DrawSystemOverview(Graphics* g);
    void DrawSecurityPolicies(Graphics* g);
    void DrawNetworkFirewall(Graphics* g);
    void DrawAntivirusUpdates(Graphics* g);
    void DrawReports(Graphics* g);

    // Chart drawing functions
    void DrawComplianceGauge(Graphics* g, int x, int y, int size, int score);
    void DrawPieChart(Graphics* g, int x, int y, int radius, const std::vector<std::pair<std::string, int>>& data);
    void DrawBarChart(Graphics* g, int x, int y, int width, int height, const std::vector<std::pair<std::string, int>>& data);
    void DrawLineGraph(Graphics* g, int x, int y, int width, int height, const std::vector<int>& data);
    void DrawModuleCards(Graphics* g);
    void DrawCard(Graphics* g, const ModuleCard& card);
    void DrawGradientBackground(Graphics* g, const Rect& rect, Color color1, Color color2);
    void DrawGlassEffect(Graphics* g, const Rect& rect);

    // Scan operations
    void PerformScan();
    void UpdateDashboard();
    void UpdateResultsDisplay();
    void EnableScanControls(bool enable);
    std::vector<std::string> GetSelectedModules();

    void CreateTabControl();
    void CreateButtons();
    void CreateModuleCheckboxes();

    // Animation
    void StartAnimation();
    void StopAnimation();
    void UpdateAnimation();

    HWND m_hwnd;
    HINSTANCE m_hInstance;

    // GDI+ members
    ULONG_PTR m_gdiplusToken;
    Graphics* m_graphics;

    // Controls - FIXED: All control handles now properly declared
    HWND m_tabControl;
    HWND m_scanButton;
    HWND m_exportTextButton;
    HWND m_exportJSONButton;

    // Module cards
    std::vector<ModuleCard> m_moduleCards;
    int m_hoveredCard;

    // Compliance engine
    ComplianceEngine m_engine;
    bool m_scanInProgress;

    // Animation state
    float m_animProgress;
    bool m_animating;
    UINT_PTR m_animTimer;

    // Historical data for graphs
    std::vector<int> m_scoreHistory;

    // Tab indices
    enum TabIndex {
        TAB_DASHBOARD = 0,
        TAB_SYSTEM_OVERVIEW = 1,
        TAB_SECURITY_POLICIES = 2,
        TAB_NETWORK_FIREWALL = 3,
        TAB_ANTIVIRUS_UPDATES = 4,
        TAB_REPORTS = 5
    };

    int m_currentTab;

    // Control IDs
    enum {
        IDC_TAB = 1000,
        IDC_SCAN_BUTTON,
        IDC_EXPORT_TEXT_BUTTON,
        IDC_EXPORT_JSON_BUTTON,
        TIMER_ANIMATION = 2000
    };

    // Canvas area for custom drawing
    RECT m_canvasRect;
};