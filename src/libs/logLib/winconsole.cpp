#ifdef WIN32

#include "winconsole.h"
#include <cstdio>

using namespace LandaJune::Loggers;

#define ERR(bSuccess) { if(!bSuccess) return; }
#define CHECK(hHandle) { if(hHandle == NULL) return; };


bool WinLogConsole::Create(const wchar_t* szTitle, const bool bNoClose, bool& bUsedExisting )
{
	// Has console been already created?
	if(hConsole != nullptr)
		return false;
	
	// Allocate a new console for our app
	if(AllocConsole())
	{
		bUsedExisting = false;
	// Create the actual console
		hConsole = CreateFileW(L"CONOUT$", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		if(hConsole == INVALID_HANDLE_VALUE)
			return false;

		if(SetConsoleMode(hConsole, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT ) == 0)
			return false;


		// sync stdio with the console
		_wfreopen(L"CONIN$", L"r", stdin);
		_wfreopen(L"CONOUT$", L"w", stdout);
		_wfreopen(L"CONOUT$", L"w", stderr);

		auto bRet = SetStdHandle(STD_OUTPUT_HANDLE, hConsole);
	}
	else
	{
		bUsedExisting = true;
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	// if true, disable the [x] button of the console
	if(bNoClose)
		DisableClose();
				
	// set the console title
	if(szTitle != nullptr)
		SetConsoleTitleW(szTitle);
	
	return true;
}

void WinLogConsole::Color(const short wColor) const
{
	CHECK(hConsole);

	// no color specified, reset to defaults (white font on black background)
	if(wColor != NULL)
		SetConsoleTextAttribute(hConsole, wColor );
	// change font and/or background color
	else
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE ); // white text on black bg
}

void WinLogConsole::Output(const wchar_t* szOutput, ...) const
{
	CHECK(hConsole);
	DWORD		dwWritten;
	// if not parameter set, write a new line
	if(szOutput == nullptr) 
    {
        WriteConsoleW(hConsole, L"\n", 1, &dwWritten, nullptr);
    }
	// process arguments
	else
	{
        WriteConsoleW(hConsole, szOutput, static_cast<DWORD>(wcslen(szOutput)) ,&dwWritten, nullptr);
	}		
}

void WinLogConsole::SetTitle(const wchar_t *title) const
{
	// self-explanatory
	SetConsoleTitleW(title);
}

wchar_t * WinLogConsole::GetTitle() const
{
	// get the title of our console and return it
	static wchar_t szWindowTitle[256] = L"";
	GetConsoleTitleW(szWindowTitle, wcslen(szWindowTitle));

	return szWindowTitle;
}


HWND WinLogConsole::GetHWND() const
{
	if(hConsole == nullptr) 
		return nullptr;

	// try to find our console window and return its HWND
	return FindWindowW(L"ConsoleWindowClass", GetTitle() );
}

void WinLogConsole::Show(const bool bShow) const
{
	CHECK(hConsole);

	// get out console's HWND and show/hide the console
	const auto hWnd = GetHWND();
	if(hWnd != nullptr)
		ShowWindow(hWnd, SW_HIDE ? SW_SHOW : bShow);
}

void WinLogConsole::DisableClose() const
{
	CHECK(hConsole);

	const auto hWnd = GetHWND();
	
	// disable the [x] button if we found our console
	if(hWnd != nullptr)
	{
		const auto hMenu = GetSystemMenu(hWnd,0);
		if(hMenu != nullptr)
		{
			DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
			DrawMenuBar(hWnd);
		}
	}
}


void WinLogConsole::Clear() const
{
	CHECK(hConsole);

	/***************************************/
	// This code is from one of Microsoft's
	// knowledge base articles, you can find it at 
    // http://support.microsoft.com/default.aspx?scid=KB;EN-US;q99261&
	/***************************************/

	const COORD coordScreen = { 0, 0 };

	DWORD cTCHARsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */

	/* get the number of TCHARacter cells in the current buffer */

	auto bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
    ERR(bSuccess);
	const DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    /* fill the entire screen with blanks */ 

    bSuccess = FillConsoleOutputCharacterW( hConsole, static_cast<wchar_t>(L' '), dwConSize, coordScreen, &cTCHARsWritten );
    ERR(bSuccess);

    /* get the current text attribute */ 

    bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
    ERR(bSuccess);

    /* now set the buffer's attributes accordingly */ 

    bSuccess = FillConsoleOutputAttribute( hConsole, csbi.wAttributes, dwConSize, coordScreen, &cTCHARsWritten );
    ERR(bSuccess);

    /* put the cursor at (0, 0) */ 

    bSuccess = SetConsoleCursorPosition( hConsole, coordScreen );
    ERR(bSuccess);
}


HANDLE WinLogConsole::GetHandle() const
{
	// simply return the handle to our console
	return hConsole;
}

void WinLogConsole::Close()
{
	// free the console, now it can't be used anymore until we Create() it again
	auto hwnd = GetConsoleWindow();
	CloseHandle(hConsole);
	FreeConsole();
	hConsole = nullptr;
	CloseWindow(hwnd);
}
#endif