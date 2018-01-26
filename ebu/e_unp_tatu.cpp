/* ******************************************************************** **
** @@ E_Unpacker_Tatu
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

#include "..\shared\search_quick.h"
#include "..\shared\search_bmh.h"
#include "..\shared\file.h"
#include "..\shared\text.h"
#include "..\shared\pack_zlib_zip.h"
#include "..\shared\pack_zlib_zconf.h"
#include "..\shared\pack_zlib.h"

#include "e_unp.h"
#include "e_unp_Tatu.h"

/* ******************************************************************** **
** @@ internal defines
** ******************************************************************** */

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
** @@ QS_Finder()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

// For warning numbers greater than 4699, those associated with code generation, 
// the warning pragma has effect only when placed outside function definitions.
#pragma warning(push)
#pragma warning(disable: 4706) // assignment within conditional expression
static void __fastcall QS_Finder(void* pParam,DWORD dwFoundAt)
{
   BYTE*    pBuf = (BYTE*)pParam + dwFoundAt;

   int      iNameLen = 0;

   for (int ii = 0; ii < MAX_PATH; ++ii)
   {
      if (*(pBuf + 4 + ii) == 0xB6)
      {
         iNameLen = ii;
         break;
      }
   }

   if (!iNameLen)
   {
      // Error !
      return;
   }

   char     pszName[MAX_PATH + 1];

   memset(pszName,0,sizeof(pszName));

   memcpy(pszName,pBuf + 4,iNameLen);

   DWORD    dwOfs      = E_Unpacker::_dwSizeStub + dwFoundAt;

   DWORD    dwZipStart = 4 + iNameLen + 1;
   BYTE*    pZipStart  = pBuf + dwZipStart;

   const char*    pszPattern = "!2!#";

   int   iZipSize = BMH_Search(pZipStart,E_Unpacker::_dwSizeOvr - dwZipStart,(BYTE*)pszPattern,sizeof(pszPattern) - 1);

   if (iZipSize <= 0)
   {
      // Nothing found!
      return;
   }
   
   DWORD    dwZipSize = (DWORD)iZipSize;

   DWORD    dwMethod   = *(WORD*)(pBuf + dwZipStart);

   dwZipStart += dwFoundAt;   // For print

   if (dwMethod == ZIP_STREAM_BEST)
   {
      fprintf(E_Unpacker::_pOut,"Z  %s  %08X  [%08X]  %08X  (%08X)  %04X  %02X  [%3d]  %s\n","<Best>", dwOfs,dwFoundAt,dwZipSize,dwZipStart,dwMethod,iNameLen,iNameLen,pszName);
   }
   else if (dwMethod == ZIP_STREAM_GOOD)
   {
      fprintf(E_Unpacker::_pOut,"Z  %s  %08X  [%08X]  %08X  (%08X)  %04X  %02X  [%3d]  %s\n","<Good>", dwOfs,dwFoundAt,dwZipSize,dwZipStart,dwMethod,iNameLen,iNameLen,pszName);
   }
   else if (dwMethod == ZIP_STREAM_FAST)
   {
      fprintf(E_Unpacker::_pOut,"Z  %s  %08X  [%08X]  %08X  (%08X)  %04X  %02X  [%3d]  %s\n","<Fast>", dwOfs,dwFoundAt,dwZipSize,dwZipStart,dwMethod,iNameLen,iNameLen,pszName);
   }
   else
   {
      fprintf(E_Unpacker::_pOut,"           %08X  [%08X]  %08X  (%08X)        %02X  [%3d]  %s\n",dwOfs,dwFoundAt,dwZipSize,dwZipStart,iNameLen,iNameLen,pszName);
   }

   if (
         (dwMethod == ZIP_STREAM_BEST) ||
         (dwMethod == ZIP_STREAM_GOOD) ||
         (dwMethod == ZIP_STREAM_FAST)
      )
   {
      char     pszPath[MAX_PATH + 1];

      strcpy(pszPath,E_Unpacker::_pszBook);

      // Force proper delimiter
      char*    pBadChr = NULL;

      // For warning numbers greater than 4699, those associated with code generation, 
      // the warning pragma has effect only when placed outside function definitions.
      // #pragma warning(disable: 4706) // assignment within conditional expression
      while (pBadChr = strchr(pszName,'/'))
      {
         *pBadChr = '\\';
      }

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
            // For this routine, we have no idea what the dictionary is, 
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
   #ifdef _DEBUG   
   else
   {
      fprintf(E_Unpacker::_pOut,"Debug mode: Unknown pack method detected [%08X]\n",dwMethod);
   }
   #endif
}
#pragma warning(pop)

/* ******************************************************************** **
** @@ E_Unpacker_Tatu::E_Unpacker_Tatu()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Constructor
** ******************************************************************** */

E_Unpacker_Tatu::E_Unpacker_Tatu(DWORD dwSizeStub,const BYTE* const pBuf,DWORD dwSize,FILE* pFile)
:  E_Unpacker(dwSizeStub,pBuf,dwSize,pFile),_pNext(NULL),_pStart(NULL) 
{
}

/* ******************************************************************** **
** @@ E_Unpacker_Tatu::~E_Unpacker_Tatu()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Destructor
** ******************************************************************** */

E_Unpacker_Tatu::~E_Unpacker_Tatu()
{
}

/* ******************************************************************** **
** @@ E_Unpacker_Tatu::Go()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void E_Unpacker_Tatu::Go(const char* const pszFilename)
{
   char*    pExt = strrchr(pszFilename,'.');

   if (pExt)
   {
      *pExt = 0;  // Remove Extension!
   }

   _pStart  = _pBuf;

   // Create the Book directory first !
   if (GetFileAttributes(pszFilename) == 0xFFFFFFFF) 
   {
     EnsureDirectory(pszFilename);
   }

   const char*    TATU_PATTERN      = "#1!#";
   const int      TATU_PATTERN_SIZE = 4; 

   QuickSearch((BYTE*)TATU_PATTERN,TATU_PATTERN_SIZE,_pStart,_dwSizeOvr,_pStart,QS_Finder);
}

/* ******************************************************************** **
** End of File
** ******************************************************************** */
