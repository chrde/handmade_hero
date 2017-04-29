#include <dsound.h>
#include <stdint.h>
#include <windows.h>
#include <xinput.h>
#define internal static
#define local_persist static
#define global_variable static
typedef int32_t bool32;

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

internal void renderSomething(win32_offscreen_buffer *buffer, int xOffset, int yOffset) {
  uint8_t *row = (uint8_t *)buffer->memory;
  for (int y = 0; y < buffer->height; ++y) {
    uint32_t *pixel = (uint32_t *)row;
    for (int x = 0; x < buffer->width; ++x) {
      uint8_t green = y + yOffset;
      uint8_t blue = x + xOffset;
      *pixel = ((green << 8) | blue);
      ++pixel;
    }
    row += buffer->pitch;
  }
}

struct win32_window_dimension {
  int width;
  int height;
};

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) { return (ERROR_DEVICE_NOT_CONNECTED); }
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) { return (ERROR_DEVICE_NOT_CONNECTED); }
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void Win32LoadXInput(void) {
  HMODULE xInputLibrary = LoadLibraryA("xinput1_4.dll");
  if (!xInputLibrary) {
    xInputLibrary = LoadLibraryA("xinput1_3.dll");
  }
  if (xInputLibrary) {
    XInputGetState = (x_input_get_state *)GetProcAddress(xInputLibrary, "XInputGetState");
    XInputSetState = (x_input_set_state *)GetProcAddress(xInputLibrary, "XInputSetState");
  } else {
    // TODO diagnostic
  }
}

internal void Win32InitDSound(HWND window, int32_t samplesPerSecond, int32_t bufferSize) {
  HMODULE dSoundLibrary = LoadLibraryA("dsound.dll");
  if (dSoundLibrary) {
    direct_sound_create *directSoundCreate = (direct_sound_create *)GetProcAddress(dSoundLibrary, "DirectSoundCreate");
    LPDIRECTSOUND directSound;
    if (directSoundCreate && SUCCEEDED(directSoundCreate(0, &directSound, 0))) {
      WAVEFORMATEX waveFormat = {};
      waveFormat.wFormatTag = WAVE_FORMAT_PCM;
      waveFormat.nChannels = 2;
      waveFormat.nSamplesPerSec = samplesPerSecond;
      waveFormat.wBitsPerSample = 16;
      waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
      waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
      waveFormat.cbSize = 0;
      if (SUCCEEDED(directSound->SetCooperativeLevel(window, DSSCL_PRIORITY))) {
        DSBUFFERDESC bufferDescription = {};
        bufferDescription.dwSize = sizeof(bufferDescription);
        bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
        LPDIRECTSOUNDBUFFER primaryBuffer;
        if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0))) {
          if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) {
            OutputDebugStringA("Primary buffer format was set.\n");
          } else {
            // TODO diagnostic
          }
        }
      } else {
        // TODO diagnostic
      }
      DSBUFFERDESC bufferDescription = {};
      bufferDescription.dwSize = sizeof(bufferDescription);
      bufferDescription.dwFlags = 0;
      bufferDescription.dwBufferBytes = bufferSize;
      bufferDescription.lpwfxFormat = &waveFormat;
      LPDIRECTSOUNDBUFFER secondaryBuffer;
      if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &secondaryBuffer, 0))) {
        if (SUCCEEDED(secondaryBuffer->SetFormat(&waveFormat))) {
          OutputDebugStringA("Secondary buffer format was set.\n");
        } else {
          // TODO diagnostic
        }
      }
    } else {
      // TODO diagnostic
    }
  }
  // start playing sound
}

internal win32_window_dimension Win32GetWindowDimension(HWND window) {
  win32_window_dimension dimension;
  RECT rectangle;
  GetClientRect(window, &rectangle);
  dimension.width = rectangle.right - rectangle.left;
  dimension.height = rectangle.bottom - rectangle.top;
  return dimension;
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height) {
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

internal void Win32DisplayBufferInWindow(win32_offscreen_buffer *buffer, HDC deviceContext, int windowWidth,
                                         int windowHeight, int x, int y, int width, int height) {
  // TODO aspect ratio correction
  StretchDIBits(deviceContext, 0, 0, windowWidth, windowHeight, 0, 0, buffer->width, buffer->height, buffer->memory,
                &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;
  switch (message) {
    case WM_SIZE: {
      break;
    }
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP: {
      uint32_t vkCode = wParam;
      bool wasDown = ((lParam & (1 << 30)) != 0);
      bool isDown = ((lParam & (1 << 31)) == 0);
      if (isDown == wasDown) {
        break;
      }
      if (vkCode == VK_UP) {
        if (isDown) {
          OutputDebugString("UP is down\n");
        }
        if (wasDown) {
          OutputDebugString("UP was down\n");
        }
      } else if (vkCode == 'W') {
        OutputDebugString("W\n");
      } else if (vkCode == VK_ESCAPE) {
      } else if (vkCode == VK_SPACE) {
      }
      bool32 altKeyWasDown = (lParam & (1 << 29));
      if ((vkCode == VK_F4) && altKeyWasDown) {
        running = false;
      }
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
      Win32DisplayBufferInWindow(&backBuffer, deviceContext, dimension.width, dimension.height, x, y, width, height);
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

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode) {
  Win32LoadXInput();
  WNDCLASSA WindowClass = {};
  Win32ResizeDIBSection(&backBuffer, 1280, 720);
  WindowClass.style = CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = Win32MainWindowCallback;
  WindowClass.hInstance = instance;
  WindowClass.lpszClassName = "MyWindowClass";
  if (RegisterClass(&WindowClass)) {
    HWND window = CreateWindowEx(0, WindowClass.lpszClassName, "Super stuff", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);
    if (window) {
      Win32InitDSound(window, 48000, 48000 * sizeof(int16_t) * 2);
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
        for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++) {
          XINPUT_STATE state;
          // ZeroMemory(&state, sizeof(XINPUT_STATE));

          if (XInputGetState(controllerIndex, &state) == ERROR_SUCCESS) {
            // TODO see if state.dwPacketNumber is too big
            XINPUT_GAMEPAD *pad = &state.Gamepad;

            bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
            bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool A = (pad->wButtons & XINPUT_GAMEPAD_A);
            bool B = (pad->wButtons & XINPUT_GAMEPAD_B);
            bool X = (pad->wButtons & XINPUT_GAMEPAD_X);
            bool Y = (pad->wButtons & XINPUT_GAMEPAD_Y);
            int16_t stickX = pad->sThumbLX;
            int16_t stickY = pad->sThumbLY;

            if (A) {
              yOffset += 2;
            }
            if (B) {
              xOffset += 2;
            }

          } else {
            // Controller is not connected
          }
        }
        XINPUT_VIBRATION vibration;
        vibration.wLeftMotorSpeed = 60000;
        vibration.wRightMotorSpeed = 60000;
        // XInputSetState(0, &vibration);
        renderSomething(&backBuffer, xOffset, yOffset);
        HDC deviceContext = GetDC(window);
        win32_window_dimension dimension = Win32GetWindowDimension(window);
        Win32DisplayBufferInWindow(&backBuffer, deviceContext, dimension.width, dimension.height, 0, 0, dimension.width,
                                   dimension.height);
        ReleaseDC(window, deviceContext);
        // ++xOffset;
        // ++yOffset;
      }
    } else {
      // TODO logging
    }
  } else {
    // TODO logging
  }
  return (0);
}
