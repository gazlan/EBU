/* ******************************************************************** **
** @@ PELib Header File
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

#ifndef _PELIB_HPP_
#define _PELIB_HPP_

#if _MSC_VER > 1000
#pragma once
#endif

/* ******************************************************************** **
** @@ internal defines
** ******************************************************************** */

#define MAX_SECT_NAME_LEN     (8)

/* ******************************************************************** **
** @@ internal prototypes
** ******************************************************************** */

/* ******************************************************************** **
** @@ external global variables
** ******************************************************************** */

/* ******************************************************************** **
** @@ static global variables
** ******************************************************************** */

/* ******************************************************************** **
** @@ Global Function Prototypes
** ******************************************************************** */

/* ******************************************************************** **
** @@ PELib
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

class PELib
{
   private:

      BYTE*                      _pMMF;
      DWORD                      _dwSize;
      bool                       _bValid;
      bool                       _bPE64;
      bool                       _bOverlay;
      DWORD                      _dwStubSize;
      DWORD                      _dwOverSize;
      char                       _pszSectName[MAX_SECT_NAME_LEN + 1];
      PIMAGE_DOS_HEADER          _pDosHeader;
      PIMAGE_NT_HEADERS          _pNTHeader;

   public:

               PELib();
      virtual ~PELib();

      bool                          Open(const BYTE* const pBuf,DWORD dwSize);
      void                          Close();
      bool                          IsValid();
                                    
      bool                          HaveOverlay();
      DWORD                         GetOverlayOfs();
      DWORD                         GetOverlaySize();
                                    
      DWORD                         FileChkSum();
      DWORD                         CalcChkSum();

      DWORD                         GetDOSHeaderSize();
      DWORD                         GetGapSize();
      DWORD                         GetNTHeaderStart();
      unsigned __int64              GetImageBase();
      DWORD                         GetAddressOfEntryPoint();
      unsigned __int64              GetEntryPointVA();
      DWORD                         GetEntryPointOffset(int& riSectNum);
      unsigned __int64              OFS2VA(DWORD dwOfs);
      DWORD                         VA2OFS(unsigned __int64 ui64VA);
      DWORD                         OFS2RVA(DWORD dwOfs);
      int                           GetEntryPointSectNum();
      const char* const             GetSectionName(int iSectNum);
      DWORD                         GetHeaderSize();
      const PIMAGE_DOS_HEADER       GetDosHeader();
      const PIMAGE_NT_HEADERS       GetNTHeader();
      const PIMAGE_SECTION_HEADER   GetSection(int iSectNum);
      WORD                          GetSectCnt();
      DWORD                         GetPointerToRawData(const PIMAGE_SECTION_HEADER pSect);
      DWORD                         GetSectionOffset(int iSectNum);
      DWORD                         GetSectionSize(int iSectNum);
      const PIMAGE_SECTION_HEADER   GetFirstSection();
      const PIMAGE_SECTION_HEADER   GetLastSection();  
      const PIMAGE_SECTION_HEADER   GetSectionByVA(unsigned __int64 ui64VA,int& iSectNum);
      const PIMAGE_SECTION_HEADER   GetSectionByOffset(DWORD dwOfs,int& iSectNum);
      const PIMAGE_SECTION_HEADER   GetSectStartVA(int iSectNum,unsigned __int64& rui64VA);

   private:

      void  _Reset();         
};

/* ******************************************************************** **
** @@                   The End
** ******************************************************************** */

#endif
