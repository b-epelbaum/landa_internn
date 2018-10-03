#include "functions.h"
#include <string>
#include "util.h"
#include "ensureCleanup.h"
#include "cmdHdr.h"
#include "RealTimeStats.h"
#include <Windows.h>
#include "applog.h"

#define BUFFSIZE              (64 * 1024) // The size of an I/O buffer
#define MAX_PENDING_IO_REQS   4           // The maximum # of I/Os
#define CK_WRITE 2

namespace LandaJune
{
	using namespace Helpers;

	namespace Functions
	{
		class CIOCP 
		{
			public:
				explicit CIOCP(int nMaxConcurrency = -1) 
				{ 
					m_hIOCP = nullptr; 
					if (nMaxConcurrency != -1)
						(void) Create(nMaxConcurrency);
				}

				CIOCP(const CIOCP &) = delete;
				CIOCP(CIOCP &&) = delete;
				const CIOCP & operator = (const CIOCP &) = delete;
				CIOCP & operator = (CIOCP &&) = delete;

				~CIOCP() 
				{ 
					if (m_hIOCP != nullptr) 
						chVERIFY(CloseHandle(m_hIOCP)); 
				}


				BOOL Close() 
				{
					const auto bResult = CloseHandle(m_hIOCP);
					m_hIOCP = nullptr;
					return(bResult);
				}

				BOOL Create(const int nMaxConcurrency = 0) 
				{
					m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, nMaxConcurrency);
					chASSERT(m_hIOCP != nullptr);
					return(m_hIOCP != nullptr);
				}

				BOOL AssociateDevice(const HANDLE hDevice, const ULONG_PTR CompKey) const
				{
					const BOOL fOk = (CreateIoCompletionPort(hDevice, m_hIOCP, CompKey, 0) == m_hIOCP);
					chASSERT(fOk);
					return(fOk);
				}

				BOOL AssociateSocket(const SOCKET hSocket, const ULONG_PTR CompKey) const
				{
					return(AssociateDevice(reinterpret_cast<HANDLE>(hSocket), CompKey));
				}

				BOOL PostStatus(ULONG_PTR CompKey, DWORD dwNumBytes = 0, OVERLAPPED* po = nullptr) const
				{
					const auto fOk = PostQueuedCompletionStatus(m_hIOCP, dwNumBytes, CompKey, po);
					chASSERT(fOk);
					return(fOk);
				}

				BOOL GetStatus(ULONG_PTR* pCompKey, const PDWORD pdwNumBytes, OVERLAPPED** ppo, DWORD dwMilliseconds = INFINITE) const
				{
					return(GetQueuedCompletionStatus(m_hIOCP, pdwNumBytes, pCompKey, ppo, dwMilliseconds));
				}

			private:
				HANDLE m_hIOCP;
		};

		class CIOReq : public OVERLAPPED 
		{
			public:
				CIOReq() : _OVERLAPPED()
				{
					Internal = InternalHigh = 0;
					Offset = OffsetHigh = 0;
					hEvent = nullptr;
					m_nBuffSize = 0;
					m_pvData = nullptr;
				}

				CIOReq(const CIOReq &) = delete;
				CIOReq(CIOReq &&) = delete;
				const CIOReq & operator = (const CIOReq &) = delete;
				CIOReq & operator = (CIOReq &&) = delete;

				~CIOReq() 
				{
					if (m_pvData != nullptr)
					VirtualFree(m_pvData, 0, MEM_RELEASE);
				}

				BOOL AllocBuffer(const SIZE_T nBuffSize) 
				{
					m_nBuffSize = nBuffSize;
					m_pvData = VirtualAlloc(nullptr, m_nBuffSize, MEM_COMMIT, PAGE_READWRITE);
					return(m_pvData != nullptr);
				}
				
				BOOL Write(const HANDLE hDevice, const PLARGE_INTEGER pliOffset = nullptr) 
				{
					if (pliOffset != nullptr) 
					{
						Offset     = pliOffset->LowPart;
						OffsetHigh = pliOffset->HighPart;
					}
					return(::WriteFile(hDevice, m_pvData, m_nBuffSize, nullptr, this));
				}

