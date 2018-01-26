/* ******************************************************************** **
** @@ E_Unpacker_EEP
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Modify : 
** @  Notes  :
** ******************************************************************** */

/* ******************************************************************** **
** uses pre-compiled headers
** ******************************************************************** */

#include "stdafx.h"

#include <math.h>

#include "..\shared\hash_md5.h"
#include "..\shared\search_quick.h"
#include "..\shared\file.h"
#include "..\shared\text.h"
#include "..\shared\pack_zlib_zip.h"
#include "..\shared\pack_zlib_zconf.h"
#include "..\shared\pack_zlib.h"

#include "e_unp.h"
#include "e_unp_eep.h"

/* ******************************************************************** **
** @@ internal defines
** ******************************************************************** */

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  TITLE_OFS                  (0x15)
#define  TITLE_OFS_3_34             (0x16)

#define  FLAG_0                     (0x00000001 << (0 << 2))
#define  FLAG_1                     (0x00000001 << (1 << 2))
#define  FLAG_2                     (0x00000001 << (2 << 2))
#define  FLAG_3                     (0x00000001 << (3 << 2))
#define  FLAG_4                     (0x00000001 << (4 << 2))
#define  FLAG_5                     (0x00000001 << (5 << 2))
#define  FLAG_6                     (0x00000001 << (6 << 2))
#define  FLAG_7                     (0x00000001 << (7 << 2))

static EEP_PROTECTIONS     _SecurityType = EEP_PR_NONE;

char        E_Unpacker_EEP::_pszHash3[HASH_MD5_SIZE + 1];
EEP_VERSION E_Unpacker_EEP::_EEP_Version = EEP_VER_3_21;

const char* pEEP_Protections[4] =
{
   "None",
   "Password",
   "Username + Serial",
   "Unlock Key"
};

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
** @@ real code
** ******************************************************************** */

/* ******************************************************************** **
** @@ DecodeString()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static const char* const DecodeString(const BYTE* const pString,BYTE bySize)
{
   if (!bySize)
   {
      // Error !
      return NULL;
   }

   static char    pszString[MAX_PATH + 1];
   
   memset(pszString,0,sizeof(pszString));

   memcpy(pszString,pString,bySize);   // Always secure!

   bool     bPrintable = true;

   *pszString ^= bySize;
   
   if (!IsPrintable(*pszString))
   {
      // Error !
      return NULL;
   }

   for (int ii = 1; ii < (int)bySize; ++ii)
   {
      pszString[ii] ^= pszString[ii - 1];

      bPrintable = IsPrintable((BYTE)pszString[ii])  ?  bPrintable  :  false;
   }

   return bPrintable  ?  pszString  :  NULL;
}

/* ******************************************************************** **
** @@ GetTitle()
** @  Copyrt : 
** @  Author : 
** @  Update :
** @  Update :
** @  Notes  :
** ******************************************************************** */

int E_Unpacker_EEP::GetTitle()
{
   memset(_pszTitle,0,sizeof(_pszTitle));

   // Use offset correction for 3.34+
   BYTE*    pTitle = _pBuf + ((_EEP_Version == EEP_VER_3_34)  ?  TITLE_OFS_3_34  :  TITLE_OFS);

   DWORD    dwSize = *(DWORD*)pTitle;

   if (dwSize)
   {
      const char*    pszString = DecodeString(pTitle + sizeof(dwSize),(BYTE)dwSize);

      if (pszString && *pszString)
      {
         strcpy(_pszTitle,pszString);
         return sizeof(DWORD) + dwSize;
      }
      else
      {
         return sizeof(DWORD);
      }
   }

   return sizeof(DWORD);
}

