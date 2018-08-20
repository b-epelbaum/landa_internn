#pragma once

#ifdef WIN32
#include <Windows.h>

namespace LandaJune
{
	namespace Loggers
	{
		class WinLogConsole
		{
		public:
			WinLogConsole()
			{
				hConsole = nullptr;
			};

			// create the console
			bool   Create(const wchar_t* szTitle, bool bNoClose, bool& bUsedExisting);

			// set color for output
			void   Color(short wColor = 0) const;
			// write output to console
			void   Output(const wchar_t* szOutput = nullptr, ...) const;

			// set and get title of console
			void   SetTitle(const wchar_t* title) const;
			wchar_t*  GetTitle() const;

			// get HWND and/or HANDLE of console
			HWND   GetHWND() const;
			HANDLE GetHandle() const;

			// show/hide the console
			void   Show(bool bShow = true) const;
			// disable the [x] button of the console
			void   DisableClose() const;
			// clear all output
			void   Clear() const;

			// close the console and delete it
			void   Close();

		private:
			HANDLE hConsole;
		};
	}
}
#endif