#include <filesystem>
#include <format>
#include <set>
using namespace std::literals;
namespace fs = std::filesystem;
fs::path datadir = fs::current_path();

#include <math.h>

#include <windows.h>
#include <CommCtrl.h>
#include <objbase.h>

void err(const std::wstring &s) {
    MessageBox(0, s.c_str(), TEXT("ERROR"), MB_OK);
    exit(1);
}
struct cfg {
    fs::path cfgfn;
    const wchar_t *sectionstr = L"PATH";
    const wchar_t *keystr = L"TextFile";

    cfg() {
        if (fs::exists(datadir / "data")) {
            datadir /= "data";
        } else if (datadir.filename() == "data") {
        } else {
            err(L"Can't find data directory!");
        }
        cfgfn = datadir / "config" / "cfg.ini";
    }
    std::wstring get_current_lang() {
        std::wstring fn(256,0);
        GetPrivateProfileStringW(sectionstr, keystr, 0, fn.data(), fn.size(), cfgfn.wstring().c_str());
        if (GetLastError() != 0) {
            err(L"can't read file");
        }
        fn = fn.c_str();
        auto w = fs::path{fn}.stem().wstring();
        std::transform(w.begin(), w.end(), w.begin(), ::towlower);
        auto tofind = L"quest_"s;
        if (w.starts_with(tofind)) {
            return w.substr(tofind.size());
        }
        return L"no language set (your default language)";
    }
    bool set_lang(auto &&lang) {
        if (lang.contains(L' ') || lang.contains(L" ")) {
            return false;
        }
        auto fn = std::format(L"Data\\Quest_{}.dat", lang);
        if (!WritePrivateProfileStringW(sectionstr, keystr, fn.c_str(), cfgfn.wstring().c_str())) {
            return false;
        }
        return true;
    }
} c;

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE) & __ImageBase)
#endif

struct DemoApp {
    HRESULT Initialize() {
        WNDCLASSEX wcex = {sizeof(WNDCLASSEX)};
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = DemoApp::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(LONG_PTR);
        wcex.hInstance = HINST_THISCOMPONENT;
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = NULL;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.lpszClassName = TEXT("DemoApp");
        RegisterClassEx(&wcex);

        // Create the application window.
        int dpiX = 0;
        int dpiY = 0;
        int horpix{}, vertpix{};
        HDC hdc = GetDC(NULL);
        if (hdc) {
            dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
            dpiY = GetDeviceCaps(hdc, LOGPIXELSY);

            horpix = GetDeviceCaps(hdc, HORZRES);
            vertpix = GetDeviceCaps(hdc, VERTRES);
            ReleaseDC(NULL, hdc);
        }

        int width = 400;
        int height = 100;
        m_hwnd = CreateWindow(TEXT("DemoApp"), TEXT("AIM1 Language Switcher"), WS_OVERLAPPEDWINDOW
            , (horpix - width) / 2
            , (vertpix - height) / 2
            , static_cast<UINT>(ceil(width * dpiX / 96.f))
            , static_cast<UINT>(ceil(height * dpiY / 96.f))
            , NULL, NULL, HINST_THISCOMPONENT, this);

        auto hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr)) {
            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            UpdateWindow(m_hwnd);
        }

        // Create the Combobox
        int nwidth = 300;         // Width of the window
        int nheight = 200;        // Height of the window
        int xpos = (width - nwidth) / 2;             // Horizontal position of the window.
        int ypos = 15;//(height - nheight) / 2;             // Vertical position of the window.
        HWND hwndParent = m_hwnd; // Handle to the parent window

        HWND hWndComboBox =
            CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE
                | CBS_AUTOHSCROLL | WS_HSCROLL | WS_VSCROLL,
                         xpos, ypos, nwidth, nheight, hwndParent, NULL, HINST_THISCOMPONENT, NULL);

        std::set<std::wstring> langs;
        for (auto &&fn : fs::directory_iterator(datadir)) {
            if (!fs::is_regular_file(fn)) {
                continue;
            }
            auto q = fn.path().stem().wstring();
            std::transform(q.begin(), q.end(), q.begin(), ::towlower);
            auto tofind = L"quest_"s;
            if (!q.starts_with(tofind)) {
                continue;
            }
            auto lang = q.substr(tofind.size());
            langs.insert(lang);
        }
        auto [it,_] = langs.insert(c.get_current_lang());
        auto dist = std::distance(langs.begin(), it);
        for (auto &&l : langs) {
            SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)l.c_str());
        }
        // Send the CB_SETCURSEL message to display an initial item in the selection field
        SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)dist, (LPARAM)0);
        return hr;
    }
    void RunMessageLoop() {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        LRESULT result = 0;
        if (message == WM_CREATE) {
            auto pcs = (LPCREATESTRUCT)lParam;
            auto pDemoApp = (DemoApp *)pcs->lpCreateParams;
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, PtrToUlong(pDemoApp));
            result = 1;
        } else {
            auto pDemoApp = reinterpret_cast<DemoApp *>(static_cast<LONG_PTR>(::GetWindowLongPtr(hwnd, GWLP_USERDATA)));
            bool wasHandled = false;
            if (pDemoApp) {
                switch (message) {
                case WM_COMMAND:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        // If the user makes a selection from the list:
                        //   Send CB_GETCURSEL message to get the index of the selected list item.
                        //   Send CB_GETLBTEXT message to get the item.
                        //   Display the item in a messagebox.
                        int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                        TCHAR ListItem[256]{};
                        SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)ListItem);
                        std::wstring s = ListItem;
                        if (c.set_lang(s)) {
                            std::wstring msg = L"Language changed to " + s;
                            MessageBox(hwnd, msg.c_str(), TEXT("Language Changed"), MB_OK);
                        }
                    }
                    wasHandled = true;
                    result = 0;
                    break;
                case WM_DISPLAYCHANGE:
                    InvalidateRect(hwnd, NULL, FALSE);
                    wasHandled = true;
                    result = 0;
                    break;
                case WM_DESTROY:
                    PostQuitMessage(0);
                    wasHandled = true;
                    result = 1;
                    break;
                }
            }
            if (!wasHandled) {
                result = DefWindowProc(hwnd, message, wParam, lParam);
            }
        }
        return result;
    }

private:
    HWND m_hwnd{};
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    if (SUCCEEDED(CoInitialize(NULL))) {
        {
            DemoApp app;
            if (SUCCEEDED(app.Initialize())) {
                app.RunMessageLoop();
            }
        }
        CoUninitialize();
    }
    return 0;
}
