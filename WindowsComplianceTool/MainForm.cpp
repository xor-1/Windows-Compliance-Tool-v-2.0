#include "MainForm.h"
#include <sstream>
#include <algorithm>
#include <cmath>
#include <commdlg.h>

#pragma comment(lib, "comdlg32.lib")

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MainForm* MainForm::GetThisFromWindow(HWND hwnd) {
    return reinterpret_cast<MainForm*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

LRESULT CALLBACK MainForm::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainForm* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainForm*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        if (pThis) {
            pThis->m_hwnd = hwnd;
        }
    }
    else {
        pThis = GetThisFromWindow(hwnd);
    }

    if (pThis) {
        return pThis->HandleMessage(uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

MainForm::MainForm()
    : m_hwnd(nullptr), m_hInstance(nullptr), m_currentTab(0),
    m_scanInProgress(false), m_gdiplusToken(0), m_graphics(nullptr),
    m_animProgress(0.0f), m_animating(false), m_animTimer(0),
    m_hoveredCard(-1), m_tabControl(nullptr), m_scanButton(nullptr),
    m_exportTextButton(nullptr), m_exportJSONButton(nullptr) {
    m_canvasRect = { 0, 0, 0, 0 };
}

MainForm::~MainForm() {
    CleanupGDIPlus();
}

bool MainForm::Create(HINSTANCE hInstance, int nCmdShow) {
    m_hInstance = hInstance;

    InitializeGDIPlus();

    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(ModernColors::BG_DARK);
    wcex.lpszClassName = L"ModernComplianceToolForm";
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    RegisterClassEx(&wcex);

    m_hwnd = CreateWindowEx(
        0,
        L"ModernComplianceToolForm",
        L"Windows Compliance Tool",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1400, 900,
        nullptr, nullptr, hInstance, this
    );

    if (!m_hwnd) return false;

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);

    return true;
}

int MainForm::MessageLoop() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

LRESULT MainForm::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        if (!m_hwnd) return -1;
        PostMessage(m_hwnd, WM_APP + 1, 0, 0);
        return 0;

    case WM_APP + 1:
        if (IsWindow(m_hwnd)) {
            OnCreate(m_hwnd);
        }
        return 0;

    case WM_COMMAND:
        OnCommand(wParam);
        return 0;

    case WM_NOTIFY:
        OnNotify(wParam, lParam);
        return 0;

    case WM_PAINT:
        OnPaint();
        return 0;

    case WM_SIZE:
        OnSize();
        return 0;

    case WM_MOUSEMOVE:
        OnMouseMove(wParam, lParam);
        return 0;

    case WM_LBUTTONDOWN:
        OnLButtonDown(wParam, lParam);
        return 0;

    case WM_TIMER:
        OnTimer(wParam);
        return 0;

    case WM_DRAWITEM:
        OnDrawItem(wParam, lParam);
        return TRUE;

    case WM_DESTROY:
        OnDestroy();
        return 0;
    }

    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