/* ******************************************************************** **
** @@ E_Unpacker_EEP::CalcHash1()
** @  Copyrt : 
** @  Author : 
** @  Update :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void E_Unpacker_EEP::CalcHash1()
{
   memset(_pszHash1,0,sizeof(_pszHash1));

   _pNext = _pBuf;

   // eBook ID
   // DWORD dwBookID = *(DWORD*)pNext;

   _pNext += sizeof(DWORD);

   // OLETIME - 64-bit value representing date and time in OLE and Delphi using the format 'MM/dd/yyyy hh:mm:ss'. 
   // OLETIME is a 64-bit double representing the number of days since 12/30/1899 12:00 AM.
   double      TimeStamp = *(double*)_pNext;

   _pNext += sizeof(TimeStamp);

   // Use offset correction for 3.34+
   _pNext += (_EEP_Version == EEP_VER_3_34)  ?  1  :  0;

   DWORD       dwWinHeight = *(DWORD*)_pNext;

   _pNext += sizeof(dwWinHeight);

   DWORD       dwWinWidth = *(DWORD*)_pNext;

   _pNext += sizeof(dwWinWidth);

   BYTE        byState = *_pNext;

   _pNext += sizeof(BYTE);

   double      fInt = 0.0;

   double      fFrac = modf(TimeStamp,&fInt);

   int         iTimeStamp = 0;

   // Fucking Pascal Emulation !
   if (fFrac < 0.5)
   {
      iTimeStamp = (int)fInt;
   }
   else if (fFrac > 0.5)
   {
      iTimeStamp = (int)fInt + 1;
   }
   else
   {
      iTimeStamp = (int)fInt;

      if (iTimeStamp % 2)
      {
         ++iTimeStamp;
      }
   }

   char     pszTemp[MAX_PATH + 1];

   sprintf(pszTemp,"%d%d%d%ld",byState,dwWinWidth,dwWinHeight,iTimeStamp);

   MD5      _Hash;

   _Hash.Reset();
   _Hash.Update((BYTE*)pszTemp,strlen(pszTemp));
   _Hash.Finalize();

   strcpy(_pszHash1,_Hash.Signature());
}

/* ******************************************************************** **
** @@ E_Unpacker_EEP::CalcHash2()
** @  Copyrt : 
** @  Author : 
** @  Update :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void E_Unpacker_EEP::CalcHash2()
{
   memset(_pszHash2,0,sizeof(_pszHash2));
   
   _pNext += GetTitle();

   char     pszBackColor[MAX_PATH + 1];
   char     pszForeColor[MAX_PATH + 1];
   char     pszReferer  [MAX_PATH + 1];

   memset(pszBackColor,0,sizeof(pszBackColor));
   memset(pszForeColor,0,sizeof(pszForeColor));
   memset(pszReferer,  0,sizeof(pszReferer));

   DWORD    dwSize = 0;
   
   if (IsBadReadPtr(_pNext,sizeof(DWORD)))
   {
      // Error !
      fprintf(E_Unpacker::_pOut,"\nErr: Can't read at adr %p [%d]\n",_pNext,__LINE__);
      return;
   }
         
   dwSize = *(DWORD*)_pNext;

   if (dwSize)
   {
      const char*    pszString = DecodeString(_pNext + sizeof(dwSize),(BYTE)dwSize);

      if (pszString && *pszString)
      {
         strcpy(pszBackColor,pszString);
         _pNext += sizeof(DWORD) + dwSize;
      }
      else
      {
         _pNext += sizeof(DWORD);
      }
   }
   else
   {
      _pNext += sizeof(DWORD);
   }

   if (IsBadReadPtr(_pNext,sizeof(DWORD)))
   {
      // Error !
      fprintf(E_Unpacker::_pOut,"\nErr: Can't read at adr %p [%d]\n",_pNext,__LINE__);
      return;
   }

   dwSize = *(DWORD*)_pNext;

   if (dwSize)
   {
      const char*    pszString = DecodeString(_pNext + sizeof(dwSize),(BYTE)dwSize);

      if (pszString && *pszString)
      {
         strcpy(pszForeColor,pszString);
         _pNext += sizeof(DWORD) + dwSize;
      }
      else
      {
         _pNext += sizeof(DWORD);
      }
   }
   else
   {
      _pNext += sizeof(DWORD);
   }

   if (IsBadReadPtr(_pNext,sizeof(DWORD)))
   {
      // Error !
      fprintf(E_Unpacker::_pOut,"\nErr: Can't read at adr %p [%d]\n",_pNext,__LINE__);
      return;
   }

   dwSize = *(DWORD*)_pNext;

   if (dwSize)
   {
      const char*    pszString = DecodeString(_pNext + sizeof(dwSize),(BYTE)dwSize);

      if (pszString && *pszString)
      {
         strcpy(pszReferer,pszString);
         _pNext += sizeof(DWORD) + dwSize;
      }
      else
      {
         _pNext += sizeof(DWORD);
      }
   }
   else
   {
      _pNext += sizeof(DWORD);
   }

   char     pszTemp[MAX_PATH + 1];

   sprintf(pszTemp,"%s%s%s%s",strlwr(_pszHash1),pszBackColor,pszForeColor,_pszTitle);

   MD5      _Hash;

   _Hash.Reset();
   _Hash.Update((BYTE*)pszTemp,strlen(pszTemp));
   _Hash.Finalize();

   strcpy(_pszHash2,_Hash.Signature());
}

/* ******************************************************************** **
** @@ E_Unpacker_EEP::CalcHash3()
** @  Copyrt : 
** @  Author : 
** @  Update :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void E_Unpacker_EEP::CalcHash3()
{
   memset(_pszHash3,0,sizeof(_pszHash3));

   memset(_pszUserName, 0,sizeof(_pszUserName));
   memset(_pszSerialNum,0,sizeof(_pszSerialNum));
   memset(_pszReferer,  0,sizeof(_pszReferer));

   DWORD    dwSize = 0;
   
   if (IsBadReadPtr(_pNext,sizeof(DWORD)))
   {
      // Error !
      fprintf(E_Unpacker::_pOut,"\nErr: Can't read at adr %p [%d]\n",_pNext,__LINE__);
      return;
   }

   dwSize = *(DWORD*)_pNext;

   if (dwSize)
   {
      const char*    pszString = DecodeString(_pNext + sizeof(dwSize),(BYTE)dwSize);

      if (pszString && *pszString)
      {
         strcpy(_pszUserName,pszString);
         _pNext += sizeof(DWORD) + dwSize;
      }
      else
      {
         _pNext += sizeof(DWORD);
      }
   }
   else
   {
      _pNext += sizeof(DWORD);
   }

   if (IsBadReadPtr(_pNext,sizeof(DWORD)))
   {
      // Error !
      fprintf(E_Unpacker::_pOut,"\nErr: Can't read at adr %p [%d]\n",_pNext,__LINE__);
      return;
   }

   if (IsBadReadPtr(_pNext,sizeof(DWORD)))
   {
      // Error !
      fprintf(E_Unpacker::_pOut,"\nErr: Can't read at adr %p [%d]\n",_pNext,__LINE__);
      return;
   }

   dwSize = *(DWORD*)_pNext;

   if (dwSize)
   {
      const char*    pszString = DecodeString(_pNext + sizeof(dwSize),(BYTE)dwSize);

      if (pszString && *pszString)
      {
         strcpy(_pszSerialNum,pszString);
         _pNext += sizeof(DWORD) + dwSize;
      }
      else
      {
         _pNext += sizeof(DWORD);
      }
   }
   else
   {
      _pNext += sizeof(DWORD);
   }

   if (IsBadReadPtr(_pNext,sizeof(DWORD)))
   {
      // Error !
      fprintf(E_Unpacker::_pOut,"\nErr: Can't read at adr %p [%d]\n",_pNext,__LINE__);
      return;
   }

   dwSize = *(DWORD*)_pNext;

   if (dwSize)
   {
      const char*    pszString = DecodeString(_pNext + sizeof(dwSize),(BYTE)dwSize);

      if (pszString && *pszString)
      {
         strcpy(_pszReferer,pszString);
         _pNext += sizeof(DWORD) + dwSize;
      }
      else
      {
         _pNext += sizeof(DWORD);
      }
   }
   else
   {
      _pNext += sizeof(DWORD);
   }

   char     pszTemp[MAX_PATH + 1];

   sprintf(pszTemp,"%s%s",strlwr(_pszHash2),_pszUserName);

   MD5      _Hash;

   _Hash.Reset();
   _Hash.Update((BYTE*)pszTemp,strlen(pszTemp));
   _Hash.Finalize();

   strcpy(_pszHash3,_Hash.Signature());
}

/* ******************************************************************** **
** @@ E_Unpacker_EEP::Decrypt()
** @  Copyrt : 
** @  Author : 
** @  Update :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void E_Unpacker_EEP::Decrypt(BYTE* pBuf,int iSize)
{
   CString     sHash = _pszHash3;

   for (int ii = 0; ii < iSize; ++ii)
   {
      int      iRest = iSize - ii;
      int      iPos  = iRest % HASH_MD5_SIZE;

      CString     sAL = sHash.Mid(iPos - 1,2);

      BYTE     _AL = (BYTE)strtol((LPCTSTR)sAL,0,0x10);

      DWORD    _EDX = ii;

      _EDX &= 0x80000003;

      if (_EDX & 0x80000000)
      {
         --_EDX;
         _EDX |= 0xFFFFFFFC;
         ++_EDX;
      }

      DWORD    _ECX = ii + ii * 2;

      _ECX &= 0x80000003;

      if (_ECX & 0x80000000)
      {
         --_ECX;
         _ECX |= 0xFFFFFFFC;
         ++_ECX;
      }

      BYTE     _DL = (BYTE)(_EDX & 0xFF);
      BYTE     _CL = (BYTE)(_ECX & 0xFF);

      _DL = (BYTE)(_DL - _CL);

      _DL ^= pBuf[ii];

      _AL ^= _DL;

      pBuf[ii] = _AL;
   }
}

/* ******************************************************************** **
** @@ E_Unpacker_EEP::GuardFinder_32()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void __fastcall GuardFinder_32(void* pParam,DWORD dwFoundAt)
{
   BYTE*    pBuf = (BYTE*)pParam + dwFoundAt;

   int      iStrSize = *(int*)pBuf;

   const char*    pszString = DecodeString(pBuf + sizeof(iStrSize),(BYTE)iStrSize);

   if (!pszString || !*pszString)
   {
      // Error !
      return;
   }

   if (!strcmp(pszString,"sword"))
   {
      _SecurityType = (EEP_PROTECTIONS)*(BYTE*)(pBuf - 2);
   }
}

/* ******************************************************************** **
** @@ GuardFinder_33()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void __fastcall GuardFinder_33(void* pParam,DWORD dwFoundAt)
{
   BYTE*    pBuf = (BYTE*)pParam + dwFoundAt;

   int      iStrSize = *(int*)pBuf;

   const char*    pszString = DecodeString(pBuf + sizeof(iStrSize),(BYTE)iStrSize);

   if (!pszString || !*pszString)
   {
      // Error !
      return;
   }

   if (!strcmp(pszString,"drowssap"))
   {
      _SecurityType = (EEP_PROTECTIONS)*(BYTE*)(pBuf - 2);
   }
}

/* ******************************************************************** **
** @@ E_Unpacker_EEP::GetProtection_32x()
** @  Copyrt : 
** @  Author : 
** @  Update :
** @  Update :
** @  Notes  :
** ******************************************************************** */

