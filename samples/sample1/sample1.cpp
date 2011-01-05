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
#include "broker_worker.h"

#include <map>

const wchar_t kWorkerCmdline[] = L"--worker";

Broker* g_broker = NULL;

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int) {
  // The process can be either the broker or the child worker
  const wchar_t* cmdline = ::GetCommandLineW();
  if (wcsstr(cmdline, kWorkerCmdline)) {
    return WorkerMain(cmdline);
  } else {
    return BrokerMain(instance, cmdline);
  }
}

int WorkerMain(const wchar_t*) {
 ::MessageBoxW(NULL, L"worker process", L"sample1", MB_OK);

  Worker worker;
  if (!worker.ConnectToBroker(::GetCommandLineW())) {
    return 1;
  }
  for (int ix = 0; ix != 3000; ++ix) {
    std::string str;
   // if (!worker.ReadWebPage("www.yahoo.com", &str))
   //   return 1;
   
    //if (!worker.ReadWebPage("localhost", 8080, &str))
    //  return 1;

    if (!worker.WriteFileStr("x01234567899876543210")) {
      ::MessageBoxW(NULL, L"error worker exit", L"sample1", MB_OK);
      return 2;
    }
    ::Sleep(5);
  }
  ::MessageBoxW(NULL, L"normal worker exit", L"sample1", MB_OK);
  return 0;
}

int BrokerMain(HINSTANCE instance, const wchar_t*) {
  HWND win = CreateMainWindow(instance);
  if (!win) {
    return -1;
  }
  Broker broker(win);
  broker.SetPolicy(Broker::FILES, true);
  broker.SpawnWorker(kWorkerCmdline);
  g_broker = &broker;

  MSG msg = {0};
  while (::GetMessageW(&msg, NULL, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessageW(&msg);
	}

	return (int) msg.wParam;
}

HWND CreateMainWindow(HINSTANCE instance) {
  const wchar_t kWinClass[] = L"ipc.sample1.tw1";

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
      return NULL;
   }

   ::ShowWindow(window, SW_SHOWDEFAULT);
   ::UpdateWindow(window);

   ::SetTimer(window, 7, 2000, NULL);
   return window;
}

void PaintMainWindow(HDC dc, PAINTSTRUCT* /*ps*/) {
  ::SelectObject(dc, GetStockObject(DC_BRUSH));    
  ::SetDCBrushColor(dc, RGB(255,0,0));
  ::Rectangle(dc, 5, 4, 400, 300);
  
  wchar_t buf[256];
  ::wsprintfW(buf, L"worker file calls : %ld", g_broker->GetNumCallsPerArea(Broker::FILES));
  RECT rect = {20, 20, 200, 200};
  ::SetBkMode(dc, TRANSPARENT);
  ::DrawTextW(dc, buf, -1, &rect, DT_SINGLELINE);
}

LRESULT __stdcall WndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	PAINTSTRUCT ps;

	switch (message) {
	  case WM_COMMAND:
		  switch (LOWORD(wparam)) {
		    case IDM_EXIT:
          ::DestroyWindow(window);
		      break;
        case ID_FILE_SUICIDE:
          ::ExitProcess(0);
          break;
	      default:
          return ::DefWindowProc(window, message, wparam, lparam);
		  }
		  break;
    case WM_CLOSE:
      ::DestroyWindow(window);
      break;
    case WM_PAINT:
      PaintMainWindow(::BeginPaint(window, &ps), &ps);
      ::EndPaint(window, &ps);
      break;
	  case WM_DESTROY:
      ::KillTimer(window, 7);
      ::PostQuitMessage(0);
		  break;
    case WM_TIMER:
      ::InvalidateRect(window, NULL, true);
      break;
	  default:
      return ::DefWindowProc(window, message, wparam, lparam);
	}
	return 0;
}

