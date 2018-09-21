#pragma once
#include <Windows.h>
//#include "cmdHdr.h" 


namespace LandaJune
{
	namespace Helpers
	{
		typedef VOID (WINAPI* PFNENSURECLEANUP)(UINT_PTR);

		template<typename TYPE, PFNENSURECLEANUP pfn, UINT_PTR tInvalid = NULL> 
		class CEnsureCleanup 
		{
			public:
			   // Default constructor assumes an invalid value (nothing to cleanup)
				CEnsureCleanup() { m_t = tInvalid; }

			   // This constructor sets the value to the specified value
				CEnsureCleanup(TYPE t) : m_t((UINT_PTR)t) { }

			   // The destructor performs the cleanup.
			   ~CEnsureCleanup() { Cleanup(); }

			   // Helper methods to tell if the value represents a valid object or not..
			   BOOL IsValid() const { return(m_t != tInvalid); }
			   BOOL IsInvalid() const { return(!IsValid()); }

			   // Re-assigning the object forces the current object to be cleaned-up.
			   TYPE operator=(TYPE t) 
				{ 
			      Cleanup(); 
			      m_t = static_cast<UINT_PTR>(t);
			      return(*this);  
			   }

			   // Returns the value (supports both 32-bit and 64-bit Windows).
				explicit operator TYPE() 
				{ 
					return (TYPE)m_t;
				}

				// Cleanup the object if the value represents a valid object
				void Cleanup() 
				{ 
					if (IsValid()) 
					{
						// In 64-bit Windows, all parameters are 64-bits, 
						// so no casting is required
						pfn(m_t);         // Close the object.
						m_t = tInvalid;   // We no longer represent a valid object.
					}
				}

				private:
					UINT_PTR m_t;           // The member representing the object
		};


///////////////////////////////////////////////////////////////////////////////


		// Macros to make it easier to declare instances of the template 
		// class for specific data types.

		#define MakeCleanupClass(className, tData, pfnCleanup) \
		   typedef CEnsureCleanup<tData, (PFNENSURECLEANUP) pfnCleanup> className;

		#define MakeCleanupClassX(className, tData, pfnCleanup, tInvalid) \
		   typedef CEnsureCleanup<tData, (PFNENSURECLEANUP) pfnCleanup, \
		   (INT_PTR) tInvalid> className;


///////////////////////////////////////////////////////////////////////////////


		// Instances of the template C++ class for common data types.
		MakeCleanupClass(CEnsureCloseHandle,        HANDLE,    CloseHandle);
		MakeCleanupClassX(CEnsureCloseFile,         HANDLE,    CloseHandle, 
		   INVALID_HANDLE_VALUE);
		MakeCleanupClass(CEnsureLocalFree,          HLOCAL,    LocalFree);
		MakeCleanupClass(CEnsureGlobalFree,         HGLOBAL,   GlobalFree);
		MakeCleanupClass(CEnsureRegCloseKey,        HKEY,      RegCloseKey);
		MakeCleanupClass(CEnsureCloseServiceHandle, SC_HANDLE, CloseServiceHandle);
		MakeCleanupClass(CEnsureCloseWindowStation, HWINSTA,   CloseWindowStation);
		MakeCleanupClass(CEnsureCloseDesktop,       HDESK,     CloseDesktop);
		MakeCleanupClass(CEnsureUnmapViewOfFile,    PVOID,     UnmapViewOfFile);
		MakeCleanupClass(CEnsureFreeLibrary,        HMODULE,   FreeLibrary);


///////////////////////////////////////////////////////////////////////////////


		// Special class for releasing a reserved region.
		// Special class is required because VirtualFree requires 3 parameters
		class CEnsureReleaseRegion 
		{
			public:
				explicit CEnsureReleaseRegion(PVOID pv = nullptr) : m_pv(pv) 
				{}

				CEnsureReleaseRegion(const CEnsureReleaseRegion &) = delete;
				CEnsureReleaseRegion(CEnsureReleaseRegion &&) = delete;
				const CEnsureReleaseRegion & operator = (const CEnsureReleaseRegion &) = delete;
				CEnsureReleaseRegion & operator = (CEnsureReleaseRegion &&) = delete;
				~CEnsureReleaseRegion() { Cleanup(); }

				PVOID operator=(PVOID pv) 
				{ 
					Cleanup(); 
					m_pv = pv; 
					return(m_pv); 
				}

				operator PVOID() const { return(m_pv); }
				void Cleanup() 
				{ 
					if (m_pv != nullptr) 
					{ 
						VirtualFree(m_pv, 0, MEM_RELEASE); 
						m_pv = nullptr; 
					} 
				}

			private:
				PVOID m_pv;
		};


///////////////////////////////////////////////////////////////////////////////


		// Special class for freeing a block from a heap
		// Special class is required because HeapFree requires 3 parameters
		class CEnsureHeapFree 
		{
			public:

				explicit CEnsureHeapFree(PVOID pv = nullptr, HANDLE hHeap = GetProcessHeap()) 
						: m_pv(pv), m_hHeap(hHeap)
				{}
				CEnsureHeapFree(const CEnsureHeapFree &) = delete;
				CEnsureHeapFree(CEnsureHeapFree &&) = delete;
				const CEnsureHeapFree & operator = (const CEnsureHeapFree &) = delete;
				CEnsureHeapFree & operator = (CEnsureHeapFree &&) = delete;
				~CEnsureHeapFree() { Cleanup(); }

				PVOID operator=(const PVOID pv) 
				{ 
					Cleanup(); 
					m_pv = pv; 
					return(m_pv); 
				}

				operator PVOID() const
				{
					return(m_pv);
				}

				void Cleanup() 
				{ 
					if (m_pv != nullptr) 
					{ 
						HeapFree(m_hHeap, 0, m_pv); 
						m_pv = nullptr; 
					} 
				}

			private:
				HANDLE m_hHeap;
				PVOID m_pv;
		};
	}
}