int E_Unpacker_EEP::GetProtection_32x()
{
   _SecurityType = EEP_PR_NONE;

   DWORD    dwPattern = 0x00000005;

   QuickSearch((BYTE*)&dwPattern,sizeof(DWORD),_pStart,_dwSizeOvr,_pStart,GuardFinder_32);

   return _SecurityType;
}

/* ******************************************************************** **
** @@ E_Unpacker_EEP::GetProtection_33x()
** @  Copyrt : 
** @  Author : 
** @  Update :
** @  Update :
** @  Notes  :
** ******************************************************************** */

int E_Unpacker_EEP::GetProtection_33x()
{
   _SecurityType = EEP_PR_NONE;

   DWORD    dwPattern = 0x00000008;

   QuickSearch((BYTE*)&dwPattern,sizeof(DWORD),_pStart,_dwSizeOvr,_pStart,GuardFinder_33);

   return _SecurityType;
}

/* ******************************************************************** **
** @@ E_Unpacker_EEP::GetProtectionType()
** @  Copyrt : 
** @  Author : 
** @  Update :
** @  Update :
** @  Notes  :
** ******************************************************************** */

const char* const E_Unpacker_EEP::GetProtectionType()
{
   if (_EEP_Version == EEP_VER_3_21)
   {
      return pEEP_Protections[GetProtection_32x()];
   }
   else if (_EEP_Version == EEP_VER_3_31)
   {
      return pEEP_Protections[GetProtection_33x()];
   }
   else
   {
      return "Unknown";  // TODO !!
   }
}

