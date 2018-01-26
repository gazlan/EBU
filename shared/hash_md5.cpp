/* ******************************************************************** **
** @@ MD5 Source File
** @  Copyrt : Public Domain
** @  Author : John Walker http://www.fourmilab.ch/
** @  Modify :
** @  Update : 
** @  Notes  :
** ******************************************************************** */

/* ******************************************************************** **
**                uses pre-compiled headers
** ******************************************************************** */

#include "stdafx.h"

#include "hash_md5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* ******************************************************************** **
** @@                   internal defines
** ******************************************************************** */

/* ******************************************************************** **
** @@                   internal prototypes
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
** @@ MD5::Md5()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update : 
** @  Notes  : Constructor
** ******************************************************************** */

MD5::MD5()
{
   Reset();
}

/* ******************************************************************** **
** @@ MD5::~Md5()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update : 
** @  Notes  : Destructor
** ******************************************************************** */

MD5::~MD5()
{
}

/* ******************************************************************** **
** @@ MD5::Reset()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update : 
** @  Notes  : Start MD5 accumulation.  Set bit count to 0 and buffer to
** @  Notes  : mysterious initialization constants.
** ******************************************************************** */

void MD5::Reset()
{
   _Context._pBuf[0] = 0x67452301;
   _Context._pBuf[1] = 0xEFCDAB89;
   _Context._pBuf[2] = 0x98BADCFE;
   _Context._pBuf[3] = 0x10325476;

   _Context._pBits[0] = 0;
   _Context._pBits[1] = 0;

   memset(_pDigest,0,MD5_SIGNATURE_SIZE);

   _bFinalized = false;
}

/* ******************************************************************** **
** @@ MD5::Update()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update : 
** @  Notes  : Update context to reflect the concatenation of another
** @  Notes  : buffer full of bytes.
** ******************************************************************** */

void MD5::Update(BYTE* pBuf,DWORD dwSize)
{
   if (!dwSize || !pBuf)
   {
      // Error !
      ASSERT(0);
      return;
   }

   if (IsBadReadPtr(pBuf,dwSize))
   {
      // Error !
      ASSERT(0);
      return;
   }

   DWORD t;

   // Update bitcount
   t = _Context._pBits[0];

   if ((_Context._pBits[0] = t + (dwSize << 3)) < t)
   {
      _Context._pBits[1]++;                                  // Carry from low to high
   }

   _Context._pBits[1] += dwSize >> 29;

   t = (t >> 3) & 0x3f;   // Bytes already _pIn shsInfo->data

   // Handle any leading odd-sized chunks
   if (t)
   {
      BYTE*    p = (BYTE*)_Context._pIn + t;

      t = 64 - t;

      if (dwSize < t)
      {
         memcpy(p,pBuf,dwSize);
         return;
      }

      memcpy(p,pBuf,t);

      _Transform(_Context._pBuf,(DWORD*)_Context._pIn);
      
      pBuf   += t;
      dwSize -= t;
   }

   // Process data _pIn 64-byte chunks
   while (dwSize >= 64)
   {
      memcpy(_Context._pIn,pBuf,64);

      _Transform(_Context._pBuf,(DWORD*)_Context._pIn);

      pBuf   += 64;
      dwSize -= 64;
   }

   // Handle any remaining bytes of data.
   memcpy(_Context._pIn,pBuf,dwSize);
}

/* ******************************************************************** **
** @@ MD5::_Finalize()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update : 
** @  Notes  : Final wrapup - pad to 64-byte boundary with the bit pattern
** @  Notes  : 1 0* (64-bit count of _pBits processed, MSB-first)
** ******************************************************************** */

void MD5::Finalize()
{
   memset(_pDigest,0,sizeof(_pDigest));

   if (_bFinalized)
   {
      return;
   }

   DWORD    count;
   BYTE*    p;

   // Compute number of bytes mod 64
   count = (_Context._pBits[0] >> 3) & 0x3F;

   // Set the first char of padding to 0x80.  This is safe since there is
   // always at least one byte free
   p = _Context._pIn + count;
   *p++ = 0x80;

   // Bytes of padding needed to make 64 bytes
   count = 64 - 1 - count;

   // Pad out to 56 mod 64
   if (count < 8)
   {
      // Two lots of padding:  Pad the first block to 64 bytes
      memset(p,0,count);

      _Transform(_Context._pBuf,(DWORD*)_Context._pIn);

      // Now fill the next block with 56 bytes
      memset(_Context._pIn,0,56);
   }
   else
   {
      // Pad block to 56 bytes
      memset(p,0,count - 8);
   }

   // Append length _pIn _pBits and transform
   ((DWORD*)_Context._pIn)[14] = _Context._pBits[0];
   ((DWORD*)_Context._pIn)[15] = _Context._pBits[1];

   _Transform(_Context._pBuf,(DWORD*)_Context._pIn);
   _bFinalized = true;

   for (int kk = 0; kk < MD5_DIGEST_SIZE; ++kk)
   {
      sprintf(&_pszSignature[kk * 2],"%02X",((BYTE*)_Context._pBuf)[kk]);
   }

   _pszSignature[MD5_SIGNATURE_SIZE - 1] = 0; // Ensure ASCIIZ
}