void MainForm::OnDrawItem(WPARAM wParam, LPARAM lParam) {
    LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;

    if (lpDrawItem->CtlType == ODT_TAB) {
        RECT rect = lpDrawItem->rcItem;
        bool isSelected = (lpDrawItem->itemState & ODS_SELECTED);

        HBRUSH bgBrush = CreateSolidBrush(isSelected ? RGB(0, 120, 212) : RGB(30, 30, 30));
        FillRect(lpDrawItem->hDC, &rect, bgBrush);
        DeleteObject(bgBrush);

        wchar_t text[256];
        TCITEM item = {};
        item.mask = TCIF_TEXT;
        item.pszText = text;
        item.cchTextMax = 256;
        TabCtrl_GetItem(m_tabControl, lpDrawItem->itemID, &item);

        SetBkMode(lpDrawItem->hDC, TRANSPARENT);
        SetTextColor(lpDrawItem->hDC, RGB(255, 255, 255));

        HFONT hFont = CreateFont(14, 0, 0, 0, isSelected ? FW_BOLD : FW_NORMAL,
            FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        HGDIOBJ oldFont = SelectObject(lpDrawItem->hDC, hFont);

        DrawText(lpDrawItem->hDC, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(lpDrawItem->hDC, oldFont);
        DeleteObject(hFont);

        if (isSelected) {
            HPEN pen = CreatePen(PS_SOLID, 3, RGB(0, 120, 212));
            HGDIOBJ oldPen = SelectObject(lpDrawItem->hDC, pen);
            MoveToEx(lpDrawItem->hDC, rect.left, rect.bottom - 2, NULL);
            LineTo(lpDrawItem->hDC, rect.right, rect.bottom - 2);
            SelectObject(lpDrawItem->hDC, oldPen);
            DeleteObject(pen);
        }
    }
    else if (lpDrawItem->CtlType == ODT_BUTTON) {
        wchar_t text[256];
        GetWindowText(lpDrawItem->hwndItem, text, 256);

        bool isPressed = (lpDrawItem->itemState & ODS_SELECTED);
        bool isFocused = (lpDrawItem->itemState & ODS_FOCUS);

        HBRUSH bgBrush = CreateSolidBrush(isPressed ? RGB(40, 40, 40) : RGB(30, 30, 30));
        FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, bgBrush);
        DeleteObject(bgBrush);

        HPEN pen = CreatePen(PS_SOLID, 2, isFocused ? RGB(0, 150, 230) : RGB(0, 120, 212));
        HGDIOBJ oldPen = SelectObject(lpDrawItem->hDC, pen);
        HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HGDIOBJ oldBrush = SelectObject(lpDrawItem->hDC, nullBrush);

        RoundRect(lpDrawItem->hDC,
            lpDrawItem->rcItem.left,
            lpDrawItem->rcItem.top,
            lpDrawItem->rcItem.right,
            lpDrawItem->rcItem.bottom,
            8, 8);

        SelectObject(lpDrawItem->hDC, oldPen);
        SelectObject(lpDrawItem->hDC, oldBrush);
        DeleteObject(pen);

        SetBkMode(lpDrawItem->hDC, TRANSPARENT);
        SetTextColor(lpDrawItem->hDC, RGB(255, 255, 255));

        HFONT hFont = CreateFont(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        HGDIOBJ oldFont = SelectObject(lpDrawItem->hDC, hFont);

        DrawText(lpDrawItem->hDC, text, -1, (LPRECT)&lpDrawItem->rcItem,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(lpDrawItem->hDC, oldFont);
        DeleteObject(hFont);
    }
}

void MainForm::OnCreate(HWND hwnd) {
    m_hwnd = hwnd;
    InitializeControls();
}

void MainForm::InitializeControls() {
    if (!IsWindow(m_hwnd)) return;

    CreateTabControl();
    CreateButtons();
    OnSize();
}

void MainForm::InitializeGDIPlus() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

void MainForm::CleanupGDIPlus() {
    if (m_graphics) {
        delete m_graphics;
        m_graphics = nullptr;
    }
    if (m_gdiplusToken) {
        GdiplusShutdown(m_gdiplusToken);
        m_gdiplusToken = 0;
    }
}

void MainForm::CreateTabControl() {
    m_tabControl = CreateWindowEx(
        0, WC_TABCONTROL, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_OWNERDRAWFIXED | TCS_HOTTRACK,
        20, 20, 100, 35,
        m_hwnd, (HMENU)IDC_TAB, nullptr, nullptr
    );

    const wchar_t* tabNames[] = {
        L" Dashboard",
        L" System",
        L" Security",
        L" Network",
        L" Antivirus",
        L" Reports"
    };

    TCITEM tie = {};
    tie.mask = TCIF_TEXT;

    for (int i = 0; i < 6; i++) {
        tie.pszText = (LPWSTR)tabNames[i];
        TabCtrl_InsertItem(m_tabControl, i, &tie);
    }
}

void MainForm::CreateButtons() {
    m_scanButton = CreateWindowEx(
        0, L"BUTTON", L" Start Full Scan",
        WS_CHILD | BS_OWNERDRAW,
        20, 75, 180, 42,
        m_hwnd, (HMENU)IDC_SCAN_BUTTON, nullptr, nullptr
    );

    m_exportTextButton = CreateWindowEx(
        0, L"BUTTON", L" Export Text",
        WS_CHILD | BS_OWNERDRAW,
        20, 75, 150, 42,
        m_hwnd, (HMENU)IDC_EXPORT_TEXT_BUTTON, nullptr, nullptr
    );

    m_exportJSONButton = CreateWindowEx(
        0, L"BUTTON", L" Export JSON",
        WS_CHILD | BS_OWNERDRAW,
        190, 75, 150, 42,
        m_hwnd, (HMENU)IDC_EXPORT_JSON_BUTTON, nullptr, nullptr
    );
}

void MainForm::CreateModuleCheckboxes() {
    // Placeholder implementation
}

void MainForm::OnCommand(WPARAM wParam) {
    int id = LOWORD(wParam);

    if (id == IDC_SCAN_BUTTON) {
        PerformScan();
    }
    else if (id == IDC_EXPORT_TEXT_BUTTON) {
        OPENFILENAMEW ofn = {};
        wchar_t szFile[260] = L"compliance_report.txt";

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hwnd;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
        ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

        if (GetSaveFileNameW(&ofn)) {
            std::string filename;
            int len = WideCharToMultiByte(CP_UTF8, 0, szFile, -1, nullptr, 0, nullptr, nullptr);
            filename.resize(len);
            WideCharToMultiByte(CP_UTF8, 0, szFile, -1, &filename[0], len, nullptr, nullptr);
            filename.pop_back();

            ReportGenerator::generateDetailedTextReport(m_engine, filename);
            MessageBoxA(m_hwnd, ("Report exported: " + filename).c_str(), "Success", MB_OK | MB_ICONINFORMATION);
        }
    }
    else if (id == IDC_EXPORT_JSON_BUTTON) {
        OPENFILENAMEW ofn = {};
        wchar_t szFile[260] = L"compliance_report.json";

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hwnd;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
        ofn.lpstrFilter = L"JSON Files\0*.json\0All Files\0*.*\0";
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

        if (GetSaveFileNameW(&ofn)) {
            std::string filename;
            int len = WideCharToMultiByte(CP_UTF8, 0, szFile, -1, nullptr, 0, nullptr, nullptr);
            filename.resize(len);
            WideCharToMultiByte(CP_UTF8, 0, szFile, -1, &filename[0], len, nullptr, nullptr);
            filename.pop_back();

            ReportGenerator::generateJSONReport(m_engine, filename);
            MessageBoxA(m_hwnd, ("Report exported: " + filename).c_str(), "Success", MB_OK | MB_ICONINFORMATION);
        }
    }
}

void MainForm::OnNotify(WPARAM wParam, LPARAM lParam) {
    NMHDR* nmhdr = (NMHDR*)lParam;
    if (nmhdr->idFrom == IDC_TAB && nmhdr->code == TCN_SELCHANGE) {
        m_currentTab = TabCtrl_GetCurSel(m_tabControl);

        ShowWindow(m_scanButton, (m_currentTab == TAB_DASHBOARD) ? SW_SHOW : SW_HIDE);
        ShowWindow(m_exportTextButton, (m_currentTab == TAB_REPORTS) ? SW_SHOW : SW_HIDE);
        ShowWindow(m_exportJSONButton, (m_currentTab == TAB_REPORTS) ? SW_SHOW : SW_HIDE);

        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}// MainForm.cpp - Part 2: Drawing and Paint Functions

void MainForm::OnPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);

    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

    Graphics* g = Graphics::FromHDC(hdcMem);
    g->SetSmoothingMode(SmoothingModeAntiAlias);
    g->SetTextRenderingHint(TextRenderingHintAntiAlias);

    SolidBrush bgBrush(Color(255, 18, 18, 18));
    g->FillRectangle(&bgBrush, 0, 0, clientRect.right, clientRect.bottom);

    switch (m_currentTab) {
    case TAB_DASHBOARD:
        DrawDashboard(g);
        break;
    case TAB_SYSTEM_OVERVIEW:
        DrawSystemOverview(g);
        break;
    case TAB_SECURITY_POLICIES:
        DrawSecurityPolicies(g);
        break;
    case TAB_NETWORK_FIREWALL:
        DrawNetworkFirewall(g);
        break;
    case TAB_ANTIVIRUS_UPDATES:
        DrawAntivirusUpdates(g);
        break;
    case TAB_REPORTS:
        DrawReports(g);
        break;
    }

    delete g;

    BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);

    EndPaint(m_hwnd, &ps);
}

void MainForm::DrawDashboard(Graphics* g) {
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);

    int startY = 140;
    int score = m_engine.getOverallComplianceScore();

    int gaugeSize = 220;
    int gaugeX = 60;
    int gaugeY = startY + 20;
    DrawComplianceGauge(g, gaugeX, gaugeY, gaugeSize, score);

    int textStartX = gaugeX + gaugeSize + 50;
    std::wstring scoreText = L"Overall Compliance: " + std::to_wstring(score) + L"%";
    Font titleFont(L"Segoe UI", 26, FontStyleBold);
    SolidBrush textBrush(Color(255, 255, 255, 255));
    g->DrawString(scoreText.c_str(), -1, &titleFont,
        PointF(static_cast<REAL>(textStartX), static_cast<REAL>(gaugeY + 50)), &textBrush);

    Color statusColor;
    std::wstring statusText;
    if (score >= 90) {
        statusColor = Color(255, 16, 124, 16);
        statusText = L" EXCELLENT";
    }
    else if (score >= 70) {
        statusColor = Color(255, 247, 99, 12);
        statusText = L" GOOD";
    }
    else if (score >= 50) {
        statusColor = Color(255, 247, 99, 12);
        statusText = L" NEEDS IMPROVEMENT";
    }
    else {
        statusColor = Color(255, 232, 17, 35);
        statusText = L" CRITICAL";
    }

    Font statusFont(L"Segoe UI", 20, FontStyleBold);
    SolidBrush statusBrush(statusColor);
    g->DrawString(statusText.c_str(), -1, &statusFont,
        PointF(static_cast<REAL>(textStartX), static_cast<REAL>(gaugeY + 95)), &statusBrush);

    if (!m_scoreHistory.empty() && m_scoreHistory.size() > 1) {
        DrawLineGraph(g, textStartX, gaugeY + 140, 400, 80, m_scoreHistory);
    }

    const auto& results = m_engine.getResults();
    if (!results.empty()) {
        m_moduleCards.clear();
        int cardY = gaugeY + gaugeSize + 50;
        int cardX = 40;
        int cardWidth = 220;
        int cardHeight = 110;
        int spacing = 18;
        int cardsPerRow = (clientRect.right - 80) / (cardWidth + spacing);

        for (size_t i = 0; i < results.size(); i++) {
            ModuleCard card;
            card.name = results[i].moduleName;
            card.score = results[i].score;
            card.status = results[i].status;
            card.severity = results[i].severity;
            card.bounds = {
                cardX, cardY,
                cardX + cardWidth, cardY + cardHeight
            };
            card.hovered = (static_cast<int>(i) == m_hoveredCard);
            card.selected = false;

            DrawCard(g, card);
            m_moduleCards.push_back(card);

            cardX += cardWidth + spacing;
            if ((i + 1) % cardsPerRow == 0) {
                cardX = 40;
                cardY += cardHeight + spacing;
            }
        }
    }
}

