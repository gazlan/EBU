/* ******************************************************************** **
** @@ Quick Search Source File
** @ Copyrt : Chistian.Charras@dir.univ-rouen.fr
** @ Author : Chistian.Charras@dir.univ-rouen.fr
** @ Modify :
** @ Update :
** @ Notes  : http://www-igm.univ-mlv.fr/~lecroq/string/node19.html
** ******************************************************************** */

/* ******************************************************************** **
**                uses pre-compiled headers
** ******************************************************************** */

/* ******************************************************************** **
** @@                   internal #defines
** ******************************************************************** */

#include "stdafx.h"

#include "search_quick.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* ******************************************************************** **
** @@                   internal prototypes
** ******************************************************************** */

/* ******************************************************************** **
** @@                   external global variables
** ******************************************************************** */

/* ******************************************************************** **
** @@                   static global variables
** ******************************************************************** */

const int ASCII_SIZE = 256;

/* ******************************************************************** **
** @@                   real code
** ******************************************************************** */

/* ******************************************************************** **
** @@ PreQS()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void __fastcall PreQS
(
   const BYTE* const    pPattern, 
   int                  iPatternSize, 
   int*                 pTable
)
{
   for (int ii = 0; ii < ASCII_SIZE; ++ii)
   {
      pTable[ii] = iPatternSize + 1;
   }
   
   for (ii = 0; ii < iPatternSize; ++ii)
   {
      pTable[pPattern[ii]] = iPatternSize - ii;
   }
}

/* ******************************************************************** **
** @@ PreIQS()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void __fastcall PreIQS
(
   const BYTE* const    pPattern, 
   int                  iPatternSize, 
   int*                 pTable
)
{
   for (int ii = 0; ii < ASCII_SIZE; ++ii)
   {
      pTable[ii] = iPatternSize + 1;
   }
   
   for (ii = 0; ii < iPatternSize; ++ii)
   {
      pTable[toupper(pPattern[ii])] = iPatternSize - ii;
      pTable[tolower(pPattern[ii])] = iPatternSize - ii;
   }
}

/* ******************************************************************** **
** @@ QuickSearch()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void __fastcall QuickSearch
(
   const BYTE* const    pPattern, 
   int                  iPatternSize, 
   const BYTE* const    pText, 
   int                  iTextSize,
   void*                pParam,
   void (__fastcall *QS_Callback)(void* pParam,DWORD dwFoundAt)
) 
{
   int   pTable[ASCII_SIZE];

   // Preprocessing 
   PreQS(pPattern,iPatternSize,pTable);

   // Searching 
   int iShift = 0;

   while (iShift < (iTextSize - iPatternSize)) 
   {
      if (!memcmp(pPattern,pText + iShift,iPatternSize))
      {
         QS_Callback(pParam,iShift);
      }

      iShift += pTable[pText[iShift + iPatternSize]];  
   }
}

/* ******************************************************************** **
** @@ QuickISearch()
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void __fastcall QuickISearch
(
   const BYTE* const    pPattern, 
   int                  iPatternSize, 
   const BYTE* const    pText, 
   int                  iTextSize,
   void*                pParam,
   void (__fastcall *QS_Callback)(void* pParam,DWORD dwFoundAt)
) 
{
   int   pTable[ASCII_SIZE];

   // Preprocessing 
   PreIQS(pPattern,iPatternSize,pTable);

   // Searching 
   int iShift = 0;

   while (iShift <= iTextSize - iPatternSize) 
   {
      if (!_memicmp(pPattern,pText + iShift,iPatternSize))
      {
         QS_Callback(pParam,iShift);
      }

      iShift += pTable[pText[iShift + iPatternSize]];  
   }
}

/* ******************************************************************** **
** End of File
** ******************************************************************** */
