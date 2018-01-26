/* ******************************************************************** **
** @@ E_Unpacker
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

#include "e_unp.h"

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
            
DWORD    E_Unpacker::_dwSizeStub = 0;
DWORD    E_Unpacker::_dwSizeOvr  = 0;
FILE*    E_Unpacker::_pOut       = NULL;
char     E_Unpacker::_pszBook[MAX_PATH + 1];

/* ******************************************************************** **
** @@ real code
** ******************************************************************** */

/* ******************************************************************** **
** @@ E_Unpacker::E_Unpacker()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Constructor
** ******************************************************************** */

E_Unpacker::E_Unpacker(DWORD dwSizeStub,const BYTE* const pBuf,DWORD dwSizeOvr,FILE* pFile)
:  _pBuf((BYTE*)pBuf)
{
   _dwSizeStub = dwSizeStub;
   _dwSizeOvr  = dwSizeOvr;

   memset(_pszBook,0,sizeof(_pszBook));

   _pOut = pFile;
}

/* ******************************************************************** **
** @@ E_Unpacker::~E_Unpacker()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Destructor
** ******************************************************************** */

E_Unpacker::~E_Unpacker()
{
   // Invalidate !
   _pBuf       = NULL;
   _dwSizeOvr  = 0;
   _dwSizeStub = 0;
}

/* ******************************************************************** **
** @@ E_Unpacker::Go()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
void E_Unpacker::Go(const char* const pszFilename)
{
}
#pragma warning(pop)

/* ******************************************************************** **
** End of File
** ******************************************************************** */