void MainForm::DrawCard(Graphics* g, const ModuleCard& card) {
    Color bgColor = card.hovered ? Color(255, 40, 40, 40) : Color(255, 30, 30, 30);

    GraphicsPath path;
    int cornerRadius = 8;
    Rect rect(card.bounds.left, card.bounds.top,
        card.bounds.right - card.bounds.left, card.bounds.bottom - card.bounds.top);

    path.AddArc(rect.X, rect.Y, cornerRadius * 2, cornerRadius * 2, 180, 90);
    path.AddArc(rect.X + rect.Width - cornerRadius * 2, rect.Y, cornerRadius * 2, cornerRadius * 2, 270, 90);
    path.AddArc(rect.X + rect.Width - cornerRadius * 2, rect.Y + rect.Height - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 0, 90);
    path.AddArc(rect.X, rect.Y + rect.Height - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 90, 90);
    path.CloseFigure();

    SolidBrush bgBrush(bgColor);
    g->FillPath(&bgBrush, &path);

    if (card.hovered) {
        Pen borderPen(Color(255, 0, 120, 212), 2.5f);
        g->DrawPath(&borderPen, &path);
    }
    else {
        Pen borderPen(Color(255, 60, 60, 60), 1);
        g->DrawPath(&borderPen, &path);
    }

    Color statusColor;
    switch (card.status) {
    case CheckStatus::Pass:
        statusColor = Color(255, 16, 124, 16);
        break;
    case CheckStatus::Warning:
        statusColor = Color(255, 247, 99, 12);
        break;
    case CheckStatus::Fail:
        statusColor = Color(255, 232, 17, 35);
        break;
    default:
        statusColor = Color(255, 100, 100, 100);
    }

    SolidBrush statusBrush(statusColor);
    GraphicsPath topBarPath;
    topBarPath.AddArc(rect.X, rect.Y, cornerRadius * 2, cornerRadius * 2, 180, 90);
    topBarPath.AddLine(rect.X + cornerRadius, rect.Y, rect.X + rect.Width - cornerRadius, rect.Y);
    topBarPath.AddArc(rect.X + rect.Width - cornerRadius * 2, rect.Y, cornerRadius * 2, cornerRadius * 2, 270, 90);
    topBarPath.AddLine(rect.X + rect.Width, rect.Y + 4, rect.X, rect.Y + 4);
    topBarPath.CloseFigure();
    g->FillPath(&statusBrush, &topBarPath);

    Font nameFont(L"Segoe UI", 12, FontStyleBold);
    SolidBrush textBrush(Color(255, 255, 255, 255));
    std::wstring wname(card.name.begin(), card.name.end());

    RectF textRect(static_cast<REAL>(card.bounds.left + 12), static_cast<REAL>(card.bounds.top + 15),
        static_cast<REAL>(card.bounds.right - card.bounds.left - 24), 50.0f);
    g->DrawString(wname.c_str(), -1, &nameFont, textRect, nullptr, &textBrush);

    int circleSize = 48;
    int circleX = card.bounds.left + (card.bounds.right - card.bounds.left) / 2 - circleSize / 2;
    int circleY = card.bounds.top + 58;

    SolidBrush circleBgBrush(Color(255, 50, 50, 50));
    Rect circleRect(circleX, circleY, circleSize, circleSize);
    g->FillEllipse(&circleBgBrush, circleRect);

    Pen scorePen(statusColor, 4.5f);
    scorePen.SetStartCap(LineCapRound);
    scorePen.SetEndCap(LineCapRound);
    int sweepAngle = static_cast<int>((card.score / 100.0f) * 360);
    Rect arcRect(circleX + 2, circleY + 2, circleSize - 4, circleSize - 4);
    g->DrawArc(&scorePen, arcRect, -90.0f, static_cast<REAL>(sweepAngle));

    Font scoreFont(L"Segoe UI", 15, FontStyleBold);
    std::wstring scoreText = std::to_wstring(card.score);
    RectF scoreRect(static_cast<REAL>(circleX), static_cast<REAL>(circleY),
        static_cast<REAL>(circleSize), static_cast<REAL>(circleSize));
    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);
    g->DrawString(scoreText.c_str(), -1, &scoreFont, scoreRect, &format, &textBrush);
}