/* ******************************************************************** **
** @@ MD5::Burn()
** @  Copyrt : 
** @  Author :
** @  Modify : 
** @  Update :
** @  Notes  : In case it's sensitive
** ******************************************************************** */

void MD5::Burn()
{
   memset(&_Context,0xFF,sizeof(_Context)); 
}

/* ******************************************************************** **
** @@ MD5::Digest()
** @  Copyrt : 
** @  Author :
** @  Modify : 
** @  Update :
** @  Notes  :
** ******************************************************************** */

const BYTE* const MD5::Digest()
{
   if (!_bFinalized)
   {
      return NULL;
   }

   return (BYTE*)_Context._pBuf;
}

/* ******************************************************************** **
** @@ MD5::Signature()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update :
** @  Notes  :
** ******************************************************************** */

const char* const MD5::Signature()
{
   if (!_bFinalized)
   {
      return NULL;
   }

   for (int ii = 0, jj = 0; ii < MD5_DIGEST_SIZE; ++ii, jj += 2)
   {
      sprintf(&_pszSignature[jj],"%02X",((BYTE*)_Context._pBuf)[ii]);
   }

   return _pszSignature;
}

/* ******************************************************************** **
** @@ MD5::_Transform()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update : 
** @  Notes  : Core Function
** @  Notes  : The core of the MD5 algorithm, this alters an existing MD5
** @  Notes  : hash to reflect the addition of 16 longwords of new data.
** @  Notes  : MD5Update blocksthe data and converts bytes into longwords
** @  Notes  : for this routine.
** ******************************************************************** */

