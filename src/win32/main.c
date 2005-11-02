/*
 * main.c - Win32 port specific code
 *
 * Copyright (C) 2000 Krzysztof Nikiel
 * Copyright (C) 2000-2005 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <process.h>

#include "atari.h"
#include "input.h"
#include "platform.h"
#include "screen.h"
#include "sound.h"
#include "ui.h"

#include "main.h"
#include "screen_win32.h"
#include "keyboard.h"
#include "joystick.h"

char *myname = "Atari800";
HWND hWndMain;
HINSTANCE myInstance;

static int bActive = 0;		/* activity indicator */

#if 1
void exit(int code)
{
	MSG msg;
	PostMessage(hWndMain, WM_CLOSE, 0, 0);
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	ExitProcess(code);
}
#endif

static long FAR PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_ACTIVATEAPP:
		bActive = wParam;
		if (bActive) {
			kbreacquire();
#ifdef SOUND
			Sound_Continue();
#endif
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		mouse_buttons = ((wParam & MK_LBUTTON) ? 1 : 0)
		              | ((wParam & MK_RBUTTON) ? 2 : 0);
		break;
	case WM_SETCURSOR:
		SetCursor(NULL);
		return TRUE;
	case WM_CREATE:
		break;
	case WM_CLOSE:
		groff();
#ifdef SOUND
		Sound_Exit();
#endif
		uninitjoystick();
		uninitinput();
		break;
	case WM_DESTROY:
		PostQuitMessage(10);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

static BOOL initwin(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = myname;
	wc.lpszClassName = myname;
	RegisterClass(&wc);

	hWndMain = CreateWindowEx(
		0, myname, myname, WS_POPUP, 0, 0,
		GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
		NULL, NULL, hInstance, NULL);

	if (!hWndMain)
		return 1;
	return 0;
}

#define MOUSE_CENTER_X  100
#define MOUSE_CENTER_Y  100

int main(int argc, char *argv[])
{
	STARTUPINFO si;
	MSG msg;
	POINT mouse;

	myInstance = GetModuleHandle(NULL);
	si.dwFlags = 0;
	GetStartupInfo(&si);
	if (initwin(myInstance, si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT))
		return 1;

	/* initialise Atari800 core */
	if (!Atari800_Initialise(&argc, argv))
		return 3;

	msg.message = WM_NULL;

	/* main loop */
	for (;;) {

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		if (!bActive)
			continue;

		key_code = Atari_Keyboard();

		GetCursorPos(&mouse);
		mouse_delta_x = mouse.x - MOUSE_CENTER_X;
		mouse_delta_y = mouse.y - MOUSE_CENTER_Y;
		if (mouse_delta_x | mouse_delta_y)
			SetCursorPos(MOUSE_CENTER_X, MOUSE_CENTER_Y);

		Atari800_Frame();
		if (display_screen)
			Atari_DisplayScreen();
	}

	return msg.wParam;
}