void MainForm::DrawComplianceGauge(Graphics* g, int x, int y, int size, int score) {
    SolidBrush bgBrush(Color(255, 50, 50, 50));
    Rect bgRect(x, y, size, size);
    g->FillEllipse(&bgBrush, bgRect);

    Pen outerPen(Color(255, 60, 60, 60), 16);
    Rect outerRect(x + 8, y + 8, size - 16, size - 16);
    g->DrawEllipse(&outerPen, outerRect);

    Color scoreColor;
    if (score >= 90) scoreColor = Color(255, 16, 124, 16);
    else if (score >= 70) scoreColor = Color(255, 247, 99, 12);
    else scoreColor = Color(255, 232, 17, 35);

    Pen scorePen(scoreColor, 16);
    scorePen.SetStartCap(LineCapRound);
    scorePen.SetEndCap(LineCapRound);

    int sweepAngle = static_cast<int>((score / 100.0f) * 360);
    g->DrawArc(&scorePen, outerRect, -90.0f, static_cast<REAL>(sweepAngle));

    std::wstring text = std::to_wstring(score) + L"%";
    Font font(L"Segoe UI", 48, FontStyleBold);
    SolidBrush textBrush(Color(255, 255, 255, 255));

    RectF rect(static_cast<REAL>(x), static_cast<REAL>(y),
        static_cast<REAL>(size), static_cast<REAL>(size));
    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);

    g->DrawString(text.c_str(), -1, &font, rect, &format, &textBrush);
}

