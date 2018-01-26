/* ******************************************************************** **
** @@ PELib Source File
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  : 
** ******************************************************************** */

/* ******************************************************************** **
**                uses pre-compiled headers
** ******************************************************************** */

#include "stdafx.h"

#include "pelib.h"

/* ******************************************************************** **
** @@                   internal defines
** ******************************************************************** */

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef IMAGE_FILE_MACHINE_AMD64
#define IMAGE_FILE_MACHINE_AMD64          (0x8664)
#endif

// MakePtr is a macro that allows you to easily add to values (including
// pointers) together without dealing with C's pointer arithmetic.  It
// essentially treats the last two parameters as DWORDs.  The first
// parameter is used to typecast the result to the appropriate pointer type.
#define MakePtr(cast,ptr,addValue) (cast)((DWORD)(ptr) + (addValue))

/* ******************************************************************** **
** @@                   internal prototypes
** ******************************************************************** */

/* ******************************************************************** **
** @@                   external global variables
** ******************************************************************** */

/* ******************************************************************** **
** @@                   static global variables
** ******************************************************************** */

/* ******************************************************************** **
** @@                   real code
** ******************************************************************** */

/* ******************************************************************** **
** @@ ChkSumBlock()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update : Calculates the checksum for a chunk of data
** ******************************************************************** */

static WORD ChkSumBlock(WORD wOldChk,WORD* pWordBuf,DWORD dwSize)
{
   DWORD    dwChk = wOldChk;
   
   while (dwSize) 
   {
      DWORD    dwLen = min(dwSize,0x4000);
      
      dwSize -= dwLen;
      
      for (DWORD jj = 0; jj < dwLen; ++jj) 
      {
         dwChk += *pWordBuf++;
      }
      
      dwChk = (dwChk & 0xFFFF) + (dwChk >> 16);
   }
   
   dwChk = (dwChk & 0xFFFF) + (dwChk >> 16);
   
   return (WORD)dwChk;
}

/* ******************************************************************** **
** @@ PELib::PELib
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** ******************************************************************** */

PELib::PELib()
{
   _Reset();
}

/* ******************************************************************** **
** @@ PELib::~PELib
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  : Destructor
** ******************************************************************** */

PELib::~PELib()
{
}

/* ******************************************************************** **
** @@ PELib::_Reset()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void PELib::_Reset()
{
   _pMMF   = NULL;
   _dwSize = 0;

   _pDosHeader = NULL;
   _pNTHeader  = NULL;

   _bValid   = false;
   _bPE64    = false;
   _bOverlay = false;

   _dwStubSize = 0;
   _dwOverSize = 0;

   memset(_pszSectName,0,sizeof(_pszSectName));
}

/* ******************************************************************** **
** @@ PELib::Open
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** ******************************************************************** */