/* ******************************************************************** **
** @@ QS_Finder()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void __fastcall QS_Finder(void* pParam,DWORD dwFoundAt)
{
   BYTE*    pBuf = (BYTE*)pParam + dwFoundAt - 1;

   int      iNameLen = *(int*)pBuf;

   const char*    pszName = DecodeString(pBuf + sizeof(iNameLen),(BYTE)iNameLen);

   if (!pszName || !*pszName)
   {
      // Error !
      return;
   }

   DWORD    dwOfs      = E_Unpacker::_dwSizeStub + dwFoundAt - 1;
   BYTE*    pParams    = pBuf + sizeof(iNameLen) + strlen(pszName);

   DWORD    dwZipSize  = *(DWORD*) pParams;
   DWORD    dwFlags    = *(DWORD*)(pParams + sizeof(dwZipSize));

   BYTE*    pTempBuf   = NULL;
   BYTE*    pZipStart  = NULL;
   DWORD    dwZipStart = 0;
   DWORD    dwMethod   = 0;

   if (E_Unpacker_EEP::_EEP_Version == EEP_VER_3_34)
   {
      dwZipStart = (dwFoundAt - 1) + sizeof(iNameLen) + strlen(pszName) + sizeof(dwFlags) + 3;
      pZipStart  = pParams + sizeof(dwZipSize) + sizeof(dwFlags) + 3;
      
      if ((dwFlags == FLAG_4) || (dwFlags == (FLAG_2 | FLAG_4)))
      {
         pTempBuf = new BYTE[dwZipSize];

         if (!pTempBuf)
         {
            // Error !
            fprintf(E_Unpacker::_pOut,"\nErr: Not enough memory!\n");
            return;
         }      

         memcpy(pTempBuf,pZipStart,dwZipSize);

         E_Unpacker_EEP::Decrypt(pTempBuf,dwZipSize);
         
         dwMethod = *(WORD*)pTempBuf;
      }
   }
   else if (E_Unpacker_EEP::_EEP_Version == EEP_VER_3_31)
   {
      dwZipStart = (dwFoundAt - 1) + sizeof(iNameLen) + strlen(pszName) + sizeof(dwFlags) + 2;
      pZipStart  = pParams + sizeof(dwZipSize) + sizeof(dwFlags) + 2;
   
      if ((dwFlags == FLAG_2) || (dwFlags == (FLAG_0 | FLAG_2)))
      {
         pTempBuf = new BYTE[dwZipSize];

         if (!pTempBuf)
         {
            // Error !
            fprintf(E_Unpacker::_pOut,"\nErr: Not enough memory!\n");
            return;
         }      

         memcpy(pTempBuf,pZipStart,dwZipSize);

         E_Unpacker_EEP::Decrypt(pTempBuf,dwZipSize);

         dwMethod = *(WORD*)pTempBuf;
      }
   }
   else if (E_Unpacker_EEP::_EEP_Version == EEP_VER_3_21)
   {
      dwZipStart = (dwFoundAt - 1) + sizeof(iNameLen) + strlen(pszName) + sizeof(dwFlags) + 2;
      pZipStart  = pParams + sizeof(dwZipSize) + sizeof(dwFlags) + 2;

      dwMethod = *(WORD*)pZipStart;
   }
   else
   {
      // Do Nothing !
   }   
   
   if (dwMethod == ZIP_STREAM_BEST)
   {
      fprintf(E_Unpacker::_pOut,"Z  %s  %08X  [%08X]  %08X  (%08X)  %08X  %04X  %02X  [%3d]  %s\n","<Best>", dwOfs,dwFoundAt - 1,dwZipSize,dwZipStart,dwFlags,dwMethod,iNameLen,iNameLen,pszName);
   }
   else if (dwMethod == ZIP_STREAM_GOOD)
   {
      fprintf(E_Unpacker::_pOut,"Z  %s  %08X  [%08X]  %08X  (%08X)  %08X  %04X  %02X  [%3d]  %s\n","<Good>", dwOfs,dwFoundAt - 1,dwZipSize,dwZipStart,dwFlags,dwMethod,iNameLen,iNameLen,pszName);
   }
   else if (dwMethod == ZIP_STREAM_FAST)
   {
      fprintf(E_Unpacker::_pOut,"Z  %s  %08X  [%08X]  %08X  (%08X)  %08X  %04X  %02X  [%3d]  %s\n","<Fast>", dwOfs,dwFoundAt - 1,dwZipSize,dwZipStart,dwFlags,dwMethod,iNameLen,iNameLen,pszName);
   }
   else
   {
      fprintf(E_Unpacker::_pOut,"           %08X  [%08X]  %08X  (%08X)  %08X        %02X  [%3d]  %s\n",dwOfs,dwFoundAt - 1,dwZipSize,dwZipStart,dwFlags,iNameLen,iNameLen,pszName);
   }

   if (
         (dwMethod == ZIP_STREAM_BEST) ||
         (dwMethod == ZIP_STREAM_GOOD) ||
         (dwMethod == ZIP_STREAM_FAST)
      )
   {
      char     pszPath[MAX_PATH + 1];

      strcpy(pszPath,E_Unpacker::_pszBook);

      char*    pBackSlash = strrchr(pszName,'\\');
      char     pszNaked[MAX_PATH + 1]; 
      memset(pszNaked,0,sizeof(pszNaked));

      if (pBackSlash)
      {
         strncpy(pszNaked,pBackSlash + 1,MAX_PATH);
         pszNaked[MAX_PATH] = 0;    // ASCIIZ

         *pBackSlash = 0;  // Remove FileName

         strcat(pszPath,"\\");
         strcat(pszPath,pszName);

         EnsureDirectory(pszPath);

         strcat(pszPath,"\\");
         strcat(pszPath,pszNaked);
      }
      else
      {
         strcat(pszPath,"\\");
         strcat(pszPath,pszName);
         // For LOG !
         strcpy(pszNaked,pszName);
      }

      // Dump ZIP Stream
      FILE*    pOutput = fopen(pszPath,"wb");

      if (!pOutput)
      {
         // Error !
         fprintf(E_Unpacker::_pOut,"\nErr: Can't create file [%s]\n",pszPath);
         delete[] pTempBuf;
         pTempBuf = NULL;
         return;
      }

      const int   ZIP_BUF_SIZE = 1 << 20; // About 1 Mb !

      BYTE*    pOut = new BYTE[ZIP_BUF_SIZE];  

      if (!pOut)
      {
         // Error !
         fprintf(E_Unpacker::_pOut,"\nErr: Not enough memory!\n");
         delete[] pTempBuf;
         pTempBuf = NULL;
         return;
      }

      memset(pOut,0,sizeof(pOut));

      // Inflate ZIP decompessor
      // see http://www.zlib.net/zlib_how.html
      // ZLib usage example
      z_stream    zipstream;

      memset(&zipstream,0,sizeof(zipstream));

      int   iErr = inflateInit2(&zipstream,-MAX_WBITS);

      if (iErr != Z_OK)
      {
         fprintf(E_Unpacker::_pOut,"inflateInit2 err %d,%s\n",zipstream.msg);
      }

      DWORD    dwRest = 0;

      // Apply corrections for ZIP_STREAM_xxx method size // sizeof(WORD)
      zipstream.next_in   = pTempBuf  ?  pTempBuf + sizeof(WORD)  :  pZipStart + sizeof(WORD);
      zipstream.avail_in  = dwZipSize - sizeof(WORD);

      zipstream.next_out  = pOut;
      zipstream.avail_out = ZIP_BUF_SIZE;

      // decompress until deflate stream ends or end of file 
      do 
      {  
         // We read input data and set the zipstream structure accordingly. 
         // If we've reached the end of the input file, then we leave the outer loop and report an error, 
         // since the compressed data is incomplete. 
         // Note that we may read more data than is eventually consumed by inflate(), 
         // if the input file continues past the zlib stream. 
         // For applications where zlib streams are embedded in other data, 
         // this routine would need to be modified to return the unused data, 
         // or at least indicate how much of the input data was not used, 
         // so the application would know where to pick up after the zlib stream.

         // The inner do-loop has the same function it did in def(), 
         // which is to keep calling inflate() until has generated all of the output 
         // it can with the provided input.

         // run inflate() on input until output buffer not full
         do 
         {
            // Now we run the decompression engine itself. 
            // there is no need to adjust the flush parameter, 
            // since the zlib format is self-terminating. 
            // The main difference here is that there are return values 
            // that we need to pay attention to. 
            // z_data_error indicates that inflate() detected an error 
            // in the zlib compressed data format, which means that 
            // either the data is not a zlib stream to begin with, 
            // or that the data was corrupted somewhere along the way since it was compressed. 
            // The other error to be processed is z_mem_error, which can occur 
            // since memory allocation is deferred until inflate() needs it, 
            // unlike deflate(), whose memory is allocated at the start by deflateinit().

            // Advanced applications may use deflatesetdictionary() to prime deflate() 
            // with a set of likely data to improve the first 32k or so of compression. 
            // This is noted in the zlib header, so inflate() requests that that dictionary 
            // be provided before it can start to decompress. 
            // Without the dictionary, correct decompression is not possible. 
            // for this routine, we have no idea what the dictionary is, 
            // so the z_need_dict indication is converted to a z_data_error.

            // inflate() can also return z_stream_error, which should not be possible here, 
            // but could be checked for as noted above for def(). 
            // z_buf_error does not need to be checked for here, for the same reasons noted for def(). 
            // z_stream_end will be checked for later.

            iErr = inflate(&zipstream,Z_NO_FLUSH);

            ASSERT(iErr != Z_STREAM_ERROR);  // state not clobbered 
               
            switch (iErr) 
            {
               case Z_NEED_DICT:
               {  
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_NEED_DICT [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  delete[] pTempBuf;
                  pTempBuf = NULL;
                  return;
               }
               case Z_ERRNO:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_ERRNO  [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  delete[] pTempBuf;
                  pTempBuf = NULL;
                  return;
               }
               case Z_STREAM_ERROR:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_STREAM_ERROR [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  delete[] pTempBuf;
                  pTempBuf = NULL;
                  return;
               }
               case Z_DATA_ERROR:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_DATA_ERROR [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  delete[] pTempBuf;
                  pTempBuf = NULL;
                  return;
               }
               case Z_MEM_ERROR:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_MEM_ERROR [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  delete[] pTempBuf;
                  pTempBuf = NULL;
                  return;
               }
               case Z_BUF_ERROR:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_BUF_ERROR [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  delete[] pTempBuf;
                  pTempBuf = NULL;
                  return;
               }
               case Z_VERSION_ERROR:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_VERSION_ERROR [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  delete[] pTempBuf;
                  pTempBuf = NULL;
                  return;
               }
            }

            // The output of inflate() is handled identically to that of deflate().
            dwRest = ZIP_BUF_SIZE - zipstream.avail_out;

            if (fwrite(pOut,1,dwRest,pOutput) != dwRest || ferror(pOutput)) 
            {  
               inflateEnd(&zipstream);
               delete[] pOut;
               pOut = NULL;
               delete[] pTempBuf;
               pTempBuf = NULL;
               return;
            }

            // Reuse the Output Buffer !
            zipstream.next_out  = pOut;
            zipstream.avail_out = ZIP_BUF_SIZE;

            // The inner do-loop ends when inflate() has no more output 
            // as indicated by not filling the output buffer, just as for deflate(). 
            // In this case, we cannot assert that zipstream.avail_in will be zero, 
            // since the deflate stream may end before the file does.
         } 
         while (zipstream.avail_out == 0);

         // The outer do-loop ends when inflate() reports that it has reached the end 
         // of the input zlib stream, has completed the decompression and integrity check, 
         // and has provided all of the output. 
         // This is indicated by the inflate() return value z_stream_end. 
         // The inner loop is guaranteed to leave ret equal to z_stream_end 
         // if the last chunk of the input file read contained the end of the zlib stream. 
         // So if the return value is not z_stream_end, the loop continues to read more input.
      } 
      while (iErr != Z_STREAM_END);  // done when inflate() says it's done
            
      delete[] pOut;
      pOut = NULL;

      delete[] pTempBuf;
      pTempBuf = NULL;
      
      // At this point, decompression successfully completed, 
      // or we broke out of the loop due to no more data being available 
      // from the input file. 
      // If the last inflate() return value is not z_stream_end, 
      // then the zlib stream was incomplete and a data error is returned. 
      // Otherwise, we return with a happy return value. 
      // Of course, inflateend() is called first to avoid a memory leak.
      iErr = inflateEnd(&zipstream);

      if (iErr != Z_OK)
      {
         fprintf(E_Unpacker::_pOut,"inflateEnd err %d,%s [%s]\n",zipstream.msg,pszNaked);
      }

      fclose(pOutput);
      pOutput = NULL;
   }   
/*
   #ifdef _DEBUG   
   else
   {
      fprintf(E_Unpacker::_pOut,"Debug mode: Unknown pack method detected [%08X]\n",dwMethod);
   }
   #endif
*/
}