void MainForm::DrawLineGraph(Graphics* g, int x, int y, int width, int height, const std::vector<int>& data) {
    if (data.empty()) return;

    Rect bgRect(x, y, width, height);
    SolidBrush bgBrush(Color(255, 30, 30, 30));
    g->FillRectangle(&bgBrush, bgRect);

    Pen borderPen(Color(255, 60, 60, 60), 1);
    g->DrawRectangle(&borderPen, bgRect);

    Font titleFont(L"Segoe UI", 11, FontStyleBold);
    SolidBrush textBrush(Color(255, 180, 180, 180));
    g->DrawString(L"Compliance History", -1, &titleFont,
        PointF(static_cast<REAL>(x + 10), static_cast<REAL>(y + 5)), &textBrush);

    if (data.size() < 2) return;

    int graphPadding = 20;
    int graphWidth = width - 2 * graphPadding;
    int graphHeight = height - 2 * graphPadding - 15;
    int graphX = x + graphPadding;
    int graphY = y + graphPadding + 15;

    int pointCount = static_cast<int>(data.size());
    float xStep = static_cast<float>(graphWidth) / (pointCount - 1);

    std::vector<PointF> points;
    for (size_t i = 0; i < data.size(); i++) {
        float px = static_cast<float>(graphX) + static_cast<float>(i) * xStep;
        float py = static_cast<float>(graphY + graphHeight) - (data[i] / 100.0f) * static_cast<float>(graphHeight);
        points.push_back(PointF(px, py));
    }

    Pen linePen(Color(255, 0, 120, 212), 2.5f);
    linePen.SetStartCap(LineCapRound);
    linePen.SetEndCap(LineCapRound);

    if (points.size() >= 2) {
        g->DrawLines(&linePen, &points[0], static_cast<INT>(points.size()));
    }

    SolidBrush pointBrush(Color(255, 0, 150, 255));
    for (const auto& point : points) {
        RectF pointRect(point.X - 3, point.Y - 3, 6, 6);
        g->FillEllipse(&pointBrush, pointRect);
    }
}

