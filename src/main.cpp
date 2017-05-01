#include <dsound.h>
#include <math.h>
#include <stdint.h>
#include <windows.h>
#include <xinput.h>
#define internal static
#define local_persist static
#define global_variable static
#define PI 3.14159265359f
typedef int32_t bool32;

struct win32_offscreen_buffer {
  // NOTE pixels are always 32bits wide, Memory order BB GG RR XX
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int pitch;
};

// TODO change this
global_variable bool32 running;
global_variable win32_offscreen_buffer backBuffer;
global_variable LPDIRECTSOUNDBUFFER audioBuffer;

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
    xInputLibrary = LoadLibraryA("xinput9_1_0.dll");
  }
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
      if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &audioBuffer, 0))) {
        OutputDebugStringA("Audio buffer created.\n");
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

  int bytesPerPixel = 4;
  int memorySize = (width * height) * bytesPerPixel;

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
  buffer->pitch = buffer->width * bytesPerPixel;
  buffer->memory = VirtualAlloc(0, memorySize, MEM_COMMIT, PAGE_READWRITE);

  // TODO clear to black
}

internal void Win32DisplayBufferInWindow(win32_offscreen_buffer *buffer, HDC deviceContext, int windowWidth,
                                         int windowHeight) {
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
      bool32 wasDown = ((lParam & (1 << 30)) != 0);
      bool32 isDown = ((lParam & (1 << 31)) == 0);
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
      win32_window_dimension dimension = Win32GetWindowDimension(window);
      Win32DisplayBufferInWindow(&backBuffer, deviceContext, dimension.width, dimension.height);
      EndPaint(window, &paint);
      break;
    }
    case WM_ACTIVATEAPP: {
      OutputDebugString("WM_ACTIVATEAPP\n");
      break;
    }
    default: {
      result = DefWindowProcA(window, message, wParam, lParam);
      break;
    }
  }
  return result;
}

struct win32_sound_output {
  int samplesPerSecond;
  int toneHz;
  uint32_t runningSampleIndex;
  int wavePeriod;
  int bytesPerSample;
  int audioBufferSize;
  int16_t toneVolume;
  float_t tSine;
  int latencySampleCount;
};

