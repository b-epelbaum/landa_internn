#include "functions.h"
#include <string>
#include "util.h"
#include "RealTimeStats.h"
#include <Windows.h>
#include "applog.h"

namespace LandaJune
{
	using namespace Helpers;

	namespace Functions
	{
		void frameSaveImage(std::tuple<std::shared_ptr<std::vector<unsigned char>>, std::string> & args) 
		{
			const auto sPath = std::get<1>(args);
			const auto pImage = std::get<0>(args);
			const auto t0 = Utility::now_in_microseconds();

			auto const hFile = CreateFileA(sPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, nullptr);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				//throw 
				PRINT_ERROR << "frameSaveImage [file " << sPath.c_str() << "] failed to open";
				return;
			}
			DWORD dwWritten = 0;
			auto const bRes =  WriteFile(hFile, pImage->data(), pImage->size(), &dwWritten, nullptr);
			CloseHandle(hFile);

			if (!bRes)
			{
				PRINT_ERROR << "frameSaveImage [file " << sPath.c_str() << "] failed to write";
			}
			RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_savedBitmapsOk, (Utility::now_in_microseconds() - t0) * 1.0e-6, pImage->size());
		}
	}
}