void MainForm::DrawPieChart(Graphics* g, int x, int y, int radius, const std::vector<std::pair<std::string, int>>& data) {
    if (data.empty()) return;

    int total = 0;
    for (const auto& item : data) {
        total += item.second;
    }
    if (total == 0) return;

    float startAngle = -90.0f;
    Color colors[] = {
        Color(255, 0, 120, 212),
        Color(255, 16, 124, 16),
        Color(255, 247, 99, 12),
        Color(255, 232, 17, 35),
        Color(255, 153, 51, 204)
    };

    Rect pieRect(x - radius, y - radius, radius * 2, radius * 2);

    for (size_t i = 0; i < data.size(); i++) {
        float sweepAngle = (data[i].second / static_cast<float>(total)) * 360.0f;
        SolidBrush brush(colors[i % 5]);
        g->FillPie(&brush, pieRect, startAngle, sweepAngle);
        startAngle += sweepAngle;
    }
}

void MainForm::DrawBarChart(Graphics* g, int x, int y, int width, int height, const std::vector<std::pair<std::string, int>>& data) {
    if (data.empty()) return;

    int maxValue = 0;
    for (const auto& item : data) {
        if (item.second > maxValue) maxValue = item.second;
    }
    if (maxValue == 0) maxValue = 100;

    int barWidth = width / static_cast<int>(data.size()) - 10;
    int currentX = x;

    SolidBrush barBrush(Color(255, 0, 120, 212));
    Font labelFont(L"Segoe UI", 10);
    SolidBrush textBrush(Color(255, 255, 255, 255));

    for (const auto& item : data) {
        int barHeight = static_cast<int>((item.second / static_cast<float>(maxValue)) * height);
        Rect barRect(currentX, y + height - barHeight, barWidth, barHeight);
        g->FillRectangle(&barBrush, barRect);

        std::wstring label(item.first.begin(), item.first.end());
        g->DrawString(label.c_str(), -1, &labelFont,
            PointF(static_cast<REAL>(currentX), static_cast<REAL>(y + height + 5)), &textBrush);

        currentX += barWidth + 10;
    }
}

void MainForm::DrawGradientBackground(Graphics* g, const Rect& rect, Color color1, Color color2) {
    LinearGradientBrush gradBrush(
        Point(rect.X, rect.Y),
        Point(rect.X, rect.Y + rect.Height),
        color1,
        color2
    );
    g->FillRectangle(&gradBrush, rect);
}

void MainForm::DrawGlassEffect(Graphics* g, const Rect& rect) {
    LinearGradientBrush glassBrush(
        Point(rect.X, rect.Y),
        Point(rect.X, rect.Y + rect.Height / 2),
        Color(80, 255, 255, 255),
        Color(10, 255, 255, 255)
    );
    g->FillRectangle(&glassBrush, rect.X, rect.Y, rect.Width, rect.Height / 2);
}

void MainForm::DrawModuleCards(Graphics* g) {
    // This function is integrated into DrawDashboard
}

void MainForm::DrawSystemOverview(Graphics* g) {
    Font titleFont(L"Segoe UI", 26, FontStyleBold);
    Font noteFont(L"Segoe UI", 15, FontStyleItalic);
    SolidBrush textBrush(Color(255, 255, 255, 255));
    SolidBrush noteBrush(Color(255, 180, 180, 180));

    g->DrawString(L"System Overview", -1, &titleFont, PointF(40, 140), &textBrush);
    g->DrawString(L" Note: Advanced system diagnostics and hardware monitoring",
        -1, &noteFont, PointF(40, 185), &noteBrush);
    g->DrawString(L"will be available in the next version of this tool.",
        -1, &noteFont, PointF(40, 210), &noteBrush);

    Rect cardRect(40, 250, 600, 380);
    SolidBrush cardBrush(Color(255, 30, 30, 30));
    g->FillRectangle(&cardBrush, cardRect);

    Pen borderPen(Color(255, 60, 60, 60), 2);
    g->DrawRectangle(&borderPen, cardRect);

    Font placeholderFont(L"Segoe UI", 18);
    SolidBrush placeholderBrush(Color(255, 120, 120, 120));
    g->DrawString(L" System Performance Metrics\n\n"
        L"• CPU Usage Monitoring\n"
        L"• Memory Analytics\n"
        L"• Disk Health Status\n"
        L"• Network Bandwidth\n"
        L"• Process Explorer\n\n"
        L"Coming Soon!", -1, &placeholderFont,
        PointF(80, 290), &placeholderBrush);
}