bool PELib::Open(const BYTE* const pBuf,DWORD dwSize)
{
   _bValid = false;

   if (IsBadReadPtr(pBuf,dwSize))
   {
      // Error !
      return false;
   }

   _pMMF   = (BYTE*)pBuf;
   _dwSize = dwSize;

   _pDosHeader = (PIMAGE_DOS_HEADER)_pMMF;

   if (_pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
   {
      return false;
   }
         
   _pNTHeader = MakePtr(PIMAGE_NT_HEADERS,_pDosHeader,_pDosHeader->e_lfanew);

   // First, verify that the e_lfanew field gave us a reasonable pointer, then verify the PE signature.
   if (IsBadReadPtr(_pNTHeader,sizeof(IMAGE_NT_HEADERS)) || _pNTHeader->Signature != IMAGE_NT_SIGNATURE)
   {
      return false;
   }

   // http://msdn.microsoft.com/en-us/library/ms680313%28v=vs.85%29.aspx
   if ((_pNTHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)   &&    // 0x014C   // x86
       (_pNTHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_IA64)   &&    // 0x0200   // Intel Itanium
       (_pNTHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64))       // 0x8664   // x64
   {
      return false;
   }

   // http://msdn.microsoft.com/en-us/library/ms680339%28v=vs.85%29.aspx
   if ((_pNTHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) &&    // 0x010B   // The file is an executable image.
       (_pNTHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) &&    // 0x020B   // The file is an executable image.
       (_pNTHeader->OptionalHeader.Magic != IMAGE_ROM_OPTIONAL_HDR_MAGIC))       // 0x0107   // The file is a ROM image.
   {
      return false;
   }

   _bValid = true;
   
   _bPE64 = (_pNTHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);

   _dwStubSize = 0;
   _dwOverSize = 0;

   PIMAGE_SECTION_HEADER      pSect = GetLastSection();

   ASSERT(pSect);

   if (!IsBadReadPtr(pSect,sizeof(IMAGE_SECTION_HEADER)))
   {  
      _dwStubSize = GetPointerToRawData(pSect) + pSect->SizeOfRawData;
      _dwOverSize = dwSize - _dwStubSize;
   
      _bOverlay = (_dwOverSize > 0);
   }
   
   return true;
}

/* ******************************************************************** **
** @@ PELib::Close
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** ******************************************************************** */

void PELib::Close()
{
   _Reset();
}

/* ******************************************************************** **
** @@ PELib::IsValid()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

bool PELib::IsValid()
{
   return _bValid;
}

/* ******************************************************************** **
** @@ PELib::GetHeaderSize()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::GetHeaderSize()
{
   if (_bPE64)
   {
      PIMAGE_OPTIONAL_HEADER64     pOptionalHeader64 = (PIMAGE_OPTIONAL_HEADER64)&_pNTHeader->OptionalHeader;

      ASSERT(pOptionalHeader64);

      if (!IsBadReadPtr(pOptionalHeader64,sizeof(IMAGE_OPTIONAL_HEADER64)))
      {
         return pOptionalHeader64->SizeOfHeaders;
      }
   }
   else
   {
      PIMAGE_OPTIONAL_HEADER     pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)&_pNTHeader->OptionalHeader;

      ASSERT(pOptionalHeader);

      if (!IsBadReadPtr(pOptionalHeader,sizeof(IMAGE_OPTIONAL_HEADER)))
      {
         return pOptionalHeader->SizeOfHeaders;
      }
   }

   // Error !
   return 0;
}

/* ******************************************************************** **
** @@ PELib::GetDosHeader()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

const PIMAGE_DOS_HEADER PELib::GetDosHeader()
{
   return _pDosHeader;
}

/* ******************************************************************** **
** @@ PELib::GetNTHeader()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

const PIMAGE_NT_HEADERS PELib::GetNTHeader()
{
   return _pNTHeader;
}

/* ******************************************************************** **
** @@ PELib::GetSectCnt()()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

WORD PELib::GetSectCnt()
{
   if (!_bValid)
   {
      return 0;
   }

   ASSERT(_pNTHeader);

   if (!IsBadReadPtr(_pNTHeader,sizeof(IMAGE_NT_HEADERS)))
   {
      return (WORD)_pNTHeader->FileHeader.NumberOfSections;
   }

   return 0;
}

/* ******************************************************************** **
** @@ PELib::GetSection()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

const PIMAGE_SECTION_HEADER PELib::GetSection(int iSectNum)
{
   if (!_bValid)
   {
      return NULL;
   }

   if (iSectNum > (GetSectCnt() - 1))
   {
      return NULL;
   }

   PIMAGE_SECTION_HEADER      pSect = IMAGE_FIRST_SECTION(_pNTHeader);

   ASSERT(pSect);

   if (!IsBadReadPtr(pSect,sizeof(IMAGE_SECTION_HEADER)))
   {
      return pSect + iSectNum;
   }

   // Error !
   return NULL;
}

/* ******************************************************************** **
** @@ PELib::GetSectionOffset()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::GetSectionOffset(int iSectNum)
{
   if (!_bValid)
   {
      return 0;
   }

   if (iSectNum > (GetSectCnt() - 1))
   {
      // Error !
      return 0;
   }

   PIMAGE_SECTION_HEADER      pSect = IMAGE_FIRST_SECTION(_pNTHeader);

   ASSERT(pSect);

   if (!IsBadReadPtr(pSect,sizeof(IMAGE_SECTION_HEADER)))
   {
      return GetPointerToRawData(pSect + iSectNum);
   }

   // Error !
   return 0;
}

/* ******************************************************************** **
** @@ PELib::GetSectionSize()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::GetSectionSize(int iSectNum)
{
   if (!_bValid)
   {
      return 0;
   }

   if (iSectNum > (GetSectCnt() - 1))
   {
      // Error !
      return 0;
   }

   PIMAGE_SECTION_HEADER      pSect = IMAGE_FIRST_SECTION(_pNTHeader);

   ASSERT(pSect);

   if (!IsBadReadPtr(pSect,sizeof(IMAGE_SECTION_HEADER)))
   {
      return (pSect + iSectNum)->SizeOfRawData;
   }

   // Error !
   return 0;
}

/* ******************************************************************** **
** @@ PELib::GetFirstSection()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

const PIMAGE_SECTION_HEADER PELib::GetFirstSection()
{
   return GetSection(0);
}

/* ******************************************************************** **
** @@ PELib::GetLastSection()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

const PIMAGE_SECTION_HEADER PELib::GetLastSection()
{
   return GetSection(GetSectCnt() - 1);
}

/* ******************************************************************** **
** @@ PELib::GetDOSHeaderSize()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update : 29 Oct 2009
** @  Notes  :
** ******************************************************************** */

DWORD PELib::GetDOSHeaderSize()
{
   ASSERT(_pDosHeader);

   if (!IsBadReadPtr(_pDosHeader,sizeof(IMAGE_DOS_HEADER)))
   {
      return _pDosHeader->e_cparhdr << 4;
   }

   // Error !
   return 0;
}

/* ******************************************************************** **
** @@ PELib::GetGapSize()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::GetGapSize()
{
   ASSERT(_pDosHeader);

   if (!IsBadReadPtr(_pDosHeader,sizeof(IMAGE_DOS_HEADER)))
   {
      return _pDosHeader->e_lfanew - (_pDosHeader->e_cparhdr << 4);
   }

   // Error !
   return 0;
}

/* ******************************************************************** **
** @@ PELib::GetNTHeaderStart()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::GetNTHeaderStart()
{
   ASSERT(_pDosHeader);

   if (!IsBadReadPtr(_pDosHeader,sizeof(IMAGE_DOS_HEADER)))
   {
      return _pDosHeader->e_lfanew;
   }

   // Error !
   return 0;
}

/* ******************************************************************** **
** @@ PELib::GetImageBase()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

unsigned __int64 PELib::GetImageBase()
{
   if (!_bValid)
   {
      return 0;
   }

   if (_bPE64)
   {
      PIMAGE_OPTIONAL_HEADER64      pOptionalHeader64 = (PIMAGE_OPTIONAL_HEADER64)&_pNTHeader->OptionalHeader;

      ASSERT(pOptionalHeader64);

      if (!IsBadReadPtr(pOptionalHeader64,sizeof(IMAGE_OPTIONAL_HEADER64)))
      {
         return pOptionalHeader64->ImageBase;
      }
   }
   else
   {
      PIMAGE_OPTIONAL_HEADER     pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)&_pNTHeader->OptionalHeader;

      ASSERT(pOptionalHeader);

      if (!IsBadReadPtr(pOptionalHeader,sizeof(IMAGE_OPTIONAL_HEADER64)))
      {
         return pOptionalHeader->ImageBase;
      }
   }

   // Error !
   return 0;
}

/* ******************************************************************** **
** @@ PELib::GetSectionName()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

const char* const PELib::GetSectionName(int iSectNum)
{
   *_pszSectName = 0;   // Reset

   if (!_bValid)
   {
      return NULL;
   }

   if (iSectNum > (GetSectCnt() - 1))
   {
      return NULL;
   }

   PIMAGE_SECTION_HEADER      pSect = GetSection(iSectNum);

   ASSERT(pSect);

   if (IsBadReadPtr(pSect,sizeof(IMAGE_SECTION_HEADER)))
   {
      // Error !
      return NULL;
   }

   if (pSect->Name && *pSect->Name)
   {
      strncpy(_pszSectName,(const char*)pSect->Name,8);
      _pszSectName[8] = 0;
   }

   return _pszSectName;
}


/* ******************************************************************** **
** @@ PELib::GetSectionByOffset()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

const PIMAGE_SECTION_HEADER PELib::GetSectionByOffset(DWORD dwOffset,int& iSectNum)
{        
   if (!_bValid)
   {
      return NULL;
   }
   
   iSectNum = 0;
                        
   int      iCnt = GetSectCnt();

   for (int ii = 0; ii < iCnt; ++ii)
   {
      PIMAGE_SECTION_HEADER      pSect = GetSection(ii);

      ASSERT(pSect);

      DWORD    dwRawDataPtr = GetPointerToRawData(pSect);

      DWORD    dwSectSize = pSect->SizeOfRawData  ?  pSect->SizeOfRawData  :  pSect->Misc.VirtualSize;
      
      if (!IsBadReadPtr(pSect,sizeof(IMAGE_SECTION_HEADER)))
      {
         if ((dwRawDataPtr <= dwOffset) && (dwOffset < (dwRawDataPtr + dwSectSize)))
         {
            iSectNum = ii;
            return pSect;
         }
      }
   }

   return NULL;
}

/* ******************************************************************** **
** @@ PELib::GetSectionByVA()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

const PIMAGE_SECTION_HEADER PELib::GetSectionByVA(unsigned __int64 ui64VA,int& iSectNum)
{        
   iSectNum = 0;
                        
   if (!_bValid)
   {
      return NULL;
   }

   int      iCnt = GetSectCnt();

   for (int ii = 0; ii < iCnt; ++ii)
   {
      PIMAGE_SECTION_HEADER      pSection = GetSection(ii);

      ASSERT(pSection);

      if (!IsBadReadPtr(pSection,sizeof(IMAGE_SECTION_HEADER)))
      {
         // Shit! pSection->Misc.VirtualSize is 0 for Sybase Watcom
         DWORD    dwVirtualSize = pSection->Misc.VirtualSize  ?  pSection->Misc.VirtualSize  :  pSection->SizeOfRawData;

         if (((GetImageBase() + pSection->VirtualAddress) <= ui64VA) && (ui64VA < (GetImageBase() + pSection->VirtualAddress + dwVirtualSize)))
         {
            iSectNum = ii;
            return pSection;
         }
      }
   }

   return NULL;
}

/* ******************************************************************** **
** @@ PELib::GetEntryPointOffset()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::GetEntryPointOffset(int& riSectNum)
{  
   DWORD                dwEntryRVA  = 0;
   unsigned __int64     ui64EntryVA = 0;

   if (_bPE64)
   {
      PIMAGE_OPTIONAL_HEADER64     pOptionalHeader = (PIMAGE_OPTIONAL_HEADER64)&_pNTHeader->OptionalHeader;

      dwEntryRVA  = pOptionalHeader->AddressOfEntryPoint;
      ui64EntryVA = pOptionalHeader->ImageBase + dwEntryRVA;
   }
   else
   {
      PIMAGE_OPTIONAL_HEADER     pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)&_pNTHeader->OptionalHeader;

      dwEntryRVA  = pOptionalHeader->AddressOfEntryPoint;
      ui64EntryVA = pOptionalHeader->ImageBase + dwEntryRVA;
   }

   riSectNum = 0;

   PIMAGE_SECTION_HEADER      pSect = GetSectionByVA(ui64EntryVA,riSectNum);

   ASSERT(pSect);

   if (IsBadReadPtr(pSect,sizeof(IMAGE_SECTION_HEADER)))
   {
      // Out of File !
      return 0;   
   }

   ++riSectNum;

   return dwEntryRVA - pSect->VirtualAddress + GetPointerToRawData(pSect);
}

/* ******************************************************************** **
** @@ PELib::GetAddressOfEntryPoint()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::GetAddressOfEntryPoint()
{
   if (!_bValid)
   {
      return (DWORD)-1;
   }

   if (_bPE64)
   {
      PIMAGE_OPTIONAL_HEADER64     pOptionalHeader64 = (PIMAGE_OPTIONAL_HEADER64)&_pNTHeader->OptionalHeader;

      ASSERT(pOptionalHeader64);

      if (!IsBadReadPtr(pOptionalHeader64,sizeof(IMAGE_OPTIONAL_HEADER64)))
      {  
         return pOptionalHeader64->AddressOfEntryPoint;
      }
   }
   else
   {
      PIMAGE_OPTIONAL_HEADER     pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)&_pNTHeader->OptionalHeader;

      ASSERT(pOptionalHeader);

      if (!IsBadReadPtr(pOptionalHeader,sizeof(IMAGE_OPTIONAL_HEADER)))
      {
         return pOptionalHeader->AddressOfEntryPoint;
      }
   }

   return (DWORD)-1;
}

/* ******************************************************************** **
** @@ PELib::GetEntryPointVA()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

unsigned __int64 PELib::GetEntryPointVA()
{
   if (!_bValid)
   {
      return 0;
   }

   DWORD    dwEntryRVA = 0;

   if (_bPE64)
   {
      PIMAGE_OPTIONAL_HEADER64     pOptionalHeader64 = (PIMAGE_OPTIONAL_HEADER64)&_pNTHeader->OptionalHeader;
   
      ASSERT(pOptionalHeader64);

      if (!IsBadReadPtr(pOptionalHeader64,sizeof(IMAGE_OPTIONAL_HEADER64)))
      {
         dwEntryRVA = GetAddressOfEntryPoint();
   
         return (dwEntryRVA != (DWORD)-1)  ?  pOptionalHeader64->ImageBase + dwEntryRVA  :  0;
      }
   }
   else
   {
      PIMAGE_OPTIONAL_HEADER     pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)&_pNTHeader->OptionalHeader;
   
      ASSERT(pOptionalHeader);

      if (!IsBadReadPtr(pOptionalHeader,sizeof(IMAGE_OPTIONAL_HEADER)))
      {
         dwEntryRVA = GetAddressOfEntryPoint();
   
         return (dwEntryRVA  != (DWORD)-1)  ?  pOptionalHeader->ImageBase + dwEntryRVA  :  0;
      }
   }

   // Error !
   return 0;
}

/* ******************************************************************** **
** @@ PELib::GetEntryPointSectNum()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

int PELib::GetEntryPointSectNum()
{
   if (!_bValid)
   {
      return 0;
   }

   int      iSectNum   = 0;
   DWORD    dwEntryRVA = 0;

   if (_bPE64)
   {
      PIMAGE_OPTIONAL_HEADER64     pOptionalHeader64 = (PIMAGE_OPTIONAL_HEADER64)&_pNTHeader->OptionalHeader;

      ASSERT(pOptionalHeader64);

      if (!IsBadReadPtr(pOptionalHeader64,sizeof(IMAGE_OPTIONAL_HEADER64)))
      {
         dwEntryRVA = pOptionalHeader64->AddressOfEntryPoint;
      }
   }
   else
   {
      PIMAGE_OPTIONAL_HEADER     pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)&_pNTHeader->OptionalHeader;

      ASSERT(pOptionalHeader);

      if (!IsBadReadPtr(pOptionalHeader,sizeof(IMAGE_OPTIONAL_HEADER)))
      {
         dwEntryRVA = pOptionalHeader->AddressOfEntryPoint;
      }
   }

   PIMAGE_SECTION_HEADER      pSection = GetSectionByOffset(dwEntryRVA,iSectNum);

   ASSERT(pSection);

   if (IsBadReadPtr(pSection,sizeof(IMAGE_SECTION_HEADER)))
   {
      // Error !
      return 0;
   }

   return iSectNum;
}

/* ******************************************************************** **
** @@ PELib::OFS2VA()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

unsigned __int64 PELib::OFS2VA(DWORD dwOfs)
{
   if (!_bValid)
   {
      return 0;
   }

   int      iSectNum = 0;
   
   PIMAGE_SECTION_HEADER      pSect = GetSectionByOffset(dwOfs,iSectNum);
   
   ASSERT(pSect);

   if (IsBadReadPtr(pSect,sizeof(PIMAGE_SECTION_HEADER)))
   {
      // Error !
      return 0;
   }

   return GetImageBase() + pSect->VirtualAddress - GetPointerToRawData(pSect) + dwOfs;
}

/* ******************************************************************** **
** @@ PELib::OFS2RVA()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::OFS2RVA(DWORD dwOfs)
{
   if (!_bValid)
   {
      return 0;
   }

   int      iSectNum = 0;
   
   PIMAGE_SECTION_HEADER      pSect = GetSectionByOffset(dwOfs,iSectNum);
   
   ASSERT(pSect);

   if (IsBadReadPtr(pSect,sizeof(PIMAGE_SECTION_HEADER)))
   {
      // Error !
      return 0;
   }

   return pSect->VirtualAddress - GetPointerToRawData(pSect) + dwOfs;
}

/* ******************************************************************** **
** @@ PELib::VA2OFS()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::VA2OFS(unsigned __int64 ui64VA)
{
   if (!_bValid)
   {
      return 0;
   }

   int      iSectNum = 0;
   
   PIMAGE_SECTION_HEADER      pSect = GetSectionByVA(ui64VA,iSectNum);
   
   ASSERT(pSect);

   if (IsBadReadPtr(pSect,sizeof(PIMAGE_SECTION_HEADER)))
   {
      // Error !
      return 0;
   }

   unsigned __int64     uiSectStartVA = GetImageBase() + pSect->VirtualAddress;
   
   return (DWORD)(ui64VA - uiSectStartVA + GetPointerToRawData(pSect));
}

/* ******************************************************************** **
** @@ PELib::GetSectStartVA()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

const PIMAGE_SECTION_HEADER PELib::GetSectStartVA(int iSectNum,unsigned __int64& rui64VA)  
{
   rui64VA = 0;
   
   PIMAGE_SECTION_HEADER      pSect = GetSection(iSectNum);

   ASSERT(pSect);

   if (IsBadReadPtr(pSect,sizeof(PIMAGE_SECTION_HEADER)))
   {
      // Error !
      return NULL;
   }
   
   rui64VA = GetImageBase() + pSect->VirtualAddress;

   return pSect;
}

/* ******************************************************************** **
** @@ PELib::HaveOverlay()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

bool PELib::HaveOverlay()
{ 
   return _bOverlay; 
}

/* ******************************************************************** **
** @@ PELib::GetOverlayOfs()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::GetOverlayOfs()
{
   return _dwStubSize;
}

/* ******************************************************************** **
** @@ PELib::GetOverlaySize()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::GetOverlaySize()
{
   return _dwOverSize;
}

/* ******************************************************************** **
** @@ PELib::CalcChkSum()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::FileChkSum()
{
   if (!_bValid)
   {
      return 0;
   }

   return _pNTHeader->OptionalHeader.CheckSum;
}

/* ******************************************************************** **
** @@ PELib::CalcChkSum()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::CalcChkSum()
{
   if (!_bValid)
   {
      return 0;
   }

   DWORD    dwCheckSumCur = _pNTHeader->OptionalHeader.CheckSum;

   DWORD    dwCheck = ChkSumBlock(0,(WORD*)_pMMF,_dwSize / 2);
  
   if (_dwSize & 1) 
   { 
      // We have to handle an extra byte if it exists
      dwCheck += _pMMF[_dwSize - 1];
      dwCheck  = (dwCheck >> 16) + (dwCheck & 0xFFFF);
   }

   // Adjust the checksum
   DWORD    dwResult = ((dwCheck - 1 < dwCheckSumCur) ? (dwCheck - 1) : dwCheck) - dwCheckSumCur;

   dwResult = (dwResult & 0xFFFF) + (dwResult >> 16);
   dwResult = (dwResult & 0xFFFF) + (dwResult >> 16);
   
   dwResult += _dwSize;

   return dwResult;
}

/* ******************************************************************** **
** @@ PELib::GetPointerToRawData()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

DWORD PELib::GetPointerToRawData(const PIMAGE_SECTION_HEADER pSect)
{
   if (!_bValid)
   {
      return 0;
   }

   DWORD    dwRawDataPtr = pSect->PointerToRawData;

   // PointerToRawData
   // The file pointer to the first page of the section within the COFF file. For 
   // executable images, this must be a multiple of FileAlignment from the optional 
   // header. For object files, the value should be aligned on a 4-byte boundary for 
   // best performance. When a section contains only uninitialized data, this field 
   // should be zero.
   DWORD    dwFileAlignment = 0;

   if (_bPE64)
   {
      PIMAGE_OPTIONAL_HEADER64     pOptionalHeader64 = (PIMAGE_OPTIONAL_HEADER64)&_pNTHeader->OptionalHeader;

      ASSERT(pOptionalHeader64);

      if (!IsBadReadPtr(pOptionalHeader64,sizeof(IMAGE_OPTIONAL_HEADER64)))
      {
         dwFileAlignment = pOptionalHeader64->FileAlignment;
      }
   }
   else
   {
      PIMAGE_OPTIONAL_HEADER     pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)&_pNTHeader->OptionalHeader;

      ASSERT(pOptionalHeader);

      if (!IsBadReadPtr(pOptionalHeader,sizeof(IMAGE_OPTIONAL_HEADER)))
      {
         dwFileAlignment = pOptionalHeader->FileAlignment;
      }
   }

   // Shit! pSect->PointerToRawData is Invalid for (Win)Upack - should be 0 !
   dwRawDataPtr /= dwFileAlignment; 
   dwRawDataPtr *= dwFileAlignment;

   return dwRawDataPtr;
}

/* ******************************************************************** **
** @@                   End of File
** ******************************************************************** */

