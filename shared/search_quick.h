/* ******************************************************************** **
** @@ Quick Search Header File
** @ Copyrt : Chistian.Charras@dir.univ-rouen.fr
** @ Author : Chistian.Charras@dir.univ-rouen.fr
** @ Modify : 
** @ Update :
** @ Notes  : http://www-igm.univ-mlv.fr/~lecroq/string/node19.html
** ******************************************************************** */

#ifndef __QSEARCH_HPP__
#define __QSEARCH_HPP__

#if _MSC_VER > 1000
#pragma once
#endif

/* ******************************************************************** **
** @@ internal defines
** ******************************************************************** */

typedef void (*QS_Callback)(void* pParam,DWORD dwFoundAt);

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

void __fastcall QuickSearch (const BYTE* const pPattern,int iPatternSize,const BYTE* const pText,int iTextSize,void* pParam,void (__fastcall *QS_Callback)(void* pParam,DWORD dwFoundAt));
void __fastcall QuickISearch(const BYTE* const pPattern,int iPatternSize,const BYTE* const pText,int iTextSize,void* pParam,void (__fastcall *QS_Callback)(void* pParam,DWORD dwFoundAt));

#endif

/* ******************************************************************** **
** End of File
** ******************************************************************** */