void MainForm::DrawSecurityPolicies(Graphics* g) {
    Font titleFont(L"Segoe UI", 26, FontStyleBold);
    Font noteFont(L"Segoe UI", 15, FontStyleItalic);
    SolidBrush textBrush(Color(255, 255, 255, 255));
    SolidBrush noteBrush(Color(255, 180, 180, 180));

    g->DrawString(L"Security Policies", -1, &titleFont, PointF(40, 140), &textBrush);
    g->DrawString(L" Note: Detailed policy editor and compliance framework",
        -1, &noteFont, PointF(40, 185), &noteBrush);
    g->DrawString(L"configuration tools are planned for future releases.",
        -1, &noteFont, PointF(40, 210), &noteBrush);

    const char* policies[] = {
        "Password Complexity Rules",
        "Account Lockout Policies",
        "Audit Policy Configuration",
        "User Rights Assignment",
        "Security Options",
        "Registry Permissions"
    };

    int cardY = 260;
    for (int i = 0; i < 6; i++) {
        Rect cardRect(40, cardY, 700, 58);
        SolidBrush cardBrush(Color(255, 30, 30, 30));
        g->FillRectangle(&cardBrush, cardRect);

        Pen borderPen(Color(255, 0, 120, 212), 1.5f);
        g->DrawRectangle(&borderPen, cardRect);

        Font policyFont(L"Segoe UI", 15);
        std::wstring policyText(policies[i], policies[i] + strlen(policies[i]));
        g->DrawString((L" " + policyText).c_str(), -1, &policyFont,
            PointF(60, static_cast<REAL>(cardY + 17)), &textBrush);

        cardY += 72;
    }
}

void MainForm::DrawNetworkFirewall(Graphics* g) {
    Font titleFont(L"Segoe UI", 26, FontStyleBold);
    Font noteFont(L"Segoe UI", 15, FontStyleItalic);
    SolidBrush textBrush(Color(255, 255, 255, 255));
    SolidBrush noteBrush(Color(255, 180, 180, 180));

    g->DrawString(L"Network & Firewall", -1, &titleFont, PointF(40, 140), &textBrush);
    g->DrawString(L" Note: Real-time network monitoring, firewall rule management,",
        -1, &noteFont, PointF(40, 185), &noteBrush);
    g->DrawString(L"and traffic analysis features are under development.",
        -1, &noteFont, PointF(40, 210), &noteBrush);

    Rect vizRect(40, 260, 800, 380);
    SolidBrush cardBrush(Color(255, 30, 30, 30));
    g->FillRectangle(&cardBrush, vizRect);

    Pen borderPen(Color(255, 60, 60, 60), 2);
    g->DrawRectangle(&borderPen, vizRect);

    Font placeholderFont(L"Segoe UI", 17);
    SolidBrush placeholderBrush(Color(255, 120, 120, 120));
    g->DrawString(L" Network Topology Visualization\n\n"
        L" Bandwidth Usage Graphs\n\n"
        L" Firewall Rules Management\n\n"
        L" Intrusion Detection Alerts\n\n"
        L" Port Scanning Detection",
        -1, &placeholderFont, PointF(80, 310), &placeholderBrush);
}

void MainForm::DrawAntivirusUpdates(Graphics* g) {
    Font titleFont(L"Segoe UI", 26, FontStyleBold);
    Font noteFont(L"Segoe UI", 15, FontStyleItalic);
    SolidBrush textBrush(Color(255, 255, 255, 255));
    SolidBrush noteBrush(Color(255, 180, 180, 180));

    g->DrawString(L"Antivirus & Updates", -1, &titleFont, PointF(40, 140), &textBrush);
    g->DrawString(L" Note: Automatic update scheduling and antivirus status tracking",
        -1, &noteFont, PointF(40, 185), &noteBrush);
    g->DrawString(L"with real-time threat detection coming in future updates.",
        -1, &noteFont, PointF(40, 210), &noteBrush);
}

