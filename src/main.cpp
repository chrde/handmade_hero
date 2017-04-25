#include <stdint.h>
#include <windows.h>
#define internal static
#define local_persist static
#define global_variable static

// TODO change this
global_variable bool running;
global_variable BITMAPINFO bitmapInfo;
global_variable void *bitmapMemory;
global_variable int bitmapWidth;
global_variable int bitmapHeight;

internal void Win32ResizeDIBSection(int width, int height) {
  if (bitmapMemory) {
    VirtualFree(bitmapMemory, 0, MEM_RELEASE);
  }

  bitmapWidth = width;
  bitmapHeight = height;

  BITMAPINFOHEADER *header = &bitmapInfo.bmiHeader;
  header->biSize = sizeof(*header);
  header->biWidth = bitmapWidth;
  header->biHeight = -bitmapHeight;
  header->biPlanes = 1;
  header->biBitCount = 32;
  header->biCompression = BI_RGB;

  int bytesPerPixel = header->biBitCount / 8;
  int bitmapMemorySize = (width * height) * bytesPerPixel;
  bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  int pitch = width * bytesPerPixel;
  uint8_t *row = (uint8_t *)bitmapMemory;
  for (int y = 0; y < bitmapHeight; ++y) {
    uint32_t *pixel = (uint32_t *)row;
    for (int x = 0; x < bitmapWidth; ++x) {
      *pixel = (uint8_t)x + ((uint8_t)y << 8);
      ++pixel;
    }
    row += pitch;
  }
}

internal void Win32UpdateWindow(HDC deviceContext, RECT *windowRect, int x,
                                int y, int width, int height) {
  int windowWidth = windowRect->right - windowRect->left;
  int windowHeight = windowRect->bottom - windowRect->top;
  StretchDIBits(deviceContext, 0, 0, bitmapWidth, bitmapHeight, 0, 0,
                windowWidth, windowHeight, bitmapMemory, &bitmapInfo,
                DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message,
                                         WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;
  switch (message) {
    case WM_SIZE: {
      RECT c;
      GetClientRect(window, &c);
      int width = c.right - c.left;
      int height = c.bottom - c.top;
      Win32ResizeDIBSection(width, height);
      break;
    }
    case WM_DESTROY: {
      // TODO try to reopen window
      running = false;
      break;
    }
    case WM_CLOSE: {
      running = false;
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
      RECT c;
      GetClientRect(window, &c);
      Win32UpdateWindow(deviceContext, &c, x, y, width, height);
      EndPaint(window, &paint);
      break;
    }
    case WM_ACTIVATEAPP: {
      OutputDebugString("WM_ACTIVATEAPP\n");
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
  WindowClass.lpfnWndProc = Win32MainWindowCallback;
  WindowClass.hInstance = instance;
  WindowClass.lpszClassName = "MyWindowClass";
  if (RegisterClass(&WindowClass)) {
    HWND windowHandle = CreateWindowEx(
        0, WindowClass.lpszClassName, "Super stuff",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);
    if (windowHandle) {
      running = true;
      MSG message;
      while (running) {
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
