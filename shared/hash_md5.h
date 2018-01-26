/* ******************************************************************** **
** @@ MD5 Header File
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

#ifndef _MD5_HPP_
#define _MD5_HPP_

#if _MSC_VER > 1000
#pragma once
#endif

#define  MD5_DIGEST_SIZE            (sizeof(DWORD) * 4)        // Binary digest
#define  MD5_SIGNATURE_SIZE         (MD5_DIGEST_SIZE * 2 + 1)  // Hex signature, ASCIIZ

/* ******************************************************************** **
** @@ class MD5
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

class MD5
{
   private:

      struct MD5_CONTEXT
      {
         DWORD    _pBuf [4];
         DWORD    _pBits[2];
         BYTE     _pIn  [64];
      };

      MD5_CONTEXT       _Context;
      BYTE              _pDigest     [MD5_DIGEST_SIZE];
      char              _pszSignature[MD5_SIGNATURE_SIZE];
      bool              _bFinalized;

   public:

               MD5();
      virtual ~MD5();

      void            Reset();
      void __fastcall Update(BYTE* pBuf,DWORD dwSize);
      void            Finalize();
      void            Burn();
        
      const BYTE* const Digest();
      const char* const Signature();

   private:

      void __fastcall _Transform(DWORD buf[4],DWORD in[MD5_DIGEST_SIZE]);

      inline void _Step(DWORD (__fastcall *Transform)(DWORD x,DWORD y,DWORD z),DWORD& w,DWORD x,DWORD y,DWORD z,DWORD data,DWORD s);

      static DWORD __fastcall _F1(DWORD x,DWORD y,DWORD z);
      static DWORD __fastcall _F2(DWORD x,DWORD y,DWORD z);
      static DWORD __fastcall _F3(DWORD x,DWORD y,DWORD z);
      static DWORD __fastcall _F4(DWORD x,DWORD y,DWORD z);
};

#endif

/* ******************************************************************** **
** @@                   The End
** ******************************************************************** */