			private:
				SIZE_T m_nBuffSize;
				PVOID  m_pvData;
		};

		void writeRawDataSeq(const char* buf, const size_t dataSize, const std::string& filePath )
		{
			auto const hFile = CreateFileA(filePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, nullptr);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				//throw 
				PRINT_ERROR << "frameSaveData [file " << filePath.c_str() << "] failed to open";
				return;
			}
			DWORD dwWritten = 0;
			auto const bRes =  WriteFile(hFile, buf, dataSize, &dwWritten, nullptr);
			CloseHandle(hFile);

			if (!bRes)
			{
				PRINT_ERROR << "frameSaveImage [file " << filePath.c_str() << "] failed to write";
			}
		}

		void writeRawDataIOCP(const char* buf, const size_t dataSize, const std::string& filePath )
		{
			BOOL fOk = FALSE;  
			LARGE_INTEGER liFileSizeSrc = { 0 };
			LARGE_INTEGER liFileSizeDst;

			liFileSizeDst.QuadPart = chROUNDUP(liFileSizeSrc.QuadPart, BUFFSIZE);

			try 
			{
				CIOReq ior[MAX_PENDING_IO_REQS];
				CIOCP iocp(0);
				// Open the destination file without buffering & set its size
				CEnsureCloseFile hFileDst = CreateFileA(filePath.c_str()
							, GENERIC_WRITE
							, 0
							, nullptr
							, CREATE_ALWAYS
							, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED
							, nullptr);

				if (hFileDst.IsInvalid()) goto leave;

				SetFilePointerEx(static_cast<HANDLE>(hFileDst), liFileSizeDst, nullptr, FILE_BEGIN);
				SetEndOfFile(static_cast<HANDLE>(hFileDst));

				// Create an I/O completion port and associate the files with it.
				iocp.AssociateDevice(static_cast<HANDLE>(hFileDst), CK_WRITE); // Write to destination file
				int nWritesInProgress = 0;

				for (int nIOReq = 0; nIOReq < _countof(ior); nIOReq++) 
				{
					// Each I/O request requires a data buffer for transfers
					chVERIFY(ior[nIOReq].AllocBuffer(BUFFSIZE));
					nWritesInProgress++;
					iocp.PostStatus(CK_WRITE, 0, &ior[nIOReq]);
				}

				// Loop while outstanding I/O requests still exist
				while ((nWritesInProgress > 0)) 
				{
					// Suspend the thread until an I/O completes
					ULONG_PTR CompletionKey;
					DWORD dwNumBytes;
					CIOReq* pior;
					iocp.GetStatus(&CompletionKey, &dwNumBytes, reinterpret_cast<OVERLAPPED**>(&pior), INFINITE);

					switch (CompletionKey) 
					{
						/*
						case CK_READ:  // Read completed, write to destination
							nReadsInProgress--;
							pior->Write(hFileDst);  // Write to same offset read from source
							nWritesInProgress++;
						break;
						*/

						case CK_WRITE:
						{
							nWritesInProgress--;
								/*
							if (liNextReadOffset.QuadPart < liFileSizeDst.QuadPart) 
							{
								// Not EOF, read the next block of data from the source file.
								pior->Read(hFileSrc, &liNextReadOffset);
								nReadsInProgress++;
								liNextReadOffset.QuadPart += BUFFSIZE; // Advance source offset
							}
								*/
						}
						break;
					}
					fOk = TRUE;
					leave:;
				}
			}
			catch(...)
			{
			}
		}

		void frameSaveData(SaveDataType& args) 
		{
			const auto sPath = std::get<1>(args);
			const auto sData = std::get<0>(args);
			const auto t0 = Utility::now_in_microseconds();
			
			writeRawDataSeq(reinterpret_cast<const char*>(sData->data()), sData->size(), sPath );

			RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_savedBitmapsOk, (Utility::now_in_microseconds() - t0) * 1.0e-6, sData->size());
		}
	}
}
