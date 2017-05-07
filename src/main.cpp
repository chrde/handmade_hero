#include <math.h>
#include <stdint.h>
#define internal static
#define local_persist static
#define global_variable static
#define PI 3.14159265359f
typedef int32_t bool32;

#include "handmade.cpp"

#include <dsound.h>
#include <stdio.h>
#include <windows.h>
#include <xinput.h>

#include "main.h"

internal void swap(void **p1, void **p2) {
  void *temp = *p1;
  *p1 = *p2;
  *p2 = temp;
}

// TODO change this
global_variable bool32 running;
global_variable win32_offscreen_buffer backBuffer;
global_variable LPDIRECTSOUNDBUFFER audioBuffer;

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

internal debug_read_file_result DEBUGPlatformReadEntireFile(char *filename) {
  debug_read_file_result result = {};
  HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if (fileHandle != INVALID_HANDLE_VALUE) {
    LARGE_INTEGER fileSize;
    if (GetFileSizeEx(fileHandle, &fileSize)) {
      uint32_t fileSize32 = safeTruncateUInt64(fileSize.QuadPart);
      result.contents = VirtualAlloc(0, fileSize32, MEM_COMMIT, PAGE_READWRITE);
      if (result.contents) {
        DWORD bytesRead;
        if (ReadFile(fileHandle, result.contents, fileSize32, &bytesRead, 0) && (fileSize32 == bytesRead)) {
          result.contentsSize = fileSize32;
        } else {
          DEBUGPlatformFreeFileMemory(result.contents);
          result.contents = 0;
        }
      }
    }
    CloseHandle(fileHandle);
  }
  return result;
}
internal void DEBUGPlatformFreeFileMemory(void *memory) {
  if (memory) {
    VirtualFree(memory, 0, MEM_RELEASE);
  }
}
internal bool32 DEBUGPlatformWriteEntireFile(char *filename, uint32_t memorySize, void *memory) {
  bool32 result = false;
  HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
  if (fileHandle != INVALID_HANDLE_VALUE) {
    DWORD bytesWritten;
    if (WriteFile(fileHandle, memory, memorySize, &bytesWritten, 0)) {
      result = (bytesWritten == memorySize);
    } else {
    }
    CloseHandle(fileHandle);
  } else {
  }
  return result;
}

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
      assert("keyboard input came in through a non-dispatch message");
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

internal void Win32ClearBuffer(win32_sound_output *soundOutput) {
  VOID *region0;
  DWORD region0Size;
  VOID *region1;
  DWORD region1Size;

  if (SUCCEEDED(
          audioBuffer->Lock(0, soundOutput->audioBufferSize, &region0, &region0Size, &region1, &region1Size, 0))) {
    uint8_t *destSample = (uint8_t *)region0;
    for (DWORD byteIndex = 0; byteIndex < region0Size; ++byteIndex) {
      *destSample++ = 0;
    }
    destSample = (uint8_t *)region1;
    for (DWORD byteIndex = 0; byteIndex < region1Size; ++byteIndex) {
      *destSample++ = 0;
    }
    audioBuffer->Unlock(region0, region0Size, region1, region1Size);
  }
}

internal void Win32FillSoundBuffer(win32_sound_output *soundBuffer, game_sound_output_buffer *gameSoundBuffer,
                                   DWORD byteToLock, DWORD bytesToWrite) {
  VOID *region0;
  DWORD region0Size;
  VOID *region1;
  DWORD region1Size;

  if (SUCCEEDED(audioBuffer->Lock(byteToLock, bytesToWrite, &region0, &region0Size, &region1, &region1Size, 0))) {
    // TODO assert that region0size/region1size is valid
    int16_t *destSample = (int16_t *)region0;
    int16_t *sourceSample = gameSoundBuffer->samples;
    DWORD region0SampleCount = region0Size / soundBuffer->bytesPerSample;
    for (DWORD sampleIndex = 0; sampleIndex < region0SampleCount; ++sampleIndex) {
      *destSample++ = *sourceSample++;
      *destSample++ = *sourceSample++;
      ++soundBuffer->runningSampleIndex;
    }
    destSample = (int16_t *)region1;
    DWORD region1SampleCount = region1Size / soundBuffer->bytesPerSample;
    for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex) {
      *destSample++ = *sourceSample++;
      *destSample++ = *sourceSample++;
      ++soundBuffer->runningSampleIndex;
    }
    audioBuffer->Unlock(region0, region0Size, region1, region1Size);
  }
}