/* ******************************************************************** **
** @@ E_Unpacker_EEP::E_Unpacker_EEP()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Constructor
** ******************************************************************** */

E_Unpacker_EEP::E_Unpacker_EEP(DWORD dwSizeStub,const BYTE* const pBuf,DWORD dwSize,FILE* pFile)
:  E_Unpacker(dwSizeStub,pBuf,dwSize,pFile),_pNext(NULL),_pStart(NULL) 
{
   memset(_pszTitle,    0,sizeof(_pszTitle));
   memset(_pszUserName, 0,sizeof(_pszUserName));
   memset(_pszSerialNum,0,sizeof(_pszSerialNum));
   memset(_pszReferer,  0,sizeof(_pszReferer));

   memset(_pszHash1,0,sizeof(_pszHash1));
   memset(_pszHash2,0,sizeof(_pszHash2));
   memset(_pszHash3,0,sizeof(_pszHash3));

   _EEP_Version = EEP_VER_3_21;
}

/* ******************************************************************** **
** @@ E_Unpacker_EEP::~E_Unpacker_EEP()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Destructor
** ******************************************************************** */

E_Unpacker_EEP::~E_Unpacker_EEP()
{
}

/* ******************************************************************** **
** @@ E_Unpacker_EEP::Go()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void E_Unpacker_EEP::Go(const char* const pszFilename)
{
   // EEP Version
   double      fVersion = *(double*)(_pBuf + _dwSizeOvr - sizeof(DWORD) - sizeof(double));

   _EEP_Version = (fVersion < 3.3)  ?  EEP_VER_3_21  :  (fVersion > 3.31)  ?  EEP_VER_3_34  :  EEP_VER_3_31;

   // Preset
   CalcHash1();
   CalcHash2();
   CalcHash3();

   // eBook Creator e-Book ID
   DWORD       dwBookID = *(DWORD*)_pBuf;

   // Book (Overlay) Size
   DWORD       dwBookSize = *(DWORD*)(_pBuf + _dwSizeOvr - sizeof(DWORD) - sizeof(fVersion) - sizeof(dwBookSize));

   if (dwBookSize != _dwSizeOvr)
   {
      // Error !
      fprintf(E_Unpacker::_pOut,"\nErr: Shit Happens! [%s]\n",pszFilename);
      return;
   }

   // OLETIME - 64-bit value representing date and time in OLE and Delphi using the format 'MM/dd/yyyy hh:mm:ss'. 
   // OLETIME is a 64-bit double representing the number of days since 12/30/1899 12:00 AM.
   DWORD          dwTimeStampHi = *(DWORD*)(_pBuf + sizeof(dwBookID));
   DWORD          dwTimeStampLo = *(DWORD*)(_pBuf + sizeof(dwBookID) + sizeof(dwTimeStampLo));

   // Owner RegInfo
   // Haven't yet implemented

   char*    pExt = strrchr(pszFilename,'.');

   if (pExt)
   {
      *pExt = 0;  // Remove Extension!
   }

   _pStart  = _pBuf;

   fprintf(E_Unpacker::_pOut,"e-Book ID:               $%08X\n",dwBookID);
   fprintf(E_Unpacker::_pOut,"e-book TimeStamp:        $%08X%08X\n",dwTimeStampLo,dwTimeStampHi);
   fprintf(E_Unpacker::_pOut,"e-Book Title:            %s\n",_pszTitle);
   fprintf(E_Unpacker::_pOut,"Owner RegName:           %s\n",_pszUserName);
   fprintf(E_Unpacker::_pOut,"Owner SerialNum:         %s\n",_pszSerialNum);
   fprintf(E_Unpacker::_pOut,"Owner Referer:           %s\n",_pszReferer);
   fprintf(E_Unpacker::_pOut,"EEP Compiler version:    %5.3f\n",fVersion);
   fprintf(E_Unpacker::_pOut,"EXE-stub Size:           $%08X  (%12lu)\n",_dwSizeStub,_dwSizeStub);
   fprintf(E_Unpacker::_pOut,"e-book Size:             $%08X  (%12lu)\n",_dwSizeOvr,_dwSizeOvr);
   fprintf(E_Unpacker::_pOut,"e-book Protection:       %s\n",GetProtectionType());
   fprintf(E_Unpacker::_pOut,"Known packer used:       ZLib 1.13 [(c) 1995-1998 Jean-loup Gailly and Mark Adler]\n");
   fprintf(E_Unpacker::_pOut,"Known crypto used:       %s\n",(_EEP_Version == EEP_VER_3_21)  ?  "None"  :  "MD5 [(c) 1998-1999 Dave Shapiro, Professional Software, Inc.]");
   fprintf(E_Unpacker::_pOut,"Filenames Protection:    XOR (8 bits)\n");
   fprintf(E_Unpacker::_pOut,"Zip-files Protection:    %s\n",(fVersion < 3.3)  ?  "None"  :  "XOR (256 bits)");
   fprintf(E_Unpacker::_pOut,"\n\n");
   
   // Create the Book directory first !
   if (GetFileAttributes(pszFilename) == 0xFFFFFFFF) 
   {
     EnsureDirectory(pszFilename);
   }

   const DWORD    NULL_PATTERN      = 0;
   const int      NULL_PATTERN_SIZE = 3; 

   QuickSearch((BYTE*)&NULL_PATTERN,NULL_PATTERN_SIZE,_pStart,_dwSizeOvr,_pStart,QS_Finder);
}

/* ******************************************************************** **
** End of File
** ******************************************************************** */
