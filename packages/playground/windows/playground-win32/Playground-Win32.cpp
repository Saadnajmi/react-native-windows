#include "pch.h"
#include "resource.h"

#include <windows.h>

struct WindowData {
  static HINSTANCE s_instance;

  static WindowData *GetFromWindow(HWND hwnd) {
    return reinterpret_cast<WindowData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  LRESULT OnDestroy(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    PostQuitMessage(0);
    return 0;
  }

  LRESULT OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT) {
    switch (id) {
      case IDM_ABOUT:
        DialogBox(s_instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, &About);
        break;
      case IDM_EXIT:
        PostQuitMessage(0);
        break;
    }

    return 0;
  }

  /// Message handler for about box.
  static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
      case WM_INITDIALOG:
        return TRUE;

      case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
          EndDialog(hDlg, LOWORD(wParam));
          return TRUE;
        }
        break;
    }

    return FALSE;
  }
};

extern "C" IMAGE_DOS_HEADER __ImageBase;
HINSTANCE WindowData::s_instance = reinterpret_cast<HINSTANCE>(&__ImageBase);

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) noexcept {
  switch (message) {
    case WM_COMMAND: {
      return WindowData::GetFromWindow(hwnd)->OnCommand(
          hwnd, LOWORD(wparam), reinterpret_cast<HWND>(lparam), HIWORD(wparam));
    }
    case WM_DESTROY: {
      return WindowData::GetFromWindow(hwnd)->OnDestroy(hwnd, wparam, lparam);
    }
    case WM_NCCREATE: {
      auto cs = reinterpret_cast<CREATESTRUCT *>(lparam);
      auto windowData = static_cast<WindowData *>(cs->lpCreateParams);
      WINRT_ASSERT(windowData);
      SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowData));
      break;
    }
  }

  return DefWindowProc(hwnd, message, wparam, lparam);
}

_Use_decl_annotations_ int CALLBACK WinMain(HINSTANCE instance, HINSTANCE, PSTR commandLine, int showCmd) {
  constexpr PCWSTR appName = L"ReactNative Playground (Win32)";
  constexpr PCWSTR windowClassName = L"MS_REACTNATIVE_PLAYGROUND_WIN32";

  WNDCLASSEXW wcex = {};
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = &WndProc;
  wcex.cbClsExtra = DLGWINDOWEXTRA;
  wcex.cbWndExtra = 0;
  wcex.hInstance = instance;
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PLAYGROUND_WIN32);
  wcex.lpszClassName = windowClassName;
  ATOM classId = RegisterClassEx(&wcex);
  WINRT_VERIFY(classId);
  winrt::check_win32(!classId);

  auto windowData = std::make_unique<WindowData>();

  HWND hwnd = CreateWindow(
      windowClassName,
      appName,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      800,
      600,
      nullptr,
      nullptr,
      instance,
      windowData.get());

  WINRT_VERIFY(hwnd);
  winrt::check_win32(!hwnd);

  ShowWindow(hwnd, showCmd);
  UpdateWindow(hwnd);
  SetFocus(hwnd);

  HACCEL hAccelTable = LoadAccelerators(instance, MAKEINTRESOURCE(IDC_PLAYGROUND_WIN32));

  MSG msg = {};
  while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return (int)msg.wParam;
}
