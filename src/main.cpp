#include <stdint.h>
#include <windows.h>
#define internal static
#define local_persist static
#define global_variable static

struct win32_offscreen_buffer {
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int pitch;
  int bytesPerPixel;
};

// TODO change this
global_variable bool running;
global_variable win32_offscreen_buffer backBuffer;

internal void renderSomething(win32_offscreen_buffer buffer, int xOffset,
                              int yOffset) {
  uint8_t *row = (uint8_t *)buffer.memory;
  for (int y = 0; y < buffer.height; ++y) {
    uint32_t *pixel = (uint32_t *)row;
    for (int x = 0; x < buffer.width; ++x) {
      uint8_t green = y + yOffset;
      uint8_t blue = x + xOffset;
      *pixel = ((green << 8) | blue);
      ++pixel;
    }
    row += buffer.pitch;
  }
}

struct win32_window_dimension {
  int width;
  int height;
};

internal win32_window_dimension Win32GetWindowDimension(HWND window) {
  win32_window_dimension dimension;
  RECT rectangle;
  GetClientRect(window, &rectangle);
  dimension.width = rectangle.right - rectangle.left;
  dimension.height = rectangle.bottom - rectangle.top;
  return dimension;
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width,
                                    int height) {
  if (buffer->memory) {
    VirtualFree(buffer->memory, 0, MEM_RELEASE);
  }

  BITMAPINFOHEADER *header = &buffer->info.bmiHeader;
  // NOTE negative height means top-down bitmap e.g: the first three bytes of
  // the image form the color of the top left pixel
  header->biHeight = -height;
  header->biSize = sizeof(*header);
  header->biWidth = width;
  header->biPlanes = 1;
  header->biBitCount = 32;
  header->biCompression = BI_RGB;

  buffer->width = width;
  buffer->height = height;
  buffer->bytesPerPixel = header->biBitCount / 8;
  buffer->pitch = buffer->width * buffer->bytesPerPixel;
  int memorySize = (width * height) * buffer->bytesPerPixel;
  buffer->memory = VirtualAlloc(0, memorySize, MEM_COMMIT, PAGE_READWRITE);

  // TODO clear to black
}

internal void Win32DisplayBufferInWindow(HDC deviceContext, int windowWidth,
                                         int windowHeight,
                                         win32_offscreen_buffer buffer, int x,
                                         int y, int width, int height) {
  // TODO aspect ratio correction
  StretchDIBits(deviceContext, 0, 0, windowWidth, windowHeight, 0, 0,
                buffer.width, buffer.height, buffer.memory, &buffer.info,
                DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message,
                                         WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;
  switch (message) {
    case WM_SIZE: {
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
      win32_window_dimension dimension = Win32GetWindowDimension(window);
      Win32DisplayBufferInWindow(deviceContext, dimension.width,
                                 dimension.height, backBuffer, x, y, width,
                                 height);
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
  uint8_t bla[2 * 1024 * 1024] = {};
  WNDCLASS WindowClass = {};
  Win32ResizeDIBSection(&backBuffer, 1280, 720);
  WindowClass.style = CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = Win32MainWindowCallback;
  WindowClass.hInstance = instance;
  WindowClass.lpszClassName = "MyWindowClass";
  if (RegisterClass(&WindowClass)) {
    HWND window = CreateWindowEx(0, WindowClass.lpszClassName, "Super stuff",
                                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, 0, 0, instance, 0);
    if (window) {
      running = true;
      MSG message;
      int xOffset = 0;
      int yOffset = 0;
      while (running) {
        while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
          if (message.message == WM_QUIT) {
            running = false;
          }
          TranslateMessage(&message);
          DispatchMessage(&message);
        }
        renderSomething(backBuffer, xOffset, yOffset);
        HDC deviceContext = GetDC(window);
        win32_window_dimension dimension = Win32GetWindowDimension(window);
        Win32DisplayBufferInWindow(deviceContext, dimension.width,
                                   dimension.height, backBuffer, 0, 0,
                                   dimension.width, dimension.height);
        ReleaseDC(window, deviceContext);
        ++xOffset;
        ++yOffset;
      }
    } else {
      // TODO logging
    }
  } else {
    // TODO logging
  }
  return (0);
}
