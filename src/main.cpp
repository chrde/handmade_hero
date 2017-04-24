#include <windows.h>

LRESULT CALLBACK MainWindowCallback(HWND window, UINT message, WPARAM wParam,
                                    LPARAM lParam) {
  LRESULT result = 0;
  switch (message) {
    case WM_SIZE: {
      OutputDebugString("WM_SIZE");
      break;
    }
    case WM_DESTROY: {
      OutputDebugString("WM_DESTROY");
      break;
    }
    case WM_CLOSE: {
      OutputDebugString("WM_CLOSE");
      break;
    }
    case WM_PAINT: {
      PAINTSTRUCT paint;
      HDC deviceContext = BeginPaint(window, &paint);
      RECT rect = paint.rcPaint;
      int x = rect.left;
      int y = rect.top;
      int height = rect.bottom - rect.top;
      int width = rect.right - rect.left;
      PatBlt(deviceContext, x, y, width, height, WHITENESS);
      EndPaint(window, &paint);
      break;
    }
    case WM_ACTIVATEAPP: {
      OutputDebugString("WM_ACTIVATEAPP");
      break;
    }
    default: {
      result = DefWindowProc(window, message, wParam, lParam);
      break;
    }
  }
  return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance,
                     LPSTR commandLine, int showCode) {
  // MessageBox(0, "Hello", "Title", MB_OK | MB_ICONINFORMATION);
  WNDCLASS WindowClass = {};
  WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = instance;
  WindowClass.lpszClassName = "MyWindowClass";
  if (RegisterClass(&WindowClass)) {
    HWND windowHandle = CreateWindowEx(
        0, WindowClass.lpszClassName, "Super stuff",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);
    if (windowHandle) {
      MSG message;
      while (1) {
        BOOL messageResult = GetMessage(&message, 0, 0, 0);
        if (messageResult > 0) {
          TranslateMessage(&message);
          DispatchMessage(&message);
        } else {
          break;
        }
      }
    } else {
      // TODO logging
    }
  } else {
    // TODO logging
  }
  return (0);
}
