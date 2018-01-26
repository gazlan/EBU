/* ******************************************************************** **
** @@ E_Unpacker_JSSB
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Modify : 
** @  Notes  :
** ******************************************************************** */

#ifndef _E_UNPACKER_JSSB_HPP_
#define _E_UNPACKER_JSSB_HPP_

#if _MSC_VER > 1000
#pragma once
#endif

/* ******************************************************************** **
** @@ internal defines
** ******************************************************************** */

#define  HASH_MD5_SIZE              (32)

enum JSSB_PROTECTIONS
{
   JSSB_PR_NONE,
   JSSB_PR_PASSWOD,
   JSSB_PR_USERNAME_SERIAL,
   JSSB_PR_UNLOCK_KEY
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
** @@ Global Function Prototypes
** ******************************************************************** */

class E_Unpacker_JSSB : public E_Unpacker
{
   private:

      char                 _pszTitle    [MAX_PATH + 1];
      char                 _pszUserName [MAX_PATH + 1];
      char                 _pszSerialNum[MAX_PATH + 1];
      char                 _pszReferer  [MAX_PATH + 1];
      BYTE*                _pNext;
      BYTE*                _pStart;

   public:

                E_Unpacker_JSSB(DWORD dwSizeStub,const BYTE* const pBuf,DWORD dwSize,FILE* pFile);
      virtual  ~E_Unpacker_JSSB();

      virtual void      Go(const char* const pszFilename);

      int   GetTitle();

      static void Decrypt(BYTE* pBuf,int iSize);

   private:

      const char* const    GetProtectionType();
};

#endif

/* ******************************************************************** **
** End of File
** ******************************************************************** */
