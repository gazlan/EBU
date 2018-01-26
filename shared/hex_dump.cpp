/* ******************************************************************** **
** @@ Hex Dump
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

#include "hex_dump.h"
#include "text.h"

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
** @@ Ruller()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void  Ruller(FILE* pOut)
{
   char     pszTemp[MAX_PATH + 1];

   memset(pszTemp,0,sizeof(pszTemp));
   memset(pszTemp,'-',76);

   pszTemp [9] = '|';
   pszTemp[58] = '|';

   pszTemp[22] = '+';
   pszTemp[34] = '+';
   pszTemp[46] = '+';

   pszTemp[63] = '+';
   pszTemp[67] = '+';
   pszTemp[71] = '+';

   fprintf(pOut,"%s\n",pszTemp);
}

/* ******************************************************************** **
** @@ HexDumpQLine()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void HexDumpQLine(BYTE* pBuf,DWORD dwSize,DWORD dwStart,FILE* pOut)
{
   const int   BLOCK_SIZE = 0x10;     // One Line
   
   DWORD    dwOfs = dwStart - (dwStart % 16);
   
   if (IsBadReadPtr(pBuf + dwOfs,BLOCK_SIZE * 5))
   {
      return;
   }
   
   while (dwOfs < (dwStart + BLOCK_SIZE * 4))
   {
      fprintf(pOut,"%08X: ",dwOfs);
      
      int   ii = 0;
      
      for (ii = 0; ii < BLOCK_SIZE; ++ii)
      {
         if ((dwOfs + ii) < dwSize)
         {
            fprintf(pOut," %02X",pBuf[dwOfs + ii]);
         }
         else
         {
            fprintf(pOut,"   ");
         }
      }
      
      fprintf(pOut,"  ");
      
      for (ii = 0; ii < BLOCK_SIZE; ++ii)
      {
         BYTE     byChr = pBuf[dwOfs + ii];
         
         if ((dwOfs + ii) < dwSize)
         {
            fprintf(pOut,"%c",IsPrintable(byChr)  ?  byChr  :  '.');
         }
         else
         {
            fprintf(pOut," ");
         }
      }
      
      fprintf(pOut,"\n");
      
      dwOfs += BLOCK_SIZE;
   }
}

/* ******************************************************************** **
** @@                   End of File
** ******************************************************************** */
