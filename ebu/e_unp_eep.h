/* ******************************************************************** **
** @@ E_Unpacker_EEP
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Modify : 
** @  Notes  :
** ******************************************************************** */

#ifndef _E_UNPACKER_EEP_HPP_
#define _E_UNPACKER_EEP_HPP_

#if _MSC_VER > 1000
#pragma once
#endif

/* ******************************************************************** **
** @@ internal defines
** ******************************************************************** */

#define  HASH_MD5_SIZE              (32)

enum EEP_VERSION
{
   EEP_VER_3_21,
   EEP_VER_3_31,
   EEP_VER_3_34
};

enum EEP_PROTECTIONS
{
   EEP_PR_NONE,
   EEP_PR_PASSWOD,
   EEP_PR_USERNAME_SERIAL,
   EEP_PR_UNLOCK_KEY
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

class E_Unpacker_EEP : public E_Unpacker
{
   private:

      char                 _pszTitle    [MAX_PATH + 1];
      char                 _pszUserName [MAX_PATH + 1];
      char                 _pszSerialNum[MAX_PATH + 1];
      char                 _pszReferer  [MAX_PATH + 1];

      char                 _pszHash1[HASH_MD5_SIZE + 1];
      char                 _pszHash2[HASH_MD5_SIZE + 1];

      BYTE*                _pNext;
      BYTE*                _pStart;

   public:

      static EEP_VERSION          _EEP_Version;

   public:

      static char    _pszHash3[HASH_MD5_SIZE + 1];

   public:

                E_Unpacker_EEP(DWORD dwSizeStub,const BYTE* const pBuf,DWORD dwSize,FILE* pFile);
      virtual  ~E_Unpacker_EEP();

      virtual void      Go(const char* const pszFilename);

      int   GetTitle();

      static void Decrypt(BYTE* pBuf,int iSize);

   private:

      void                 CalcHash1();
      void                 CalcHash2();
      void                 CalcHash3();
      const char* const    GetProtectionType();
      int                  GetProtection_32x();
      int                  GetProtection_33x();
};

#endif

/* ******************************************************************** **
** End of File
** ******************************************************************** */
