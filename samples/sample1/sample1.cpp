// Copyright (c) 2010 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "winheaders.h"
#include "sample1.h"

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE, wchar_t* cmdline, int nshow) {
  if (!CreateMainWindow(instance, nshow)) {
    return -1;
  }
  MSG msg = {0};
  while (::GetMessageW(&msg, NULL, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessageW(&msg);
	}
	return (int) msg.wParam;
}


bool CreateMainWindow(HINSTANCE instance, int nshow) {
  const wchar_t kWinClass[] = L"ipc.sample1.tp1";

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			   = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	 = WndProc;
	wcex.cbClsExtra		 = 0;
	wcex.cbWndExtra		 = 0;
	wcex.hInstance		 = instance;
  wcex.hIcon			   = ::LoadIcon(instance, MAKEINTRESOURCE(IDI_SAMPLE1));
  wcex.hCursor		   = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	 = MAKEINTRESOURCE(IDC_SAMPLE1);
	wcex.lpszClassName = kWinClass;
  wcex.hIconSm		   = ::LoadIcon(instance, MAKEINTRESOURCE(IDI_SMALL));

  HWND window = ::CreateWindowW(MAKEINTATOM(::RegisterClassExW(&wcex)), L"*",
                                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 500, 500,
                                NULL, NULL, instance, NULL);
   if (!window) {
      return false;
   }
   ::ShowWindow(window, nshow);
   ::UpdateWindow(window);
   return true;
}

void PaintMainWindow(HDC dc, PAINTSTRUCT* ps) {
  ::SelectObject(dc, GetStockObject(DC_BRUSH));    
  ::SetDCBrushColor(dc, RGB(255,0,0));
  ::Rectangle(dc, 5, 4, 400, 300);
}

LRESULT __stdcall WndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	PAINTSTRUCT ps;

	switch (message) {
	  case WM_COMMAND:
		  switch (LOWORD(wparam)) {
		    case IDM_EXIT:
          ::DestroyWindow(window);
		      break;
	      default:
          return ::DefWindowProc(window, message, wparam, lparam);
		  }
		  break;
    case WM_PAINT:
      PaintMainWindow(::BeginPaint(window, &ps), &ps);
      ::EndPaint(window, &ps);
      break;
	  case WM_DESTROY:
      ::PostQuitMessage(0);
		  break;
	  default:
      return ::DefWindowProc(window, message, wparam, lparam);
	}
	return 0;
}