internal void Win32ProcessXInputDigitalButton(game_button_state *oldState, game_button_state *newState,
                                              DWORD buttonsState, DWORD buttonBit) {
  newState->endedDown = (buttonsState & buttonBit) == buttonBit;
  newState->halfTransitionCount = (oldState->endedDown != newState->endedDown) ? 1 : 0;
}

internal void Win32ProcessKeyboardMessage(game_button_state *newState, bool32 isDown) {
  assert(newState->endedDown != isDown);
  newState->endedDown = isDown;
  ++newState->halfTransitionCount = newState->endedDown ? 1 : 0;
}

internal void Win32ProcessPendingMessages(game_controller_input *keyboardController) {
  MSG message;
  while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
    switch (message.message) {
      case WM_QUIT: {
        running = false;
        break;
      }
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
      case WM_KEYDOWN:
      case WM_KEYUP: {
        uint32_t vkCode = (uint32_t)message.wParam;
        bool32 wasDown = ((message.lParam & (1 << 30)) != 0);
        bool32 isDown = ((message.lParam & (1 << 31)) == 0);
        if (isDown == wasDown) {
          break;
        }
        if (vkCode == VK_UP) {
          Win32ProcessKeyboardMessage(&keyboardController->actionUp, isDown);
        } else if (vkCode == VK_LEFT) {
          Win32ProcessKeyboardMessage(&keyboardController->actionLeft, isDown);
        } else if (vkCode == VK_RIGHT) {
          Win32ProcessKeyboardMessage(&keyboardController->actionRight, isDown);
        } else if (vkCode == VK_DOWN) {
          Win32ProcessKeyboardMessage(&keyboardController->actionDown, isDown);
        } else if (vkCode == 'Q') {
          Win32ProcessKeyboardMessage(&keyboardController->leftShoulder, isDown);
        } else if (vkCode == 'E') {
          Win32ProcessKeyboardMessage(&keyboardController->rightShoulder, isDown);
        } else if (vkCode == 'W') {
          Win32ProcessKeyboardMessage(&keyboardController->moveUp, isDown);
        } else if (vkCode == 'A') {
          Win32ProcessKeyboardMessage(&keyboardController->moveLeft, isDown);
        } else if (vkCode == 'S') {
          Win32ProcessKeyboardMessage(&keyboardController->moveDown, isDown);
        } else if (vkCode == 'D') {
          Win32ProcessKeyboardMessage(&keyboardController->moveRight, isDown);
        } else if (vkCode == 'H') {
          if (isDown) {
            OutputDebugString("H is down\n");
          }
          if (wasDown) {
            OutputDebugString("H was down\n");
          }
          OutputDebugString("H\n");
        } else if (vkCode == VK_ESCAPE) {
          running = false;
        } else if (vkCode == VK_SPACE) {
        }
        bool32 altKeyWasDown = (message.lParam & (1 << 29));
        if ((vkCode == VK_F4) && altKeyWasDown) {
          running = false;
        }
        break;
      }
      default: {
        TranslateMessage(&message);
        DispatchMessage(&message);
      }
    }
  }
}

