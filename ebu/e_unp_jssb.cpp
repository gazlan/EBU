/* ******************************************************************** **
** @@ E_Unpacker_JSSB
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
#include "e_unp_jssb.h"

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

static JSSB_PROTECTIONS     _SecurityType = JSSB_PR_NONE;

const char* pJSSB_Protections[4] =
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
** @@ E_Unpacker_JSSB::GetProtectionType()
** @  Copyrt : 
** @  Author : 
** @  Update :
** @  Update :
** @  Notes  :
** ******************************************************************** */

const char* const E_Unpacker_JSSB::GetProtectionType()
{
   return "Unknown";  // TODO !!
}

/* ******************************************************************** **
** @@ E_Unpacker_JSSB::GetName()
** @  Copyrt : 
** @  Author : 
** @  Update :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static const char* const GetName(const BYTE* const pBuf,BYTE bySize)
{
   if (!bySize)
   {
      // Error !
      return NULL;
   }

   static char    pszString[MAX_PATH + 1];
   
   memset(pszString,0,sizeof(pszString));

   memcpy(pszString,pBuf,bySize);   // Always secure!

   for (int ii = 0; ii < (int)bySize; ++ii)
   {
      if (!IsPrintable((BYTE)pszString[ii]))
      {
         return false;
      }
   }

   char*    pColumn = strrchr(pszString,':');

   return pColumn  ?  pColumn + 1  :  pszString;
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
   BYTE*    pBuf = (BYTE*)pParam + dwFoundAt + sizeof(DWORD);

   int      iNameLen = *(int*)pBuf;

   const char*    pszName = GetName(pBuf + sizeof(DWORD),(BYTE)iNameLen);

   if (!pszName || !*pszName)
   {
      // Error !
      return;
   }

   DWORD    dwOfs      = E_Unpacker::_dwSizeStub + dwFoundAt;
   BYTE*    pParams    = pBuf + sizeof(DWORD) + iNameLen;

   DWORD    dwParam1   = *(DWORD*)(pParams);
   char*    pszArcType = (char*)(pParams + sizeof(DWORD));
   DWORD    dwZipSize  = *(DWORD*)(pParams + sizeof(DWORD) * 2);

   DWORD    dwZipStart = dwFoundAt + sizeof(DWORD) * 2 + iNameLen + sizeof(DWORD) * 3;
   BYTE*    pZipStart  = pParams + sizeof(DWORD) * 3;

   DWORD    dwMethod = *(WORD*)pZipStart;
   
   fprintf(E_Unpacker::_pOut,"Z  %s  %08X  [%08X]  %08X  (%08X)  %04X  %02X  [%3d]  %s\n","<Best>", dwOfs,dwFoundAt,dwZipSize,dwZipStart,dwMethod,iNameLen,iNameLen,pszName);

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
         return;
      }

      const int   ZIP_BUF_SIZE = 1 << 20; // About 1 Mb !

      BYTE*    pOut = new BYTE[ZIP_BUF_SIZE];  

      if (!pOut)
      {
         // Error !
         fprintf(E_Unpacker::_pOut,"\nErr: Not enough memory!\n");
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
      zipstream.next_in   = pZipStart + sizeof(WORD);
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
                  return;
               }
               case Z_ERRNO:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_ERRNO  [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  return;
               }
               case Z_STREAM_ERROR:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_STREAM_ERROR [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  return;
               }
               case Z_DATA_ERROR:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_DATA_ERROR [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  return;
               }
               case Z_MEM_ERROR:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_MEM_ERROR [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  return;
               }
               case Z_BUF_ERROR:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_BUF_ERROR [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
                  return;
               }
               case Z_VERSION_ERROR:
               {
                  fprintf(E_Unpacker::_pOut,"*** Err: Z_VERSION_ERROR [%s]\n",pszNaked);
                  inflateEnd(&zipstream);
                  delete[] pOut;
                  pOut = NULL;
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
** @@ E_Unpacker_JSSB::E_Unpacker_JSSB()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Constructor
** ******************************************************************** */

E_Unpacker_JSSB::E_Unpacker_JSSB(DWORD dwSizeStub,const BYTE* const pBuf,DWORD dwSize,FILE* pFile)
:  E_Unpacker(dwSizeStub,pBuf,dwSize,pFile),_pNext(NULL),_pStart(NULL) 
{
   memset(_pszTitle,    0,sizeof(_pszTitle));
   memset(_pszUserName, 0,sizeof(_pszUserName));
   memset(_pszSerialNum,0,sizeof(_pszSerialNum));
   memset(_pszReferer,  0,sizeof(_pszReferer));
}

/* ******************************************************************** **
** @@ E_Unpacker_JSSB::~E_Unpacker_JSSB()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Destructor
** ******************************************************************** */

E_Unpacker_JSSB::~E_Unpacker_JSSB()
{
}

/* ******************************************************************** **
** @@ E_Unpacker_JSSB::Go()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void E_Unpacker_JSSB::Go(const char* const pszFilename)
{
   // eBook Creator e-Book ID
//   DWORD       dwBookID = *(DWORD*)_pBuf;

   // Owner RegInfo
   // Haven't yet implemented

   char*    pExt = strrchr(pszFilename,'.');

   if (pExt)
   {
      *pExt = 0;  // Remove Extension!
   }

   _pStart  = _pBuf;

//   fprintf(E_Unpacker::_pOut,"e-Book ID:               $%08X\n",dwBookID);
   fprintf(E_Unpacker::_pOut,"e-Book Title:            %s\n",_pszTitle);
   fprintf(E_Unpacker::_pOut,"EXE-stub Size:           $%08X  (%12lu)\n",_dwSizeStub,_dwSizeStub);
   fprintf(E_Unpacker::_pOut,"e-book Size:             $%08X  (%12lu)\n",_dwSizeOvr,_dwSizeOvr);
   fprintf(E_Unpacker::_pOut,"e-book Protection:       %s\n",GetProtectionType());
   fprintf(E_Unpacker::_pOut,"Known packer used:       ZLib 1.04 [(c) 1995-1998 Jean-loup Gailly and Mark Adler]\n");
   fprintf(E_Unpacker::_pOut,"\n\n");
   
   // Create the Book directory first !
   if (GetFileAttributes(pszFilename) == 0xFFFFFFFF) 
   {
     EnsureDirectory(pszFilename);
   }

// Get Very first file - do it manually !!!!!
// QS_Finder()

   const DWORD    NULL_PATTERN      = 0;
   const int      NULL_PATTERN_SIZE = 4; 

   QuickSearch((BYTE*)&NULL_PATTERN,NULL_PATTERN_SIZE,_pStart,_dwSizeOvr,_pStart,QS_Finder);
}

/* ******************************************************************** **
** End of File
** ******************************************************************** */