void MD5::_Transform(DWORD _pBuf[4],DWORD _pIn[16])
{
   DWORD a, b, c, d;

   a = _pBuf[0];
   b = _pBuf[1];
   c = _pBuf[2];
   d = _pBuf[3];

   _Step(_F1,a,b,c,d,_pIn[0]  + 0xD76AA478, 7);
   _Step(_F1,d,a,b,c,_pIn[1]  + 0xE8C7B756,12);
   _Step(_F1,c,d,a,b,_pIn[2]  + 0x242070DB,17);
   _Step(_F1,b,c,d,a,_pIn[3]  + 0xC1BDCEEE,22);
   _Step(_F1,a,b,c,d,_pIn[4]  + 0xF57C0FAF, 7);
   _Step(_F1,d,a,b,c,_pIn[5]  + 0x4787C62A,12);
   _Step(_F1,c,d,a,b,_pIn[6]  + 0xA8304613,17);
   _Step(_F1,b,c,d,a,_pIn[7]  + 0xFD469501,22);
   _Step(_F1,a,b,c,d,_pIn[8]  + 0x698098D8, 7);
   _Step(_F1,d,a,b,c,_pIn[9]  + 0x8B44F7AF,12);
   _Step(_F1,c,d,a,b,_pIn[10] + 0xFFFF5BB1,17);
   _Step(_F1,b,c,d,a,_pIn[11] + 0x895CD7BE,22);
   _Step(_F1,a,b,c,d,_pIn[12] + 0x6B901122, 7);
   _Step(_F1,d,a,b,c,_pIn[13] + 0xFD987193,12);
   _Step(_F1,c,d,a,b,_pIn[14] + 0xA679438E,17);
   _Step(_F1,b,c,d,a,_pIn[15] + 0x49B40821,22);

   _Step(_F2,a,b,c,d,_pIn[1]  + 0xF61E2562, 5);
   _Step(_F2,d,a,b,c,_pIn[6]  + 0xC040B340, 9);
   _Step(_F2,c,d,a,b,_pIn[11] + 0x265E5A51,14);
   _Step(_F2,b,c,d,a,_pIn[0]  + 0xE9B6C7AA,20);
   _Step(_F2,a,b,c,d,_pIn[5]  + 0xD62F105D, 5);
   _Step(_F2,d,a,b,c,_pIn[10] + 0x02441453, 9);
   _Step(_F2,c,d,a,b,_pIn[15] + 0xD8A1E681,14);
   _Step(_F2,b,c,d,a,_pIn[4]  + 0xE7D3FBC8,20);
   _Step(_F2,a,b,c,d,_pIn[9]  + 0x21E1CDE6, 5);
   _Step(_F2,d,a,b,c,_pIn[14] + 0xC33707D6, 9);
   _Step(_F2,c,d,a,b,_pIn[3]  + 0xF4D50D87,14);
   _Step(_F2,b,c,d,a,_pIn[8]  + 0x455A14ED,20);
   _Step(_F2,a,b,c,d,_pIn[13] + 0xA9E3E905, 5);
   _Step(_F2,d,a,b,c,_pIn[2]  + 0xFCEFA3F8, 9);
   _Step(_F2,c,d,a,b,_pIn[7]  + 0x676F02D9,14);
   _Step(_F2,b,c,d,a,_pIn[12] + 0x8D2A4C8A,20);

   _Step(_F3,a,b,c,d,_pIn[5]  + 0xFFFA3942, 4);
   _Step(_F3,d,a,b,c,_pIn[8]  + 0x8771F681,11);
   _Step(_F3,c,d,a,b,_pIn[11] + 0x6D9D6122,16);
   _Step(_F3,b,c,d,a,_pIn[14] + 0xFDE5380C,23);
   _Step(_F3,a,b,c,d,_pIn[1]  + 0xA4BEEA44, 4);
   _Step(_F3,d,a,b,c,_pIn[4]  + 0x4BDECFA9,11);
   _Step(_F3,c,d,a,b,_pIn[7]  + 0xF6BB4B60,16);
   _Step(_F3,b,c,d,a,_pIn[10] + 0xBEBFBC70,23);
   _Step(_F3,a,b,c,d,_pIn[13] + 0x289B7EC6, 4);
   _Step(_F3,d,a,b,c,_pIn[0]  + 0xEAA127FA,11);
   _Step(_F3,c,d,a,b,_pIn[3]  + 0xD4EF3085,16);
   _Step(_F3,b,c,d,a,_pIn[6]  + 0x04881D05,23);
   _Step(_F3,a,b,c,d,_pIn[9]  + 0xD9D4D039, 4);
   _Step(_F3,d,a,b,c,_pIn[12] + 0xE6DB99E5,11);
   _Step(_F3,c,d,a,b,_pIn[15] + 0x1FA27CF8,16);
   _Step(_F3,b,c,d,a,_pIn[2]  + 0xC4AC5665,23);

   _Step(_F4,a,b,c,d,_pIn[0]  + 0xF4292244, 6);
   _Step(_F4,d,a,b,c,_pIn[7]  + 0x432AFF97,10);
   _Step(_F4,c,d,a,b,_pIn[14] + 0xAB9423A7,15);
   _Step(_F4,b,c,d,a,_pIn[5]  + 0xFC93A039,21);
   _Step(_F4,a,b,c,d,_pIn[12] + 0x655B59C3, 6);
   _Step(_F4,d,a,b,c,_pIn[3]  + 0x8F0CCC92,10);
   _Step(_F4,c,d,a,b,_pIn[10] + 0xFFEFF47D,15);
   _Step(_F4,b,c,d,a,_pIn[1]  + 0x85845DD1,21);
   _Step(_F4,a,b,c,d,_pIn[8]  + 0x6FA87E4F, 6);
   _Step(_F4,d,a,b,c,_pIn[15] + 0xFE2CE6E0,10);
   _Step(_F4,c,d,a,b,_pIn[6]  + 0xA3014314,15);
   _Step(_F4,b,c,d,a,_pIn[13] + 0x4E0811A1,21);
   _Step(_F4,a,b,c,d,_pIn[4]  + 0xF7537E82, 6);
   _Step(_F4,d,a,b,c,_pIn[11] + 0xBD3AF235,10);
   _Step(_F4,c,d,a,b,_pIn[2]  + 0x2AD7D2BB,15);
   _Step(_F4,b,c,d,a,_pIn[9]  + 0xEB86D391,21);

   _pBuf[0] += a;
   _pBuf[1] += b;
   _pBuf[2] += c;
   _pBuf[3] += d;
}

/* ******************************************************************** **
** @@ MD5::_F1()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update : 
** @  Notes  : Core Function
** ******************************************************************** */

DWORD MD5::_F1(DWORD x,DWORD y,DWORD z)
{
   return z ^ (x & (y ^ z));
}

/* ******************************************************************** **
** @@ MD5::_F2()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update : 
** @  Notes  : Core Function
** ******************************************************************** */

DWORD MD5::_F2(DWORD x,DWORD y,DWORD z)
{
   return _F1(z,x,y);
}

/* ******************************************************************** **
** @@ MD5::_F3()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update : 
** @  Notes  : Core Function
** ******************************************************************** */

DWORD MD5::_F3(DWORD x,DWORD y,DWORD z)
{
   return x ^ y ^ z;
}

/* ******************************************************************** **
** @@ MD5::_F4()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update : 
** @  Notes  : Core Function
** ******************************************************************** */

DWORD MD5::_F4(DWORD x,DWORD y,DWORD z)
{
   return y ^ (x | ~z);
}

/* ******************************************************************** **
** @@ MD5::_Step()
** @  Copyrt : 
** @  Author : 
** @  Modify : 
** @  Update : 
** @  Notes  : This is the central step _pIn the MD5 algorithm.
** ******************************************************************** */

void MD5::_Step(DWORD (__fastcall *f)(DWORD x,DWORD y,DWORD z),DWORD& w,DWORD x,DWORD y,DWORD z,DWORD data,DWORD s)
{
   w += f(x,y,z) + data;
   w  = (w << s) | ( w >> (32 - s));
   w += x;
}

/* ******************************************************************** **
** @@                   End of File
** ******************************************************************** */