internal float_t Win32ProcessXInputStickValue(int16_t deadzone, float_t thumbValue) {
  float_t result = 0;
  if (thumbValue < -deadzone) {
    result = (thumbValue + deadzone) / (32768.0f - deadzone);
  } else if (thumbValue > deadzone) {
    result = (thumbValue - deadzone) / (32767.0f - deadzone);
  }
  return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode) {
  LARGE_INTEGER perfCountFrequencyResult;
  QueryPerformanceFrequency(&perfCountFrequencyResult);
  int64_t perfCountFrequency = perfCountFrequencyResult.QuadPart;
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
      win32_sound_output soundBuffer = {};
      soundBuffer.samplesPerSecond = 48000;
      soundBuffer.runningSampleIndex = 0;
      soundBuffer.bytesPerSample = sizeof(int16_t) * 2;  // waveFormat.nSamples * waveFormat.bytesPerSample
      soundBuffer.audioBufferSize = soundBuffer.samplesPerSecond * soundBuffer.bytesPerSample;
      soundBuffer.latencySampleCount = soundBuffer.samplesPerSecond / 15;
      Win32InitDSound(window, soundBuffer.samplesPerSecond, soundBuffer.audioBufferSize);
      Win32ClearBuffer(&soundBuffer);
      audioBuffer->Play(0, 0, DSBPLAY_LOOPING);
      int16_t *samples =
          (int16_t *)VirtualAlloc(0, soundBuffer.audioBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#if HANDMADE_INTERNAL
      LPVOID baseAddress = (LPVOID)Terabytes(2);
#else
      LPVOID baseAddress = 0;
#endif
      game_memory gameMemory = {};
      gameMemory.permanentStorageSize = Megabytes(64);
      gameMemory.transientStorageSize = Gigabytes(4);
      uint64_t totalSize = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
      gameMemory.permanentStorage =
          VirtualAlloc(baseAddress, (size_t)totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
      gameMemory.transientStorage = (uint8_t *)gameMemory.permanentStorage + gameMemory.permanentStorageSize;
      if (samples && gameMemory.transientStorage && gameMemory.permanentStorage) {
        game_input input[2] = {};
        game_input *newInput = &input[0];
        game_input *oldInput = &input[1];
        LARGE_INTEGER lastCounter;
        QueryPerformanceCounter(&lastCounter);
        uint64_t lastCycleCount = __rdtsc();
        while (running) {
          game_controller_input *newKeyboardController = getController(newInput, 0);
          game_controller_input *oldKeyboardController = getController(oldInput, 0);
          *newKeyboardController = {};
          newKeyboardController->isConnected = true;

          for (int buttonIndex = 0; buttonIndex < arrayCount(newKeyboardController->buttons); ++buttonIndex) {
            newKeyboardController->buttons[buttonIndex].endedDown =
                oldKeyboardController->buttons[buttonIndex].endedDown;
          }

          Win32ProcessPendingMessages(newKeyboardController);
          DWORD maxControllerCount = XUSER_MAX_COUNT;
          if (maxControllerCount > arrayCount(newInput->controllers) - 1) {
            maxControllerCount = arrayCount(newInput->controllers) - 1;
          }
          for (DWORD controllerIndex = 0; controllerIndex < maxControllerCount; controllerIndex++) {
            DWORD ourControllerIndex = controllerIndex + 1;
            game_controller_input *oldController = getController(oldInput, ourControllerIndex);
            game_controller_input *newController = getController(newInput, ourControllerIndex);
            XINPUT_STATE state;
            // ZeroMemory(&state, sizeof(XINPUT_STATE));

            if (XInputGetState(controllerIndex, &state) == ERROR_SUCCESS) {
              // TODO see if state.dwPacketNumber is too big
              XINPUT_GAMEPAD *pad = &state.Gamepad;

              newController->stickAverageX =
                  Win32ProcessXInputStickValue(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, pad->sThumbLX);
              newController->stickAverageY =
                  Win32ProcessXInputStickValue(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, pad->sThumbLY);

              if (newController->stickAverageX != 0.0f || newController->stickAverageY != 0.0f) {
                newController->isAnalog = true;
              }

              if (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP) {
                newController->isAnalog = false;
                newController->stickAverageY = 1.0f;
              }

              if (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
                newController->isAnalog = false;
                newController->stickAverageY = -1.0f;
              }

              if (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
                newController->isAnalog = false;
                newController->stickAverageX = -1.0f;
              }

              if (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
                newController->isAnalog = false;
                newController->stickAverageX = 1.0f;
              }

              float_t threshold = 0.5f;
              Win32ProcessXInputDigitalButton(&oldController->moveUp, &newController->moveUp, 1,
                                              (newController->stickAverageY < threshold) ? 1 : 0);
              Win32ProcessXInputDigitalButton(&oldController->moveDown, &newController->moveDown, 1,
                                              (newController->stickAverageY < -threshold) ? 1 : 0);
              Win32ProcessXInputDigitalButton(&oldController->moveLeft, &newController->moveLeft, 1,
                                              (newController->stickAverageX < -threshold) ? 1 : 0);
              Win32ProcessXInputDigitalButton(&oldController->moveRight, &newController->moveRight, 1,
                                              (newController->stickAverageX < threshold) ? 1 : 0);

              Win32ProcessXInputDigitalButton(&oldController->actionDown, &newController->actionDown, pad->wButtons,
                                              XINPUT_GAMEPAD_A);
              Win32ProcessXInputDigitalButton(&oldController->actionUp, &newController->actionUp, pad->wButtons,
                                              XINPUT_GAMEPAD_Y);
              Win32ProcessXInputDigitalButton(&oldController->actionRight, &newController->actionRight, pad->wButtons,
                                              XINPUT_GAMEPAD_B);
              Win32ProcessXInputDigitalButton(&oldController->actionLeft, &newController->actionLeft, pad->wButtons,
                                              XINPUT_GAMEPAD_X);
              Win32ProcessXInputDigitalButton(&oldController->leftShoulder, &newController->leftShoulder, pad->wButtons,
                                              XINPUT_GAMEPAD_LEFT_SHOULDER);
              Win32ProcessXInputDigitalButton(&oldController->rightShoulder, &newController->rightShoulder,
                                              pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
              Win32ProcessXInputDigitalButton(&oldController->start, &newController->start, pad->wButtons,
                                              XINPUT_GAMEPAD_START);
              Win32ProcessXInputDigitalButton(&oldController->back, &newController->back, pad->wButtons,
                                              XINPUT_GAMEPAD_BACK);
              newController->isConnected = true;
            } else {
              newController->isConnected = false;
            }
          }
          XINPUT_VIBRATION vibration;
          vibration.wLeftMotorSpeed = 60000;
          vibration.wRightMotorSpeed = 60000;
          // XInputSetState(0, &vibration);
          DWORD playCursor = 0;
          DWORD writeCursor = 0;
          DWORD byteToLock = 0;
          DWORD bytesToWrite = 0;
          DWORD targetCursor = 0;
          bool32 soundIsValid = false;
          if (SUCCEEDED(audioBuffer->GetCurrentPosition(&playCursor, &writeCursor))) {
            byteToLock = (soundBuffer.runningSampleIndex * soundBuffer.bytesPerSample) % soundBuffer.audioBufferSize;
            targetCursor = (playCursor + (soundBuffer.latencySampleCount * soundBuffer.bytesPerSample)) %
                           soundBuffer.audioBufferSize;
            if (byteToLock > targetCursor) {
              bytesToWrite = soundBuffer.audioBufferSize - byteToLock;
              bytesToWrite += targetCursor;
            } else {
              bytesToWrite = targetCursor - byteToLock;
            }
            soundIsValid = true;
          }
          game_sound_output_buffer gameSoundBuffer = {};
          gameSoundBuffer.samplesPerSecond = soundBuffer.samplesPerSecond;
          gameSoundBuffer.sampleCount = bytesToWrite / soundBuffer.bytesPerSample;
          gameSoundBuffer.samples = samples;
          game_offscreen_buffer gameBuffer = {};
          gameBuffer.memory = backBuffer.memory;
          gameBuffer.width = backBuffer.width;
          gameBuffer.height = backBuffer.height;
          gameBuffer.pitch = backBuffer.pitch;

          GameUpdateAndRender(&gameMemory, &gameBuffer, &gameSoundBuffer, newInput);
          if (soundIsValid) {
            Win32FillSoundBuffer(&soundBuffer, &gameSoundBuffer, byteToLock, bytesToWrite);
          }

          win32_window_dimension dimension = Win32GetWindowDimension(window);
          Win32DisplayBufferInWindow(&backBuffer, deviceContext, dimension.width, dimension.height);
          uint64_t endCycleCount = __rdtsc();
          LARGE_INTEGER endCounter;
          QueryPerformanceCounter(&endCounter);
          uint64_t cyclesElapsed = endCycleCount - lastCycleCount;
          int64_t counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
          float_t msPerFrame = (1000.0f * (float_t)counterElapsed) / (float_t)perfCountFrequency;
          float_t fps = (float_t)perfCountFrequency / (float_t)counterElapsed;
          float_t MCPF = ((float_t)cyclesElapsed / (1000.0f * 1000.0f));
#if 0
          char buffer[256];
          sprintf_s(buffer, " %.02fms/f, %.02fFPS, %.02fmc/f\n", msPerFrame, fps, MCPF);
          OutputDebugStringA(buffer);
#endif
          lastCounter = endCounter;
          lastCycleCount = endCycleCount;
          swap(&(void *)newInput, &(void *)oldInput);
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