internal void Win32FillSoundBuffer(win32_sound_output *soundOutput, DWORD byteToLock, DWORD bytesToWrite) {
  VOID *region0;
  DWORD region0Size;
  VOID *region1;
  DWORD region1Size;

  if (SUCCEEDED(audioBuffer->Lock(byteToLock, bytesToWrite, &region0, &region0Size, &region1, &region1Size, 0))) {
    // TODO assert that region0size/region1size is valid
    int16_t *sampleOut = (int16_t *)region0;
    DWORD region0SampleCount = region0Size / soundOutput->bytesPerSample;
    for (DWORD sampleIndex = 0; sampleIndex < region0SampleCount; ++sampleIndex) {
      float_t sineValue = sinf(soundOutput->tSine);
      int16_t sampleValue = (int16_t)(sineValue * soundOutput->toneVolume);
      *sampleOut++ = sampleValue;
      *sampleOut++ = sampleValue;
      soundOutput->tSine += 2.0f * PI * 1.0f / (float_t)soundOutput->wavePeriod;
      ++soundOutput->runningSampleIndex;
    }
    sampleOut = (int16_t *)region1;
    DWORD region1SampleCount = region1Size / soundOutput->bytesPerSample;
    for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex) {
      float_t sineValue = sinf(soundOutput->tSine);
      int16_t sampleValue = (int16_t)(sineValue * soundOutput->toneVolume);
      *sampleOut++ = sampleValue;
      *sampleOut++ = sampleValue;
      soundOutput->tSine += 2.0f * PI * 1.0f / (float_t)soundOutput->wavePeriod;
      ++soundOutput->runningSampleIndex;
    }
    audioBuffer->Unlock(region0, region0Size, region1, region1Size);
  }
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode) {
  Win32LoadXInput();
  WNDCLASSA WindowClass = {};
  Win32ResizeDIBSection(&backBuffer, 1280, 720);
  WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  WindowClass.lpfnWndProc = Win32MainWindowCallback;
  WindowClass.hInstance = instance;
  WindowClass.lpszClassName = "MyWindowClass";
  if (RegisterClass(&WindowClass)) {
    HWND window = CreateWindowEx(0, WindowClass.lpszClassName, "Super stuff", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);
    if (window) {
      HDC deviceContext = GetDC(window);
      running = true;
      int xOffset = 0;
      int yOffset = 0;
      win32_sound_output soundOutput = {};
      soundOutput.samplesPerSecond = 48000;
      soundOutput.toneHz = 256;
      soundOutput.runningSampleIndex = 0;
      soundOutput.wavePeriod = soundOutput.samplesPerSecond / soundOutput.toneHz;
      soundOutput.bytesPerSample = sizeof(int16_t) * 2;  // waveFormat.nSamples * waveFormat.bytesPerSample
      soundOutput.audioBufferSize = soundOutput.samplesPerSecond * soundOutput.bytesPerSample;
      soundOutput.toneVolume = 5000;
      soundOutput.latencySampleCount = soundOutput.samplesPerSecond / 15;
      Win32InitDSound(window, soundOutput.samplesPerSecond, soundOutput.audioBufferSize);
      Win32FillSoundBuffer(&soundOutput, 0, soundOutput.latencySampleCount * soundOutput.bytesPerSample);
      audioBuffer->Play(0, 0, DSBPLAY_LOOPING);
      // bool32 soundIsPlaying = false;
      while (running) {
        MSG message;
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

            bool32 up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool32 down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool32 left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool32 right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool32 start = (pad->wButtons & XINPUT_GAMEPAD_START);
            bool32 back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool32 leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool32 rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool32 A = (pad->wButtons & XINPUT_GAMEPAD_A);
            bool32 B = (pad->wButtons & XINPUT_GAMEPAD_B);
            bool32 X = (pad->wButtons & XINPUT_GAMEPAD_X);
            bool32 Y = (pad->wButtons & XINPUT_GAMEPAD_Y);
            int16_t stickX = pad->sThumbLX;
            int16_t stickY = pad->sThumbLY;

            xOffset += stickX /4096;
            yOffset += stickY /4096;
            soundOutput.toneHz = 512 + (int)(256.0f * ((float_t)stickX / 30000.f));
            soundOutput.wavePeriod = soundOutput.samplesPerSecond / soundOutput.toneHz;
          } else {
            // Controller is not connected
          }
        }
        XINPUT_VIBRATION vibration;
        vibration.wLeftMotorSpeed = 60000;
        vibration.wRightMotorSpeed = 60000;
        // XInputSetState(0, &vibration);
        renderSomething(&backBuffer, xOffset, yOffset);

        DWORD playCursor;
        DWORD writeCursor;
        if (SUCCEEDED(audioBuffer->GetCurrentPosition(&playCursor, &writeCursor))) {
          DWORD byteToLock = (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.audioBufferSize;
          DWORD targetCursor = (playCursor +(soundOutput.latencySampleCount * soundOutput.bytesPerSample))% soundOutput.audioBufferSize;
          DWORD bytesToWrite;
          if (byteToLock > targetCursor) {
            bytesToWrite = soundOutput.audioBufferSize - byteToLock;
            bytesToWrite += targetCursor;
          } else {
            bytesToWrite = targetCursor - byteToLock;
          }
          Win32FillSoundBuffer(&soundOutput, byteToLock, bytesToWrite);
        }
        win32_window_dimension dimension = Win32GetWindowDimension(window);
        Win32DisplayBufferInWindow(&backBuffer, deviceContext, dimension.width, dimension.height);
        // ReleaseDC(window, deviceContext);
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
