#include "web_server.hpp"
#include <stdio.h>
#include <windows.h>
#include "loopback-capture_lib\common.h"
#include "loopback-capture_lib\loopback-capture.h"
#include <thread>

using namespace std;
//void LoopbackCaptureThreadFunction(LoopbackCaptureThreadFunctionArguments *pArgs);

void LoopbackCaptureThreadFunction(LoopbackCaptureThreadFunctionArguments *pArgs) {

	pArgs->hr = LoopbackCapture(
		pArgs->pMMDevice,
		pArgs->hFile,
		pArgs->bInt16,
		&pArgs->nFrames
	);
}

int _cdecl wmain(int argc, LPCWSTR argv[]) {

	//start_server();
	//server_thread->join();
	HRESULT hr = S_OK;

	// parse command line
	CPrefs prefs(argc, argv, hr);
	if (FAILED(hr)) {
		ERR(L"CPrefs::CPrefs constructor failed: hr = 0x%08x", hr);
		return -__LINE__;
	}
	if (S_FALSE == hr) {
		// nothing to do
		return 0;
	}

	// create arguments for loopback capture thread
	LoopbackCaptureThreadFunctionArguments threadArgs;
	threadArgs.hr = E_UNEXPECTED; // thread will overwrite this
	threadArgs.pMMDevice = prefs.m_pMMDevice;
	threadArgs.bInt16 = prefs.m_bInt16;
	threadArgs.hFile = prefs.m_hFile;
	threadArgs.nFrames = 0;
	//std::thread t([&](viewWindow* view){ view->refreshWindow(render, playerRect, backTexture, playerTexture); }, &window);
	thread start_capture_thread([&]() 
	{
		LoopbackCaptureThreadFunction(&threadArgs); 
	});
	start_capture_thread.join();

	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	if (hStdIn == INVALID_HANDLE_VALUE) ERR("GetStdHandle");

	// at this point capture is running
	// wait for the user to press a key or for capture to error out
	{
		LOG(L"%s", L"Press Enter to quit...");

		bool bKeepWaiting = true;
		while (bKeepWaiting) {
				INPUT_RECORD rInput[128];
				DWORD nEvents;
				if (!ReadConsoleInput(hStdIn, rInput, ARRAYSIZE(rInput), &nEvents)) {
					ERR(L"ReadConsoleInput failed: last error is %u", GetLastError());
					bKeepWaiting = false;
				}
				else {
					for (DWORD i = 0; i < nEvents; i++) {
						if (
							KEY_EVENT == rInput[i].EventType &&
							VK_RETURN == rInput[i].Event.KeyEvent.wVirtualKeyCode &&
							!rInput[i].Event.KeyEvent.bKeyDown
							) {
							LOG(L"%s", L"Stopping capture...");
							bKeepWaiting = false;
							break;
						}
					}
					// if none of them were Enter key-up events,
					// continue waiting
				}

		} // while
	} // naked scope

	  // at this point the thread is definitely finished

	// write the correct data to the fact chunk
	//LONG lBytesWritten = mmioWrite(
	//	prefs.m_hFile,
	//	reinterpret_cast<PCHAR>(&threadArgs.nFrames),
	//	sizeof(threadArgs.nFrames)
	//);
	//if (lBytesWritten != sizeof(threadArgs.nFrames)) {
	//	ERR(L"Updating the fact chunk wrote %u bytes; expected %u", lBytesWritten, (UINT32)sizeof(threadArgs.nFrames));
	//	return -__LINE__;
	//}

	
}