void MainForm::DrawReports(Graphics* g) {
    Font titleFont(L"Segoe UI", 26, FontStyleBold);
    SolidBrush textBrush(Color(255, 255, 255, 255));

    g->DrawString(L"Compliance Reports", -1, &titleFont, PointF(40, 140), &textBrush);

    Font noteFont(L"Segoe UI", 15);
    SolidBrush noteBrush(Color(255, 180, 180, 180));
    g->DrawString(L"Generate detailed compliance reports in multiple formats",
        -1, &noteFont, PointF(40, 185), &noteBrush);
}

void MainForm::OnSize() {
    RECT rect;
    GetClientRect(m_hwnd, &rect);

    if (m_tabControl) {
        SetWindowPos(m_tabControl, nullptr, 20, 20, rect.right - 40, 35, SWP_NOZORDER);
    }

    if (m_scanButton) {
        SetWindowPos(m_scanButton, nullptr, 20, 75, 180, 42, SWP_NOZORDER);
    }
    if (m_exportTextButton) {
        SetWindowPos(m_exportTextButton, nullptr, 20, 75, 150, 42, SWP_NOZORDER);
    }
    if (m_exportJSONButton) {
        SetWindowPos(m_exportJSONButton, nullptr, 190, 75, 150, 42, SWP_NOZORDER);
    }

    m_canvasRect = { 0, 130, rect.right, rect.bottom };
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void MainForm::OnMouseMove(WPARAM wParam, LPARAM lParam) {
    POINT pt = { LOWORD(lParam), HIWORD(lParam) };

    int oldHovered = m_hoveredCard;
    m_hoveredCard = -1;

    for (size_t i = 0; i < m_moduleCards.size(); i++) {
        if (PtInRect(&m_moduleCards[i].bounds, pt)) {
            m_hoveredCard = static_cast<int>(i);
            break;
        }
    }

    if (oldHovered != m_hoveredCard) {
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void MainForm::OnLButtonDown(WPARAM wParam, LPARAM lParam) {
    if (m_hoveredCard >= 0 && m_hoveredCard < static_cast<int>(m_moduleCards.size())) {
        const auto& card = m_moduleCards[m_hoveredCard];
        std::wstring wname(card.name.begin(), card.name.end());
        std::wstring msg = L"Module: " + wname + L"\nScore: " + std::to_wstring(card.score) + L"%";
        MessageBoxW(m_hwnd, msg.c_str(), L"Module Details", MB_OK | MB_ICONINFORMATION);
    }
}

void MainForm::OnTimer(UINT_PTR timerID) {
    if (timerID == TIMER_ANIMATION) {
        UpdateAnimation();
    }
}

void MainForm::StartAnimation() {
    m_animating = true;
    m_animProgress = 0.0f;
    m_animTimer = SetTimer(m_hwnd, TIMER_ANIMATION, 16, nullptr);
}

void MainForm::StopAnimation() {
    if (m_animTimer) {
        KillTimer(m_hwnd, m_animTimer);
        m_animTimer = 0;
    }
    m_animating = false;
}

void MainForm::UpdateAnimation() {
    m_animProgress += 0.02f;
    if (m_animProgress >= 1.0f) {
        m_animProgress = 1.0f;
        StopAnimation();
    }
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void MainForm::PerformScan() {
    if (m_scanInProgress) return;

    m_scanInProgress = true;
    EnableScanControls(false);
    StartAnimation();

    m_engine.performFullScan();

    int score = m_engine.getOverallComplianceScore();
    m_scoreHistory.push_back(score);
    if (m_scoreHistory.size() > 10) {
        m_scoreHistory.erase(m_scoreHistory.begin());
    }

    UpdateDashboard();

    m_scanInProgress = false;
    EnableScanControls(true);

    MessageBoxA(m_hwnd, "Compliance scan completed successfully!", "Scan Complete", MB_OK | MB_ICONINFORMATION);
}

void MainForm::UpdateDashboard() {
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void MainForm::UpdateResultsDisplay() {
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void MainForm::EnableScanControls(bool enable) {
    EnableWindow(m_scanButton, enable);
    EnableWindow(m_exportTextButton, enable);
    EnableWindow(m_exportJSONButton, enable);
}

std::vector<std::string> MainForm::GetSelectedModules() {
    return m_engine.getAvailableModules();
}

void MainForm::OnDestroy() {
    StopAnimation();
    PostQuitMessage(0);
}