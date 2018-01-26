/**
  brief implementation of the Aho-Corasick pattern matching algorithm.
    
  most of this code was taken from another module by Georgios Portokalidis
  with his permission. 

  2005/03/20: I [Danny Yoo (dyoo@hkn.eecs.berkeley.edu)] munged this code a
  bit to make it more useful for my Python wrapper.  I modified the output
  State to maintain the lengths of keyword matches, refactored out some of the
  initialization code, and dropped support for the optional global tree M.
  I'll leave the notices about FFPF in here, of course, but be aware that this
  code has been mutated quite at bit.  *grin*
  
  
  ----
  
  Fairly Fast Packet Filter
  http://ffpf.sourceforge.net/

  Copyright (c), 2003 - 2004 Georgios Portokalidis, Herbert Bos & Willem de Bruijn
  contact info : wdebruij_AT_users.sourceforge.net

  ----
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

#include <stdio.h>
#include <string.h>

#include "slist.h"
#include "search_ac.h"

/* 
dyoo: Every State whose Depth is less than the
   TRANSITION_SWITCHING_THRESHOLD gets a dense transition table definition.
   Otherwise, use the sparse _pList representation.

   The number I've selected below is completely arbitrary.  Changing it will
   affect memory and performance.  I dunno, three seemed like a good odd
   number.  *grin*
*/
#define TRANSITION_SWITCHING_THRESHOLD       5 // (3)

/* 
dyoo: I want to abstract away the direct use of the State
   transition list, to see if I can use a simple linked list
   representation instead. 
*/

/* ******************************************************************** **
** @@ AC_Search::AC_Search()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Constructor
** ******************************************************************** */

AC_Search::AC_Search()
:  _pLast(NULL)
{
   // Create New ZeroState
   _pRoot = new AC_STATE;

   if (!_pRoot)
   {
      // Error !
      return;
   }

   _dwNextID = 1;

   _pRoot->_dwID    = 0;
   _pRoot->_dwDepth = 0;
   _pRoot->_wPatLen = 0;
   _pRoot->_pData   = NULL;
   _pRoot->_pFail   = NULL;
   
   InitTT(_pRoot);
}

/* ******************************************************************** **
** @@ AC_Search::~AC_Search()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Destructor
** ******************************************************************** */

AC_Search::~AC_Search()
{
   if (_pRoot) 
   {
      for (int ii = 0; ii < ASCII_SIZE; ++ii)
      {
         if (Get(_pRoot,(BYTE)ii) && (Get(_pRoot,(BYTE)ii)->_dwID > 0))
         {
            Free(Get(_pRoot,(BYTE)ii));
         }
      }

      // dyoo: added to free the last node.
      Dealloc(_pRoot);
      
      _pRoot = NULL;
   }
}

/* ******************************************************************** **
** @@ AC_Search::Set()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void AC_Search::Set(AC_STATE* pFrom,BYTE symbol,AC_STATE* pTo) 
{
   AC_LABELED_EDGE*     pEdge = NULL;

   switch (pFrom->_TType) 
   {
      case TT_DENSE_TRANSITIONS:
      {
         pFrom->_TData._pStateArr[symbol] = pTo;
         return;
      }
      case TT_SPARSE_TRANSITIONS:
      {
         pEdge = new AC_LABELED_EDGE;

         if (!pEdge) 
         {
            // Error !
            return;
         }
       
         pEdge->_byLabel = symbol;
         pEdge->_pState  = pTo;
         
         if (slist_prepend(pFrom->_TData._pList, pEdge) < 0) 
         {
            // Error !
            return;
         }
       
         return;
      }
   }
}

/* ******************************************************************** **
** @@ AC_Search::Get()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Follows the transition arrow from the State, along the Edge labeled by the symbol.  
** @  Notes  : If no such transition exists, returns NULL. 
** ******************************************************************** */

AC_STATE* AC_Search::Get(AC_STATE* pState,BYTE bySymbol) 
{
   slist_node_t*     pNode = NULL;

   switch (pState->_TType) 
   {
      case TT_DENSE_TRANSITIONS:
      {
         return pState->_TData._pStateArr[bySymbol];
      }
      case TT_SPARSE_TRANSITIONS:
      {
         pNode = slist_head(pState->_TData._pList);
       
         while (pNode) 
         {
            if (((AC_LABELED_EDGE*)pNode->data)->_byLabel == bySymbol) 
            {
               return ((AC_LABELED_EDGE*)pNode->data)->_pState;
            }

            pNode = pNode->next;
         }
       
         return NULL;
      }
   }

   // Error !
   return NULL;
}

/* ******************************************************************** **
** @@ AC_Search::InitTT()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

/* 
Initializes all of the transition arrows to NULL.  representation_type must
   be either DENSE_TRANSITIONS or SPARSE_TRANSITIONS.  If everything is ok,
   returns true. If bad things happen, returns false. 
*/

bool AC_Search::InitTT(AC_STATE* pState) 
{
   AC_TRANS_TYPE     TType = TT_DENSE_TRANSITIONS;

   if (pState->_dwDepth >= TRANSITION_SWITCHING_THRESHOLD) 
   {
      TType = TT_SPARSE_TRANSITIONS;
   }
   
   switch (TType) 
   {
      case TT_DENSE_TRANSITIONS:
      {
         pState->_TType = TT_DENSE_TRANSITIONS;
         
         pState->_TData._pStateArr = new AC_STATE*[ASCII_SIZE];

         if (!pState->_TData._pStateArr)
         {
            return false;
         }

         memset(pState->_TData._pStateArr,0,ASCII_SIZE * sizeof(AC_STATE*));
         
         return true;
      }
      case TT_SPARSE_TRANSITIONS:
      {
         pState->_TType = TT_SPARSE_TRANSITIONS;
         
         pState->_TData._pList = new slist_t;

         if (!pState->_TData._pList)
         {
            return false;
         }

         slist_init(pState->_TData._pList);

         return true;
      }
   }

   // Error !
   return false;
}

/* ******************************************************************** **
** @@ AC_Search::Destroy()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Deallocates the transition table.  Do nothing for now. 
** ******************************************************************** */

void AC_Search::Destroy(AC_STATE* pState) 
{
   switch (pState->_TType) 
   {
      case TT_DENSE_TRANSITIONS:
      {
         delete[] pState->_TData._pStateArr;
         pState->_TData._pStateArr = NULL;
         
         return;
      }
      case TT_SPARSE_TRANSITIONS:
      {
         slist_destroy(pState->_TData._pList,SLIST_FREE_DATA);

         delete pState->_TData._pList;
         pState->_TData._pList = NULL;
         
         return;
      }
   }
}

/* 
Anything below this should access State transitions only through the API
   methods here.  They should not touch the structure directly, because the
   implementation of State->transitions will be munged! 
   */

/**********************************************************************/

/*
Does an aho-corasick search, given a 'Buf' of length 'n'.  If
   we're able to find a match, returns a positive integer.  Also
   outputs the start-end indices of the match as well as the last
   matching _pState with out parameters.
*/

/* ******************************************************************** **
** @@ AC_Search::FindFirstShortest()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void* AC_Search::FindFirstShortest
(
   BYTE*          pBuf,
   DWORD          dwSize,
   DWORD          dwStartPos,
   DWORD&         rFoundAt
)
{
   AC_STATE*      pState = _pRoot;

   for (DWORD ii = dwStartPos; ii < dwSize; ++ii)
   {
      while (!Get(pState,pBuf[ii])) 
      {
         pState = pState->_pFail;
      }

      pState = Get(pState,pBuf[ii]);

      if (pState->_wPatLen) 
      {
         _pLast = pState;

         rFoundAt = ii - pState->_wPatLen + 1;
   
         return pState->_pData;
      }
   }

   rFoundAt = 0;

   return NULL;
}

/* ******************************************************************** **
** @@ AC_Search::FindFirstLongest()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Similar to the FindFirstShortest function, but tries to return the longest match.
** ******************************************************************** */

void* AC_Search::FindFirstLongest
(
   BYTE*       pBuf,
   DWORD       dwSize,
   DWORD       dwStartPos,
   DWORD&      rFoundAt
)
{
   AC_STATE*      pState = _pRoot;

   rFoundAt = 0;
   
   int   iNextPos = -1;

   AC_STATE*   pFound = NULL;

   for (DWORD ii = dwStartPos; ii < dwSize; ++ii)
   {
      if (!Get(pState,pBuf[ii]) && (iNextPos != -1)) 
      {
         _pLast = pState;

         return pFound->_pData;
      }

      while (!Get(pState,pBuf[ii]))
      {
         pState = pState->_pFail;
      }
      
      pState = Get(pState,pBuf[ii]);
      
      if (pState->_wPatLen) 
      {
         pFound = pState;

         rFoundAt = ii - (pFound->_wPatLen - 1);
         iNextPos = ii + 1;
      }
   }

   // If we reach the end of the pBuf, we still have to double check if we had a longest match queued up. 
   if (iNextPos != -1) 
   {
      return pState->_pData;
   }
   
   return NULL;
}

/* ******************************************************************** **
** @@ AC_Search::FindAllShortest()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void AC_Search::FindAllShortest
(
   BYTE*          pBuf,
   DWORD          dwSize,
   DWORD          dwStartPos,
   AS_Callback    pCallback,
   bool           bOverlaps
)
{
   AC_STATE*      pStart = _pRoot;

   DWORD    dwFoundAt = 0;

   while (true)
   {
      void*    pFound = _FindFirstShortest(pStart,pBuf,dwSize,dwStartPos,dwFoundAt);

      if (!pFound)
      {
         return;
      }

      if (pCallback)
      {
         pCallback(pFound,dwFoundAt);
      }

      dwStartPos = dwFoundAt + 1;

      pStart = bOverlaps  ?  _pLast  :  _pRoot;
   }
}

/* ******************************************************************** **
** @@ AC_Search::FindAllShortest()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void AC_Search::FindAllLongest
(
   BYTE*          pBuf,
   DWORD          dwSize,
   DWORD          dwStartPos,
   AS_Callback    pCallback,
   bool           bOverlaps
)
{
   AC_STATE*      pStart = _pRoot;

   DWORD    dwFoundAt = 0;

   while (true)
   {
      void*    pFound = _FindFirstLongest(pStart,pBuf,dwSize,dwStartPos,dwFoundAt);

      if (!pFound)
      {
         return;
      }

      if (pCallback)
      {
         pCallback(pFound,dwFoundAt);
      }

      dwStartPos = dwFoundAt + 1;

      pStart = bOverlaps  ?  _pLast  :  _pRoot;
   }
}

/* ******************************************************************** **
** @@ AC_Search::Dealloc()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Really do the memory deallocation of a State
** ******************************************************************** */

void AC_Search::Dealloc(AC_STATE* pState) 
{
   Destroy(pState);

   delete pState;
   pState = NULL;
}

/* ******************************************************************** **
** @@ AC_Search::Free()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Helper function for ac_destroy.
** @  Notes  : Recursively frees up each _pState, doing this essentially Depth-first. 
** ******************************************************************** */

void AC_Search::Free(AC_STATE* pState)
{
   for (int ii = 0; ii < ASCII_SIZE; ++ii)
   {
      if (Get(pState,(BYTE)ii)) 
      {
         Free(Get(pState,(BYTE)ii));
      }
   }

   // Actually do the memory deallocation here.
   Dealloc(pState);
}

/* ******************************************************************** **
** @@ AC_Search::Finalize()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

#pragma warning(disable: 4706)
bool AC_Search::Finalize()
{
   slist_t     queue;

   slist_init(&queue);

   // Set all Fail() transition of ZeroState to point to itself
   for (int ii = 0; ii < ASCII_SIZE; ++ii)
   {
      AC_STATE*   pState = Get(_pRoot,(BYTE)ii);

      if (!pState)
      {
         Set(_pRoot,(BYTE)ii,_pRoot);
      }
      else
      {
         // Construct Failure()
         if (slist_append(&queue,pState) < 0)
         {
            slist_destroy(&queue,SLIST_LEAVE_DATA);
            return false;
         }

         pState->_pFail = _pRoot;
      }
   }

   // Set Fail() for Depth > 0
   AC_STATE*   r = NULL;

   while ((r = (AC_STATE*)slist_pop_first(&queue)))
   {
      for (int ii = 0; ii < ASCII_SIZE; ++ii)
      {
         AC_STATE*   s = Get(r,(BYTE)ii);
                     
         if (!s)
         {
            continue;
         }

         if (slist_append(&queue,s) < 0)
         {
            slist_destroy(&queue,SLIST_LEAVE_DATA);
            return false;
         }

         AC_STATE*   pState = r->_pFail;
         
         while (!Get(pState,(BYTE)ii))
         {
            pState = pState->_pFail;
         }  

         s->_pFail = Get(pState,(BYTE)ii);
         
         // TRACE("Setting f(%u) == %u\n",s->_dwID,GetGoto(pState,(BYTE)ii)->_dwID);
      }
   }

   slist_destroy(&queue,SLIST_LEAVE_DATA);

   return true;
}

/* ******************************************************************** **
** @@ AC_Search::AddBytes()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

bool AC_Search::AddBytes
(
   BYTE*                      pBuf,
   WORD                       wItemCnt,
   const void* const          pData
)
{
   if (!wItemCnt)
   {
      // Error !
      return false;
   }

   AC_STATE*      pState = _pRoot;
   AC_STATE*      pGoto  = NULL;

   WORD  ii = 0;

   // As long as we have transitions follow them
   for (ii = 0; ii < wItemCnt; ++ii)
   {
      pGoto = Get(pState,pBuf[ii]);

      if (!pGoto)
      {
         break;
      }
      
      pState = pGoto;
   }

   if (ii == wItemCnt) 
   {
      // dyoo: added so that if a keyword ends up AC a prefix of another, 
      // we still mark that as a match.
      // Fix: Don't allow duplicates !
      if (!pGoto->_wPatLen)
      {
         pGoto->_wPatLen = wItemCnt;
         pGoto->_pData   = (void*)pData;

         return true;
      }

      // duplicated keys!
      return false;
   }

   for ( ; ii < wItemCnt; ++ii)
   {
      // Create new _pState
      pGoto = new AC_STATE;

      if (!pGoto)
      {
         return false;
      }

      pGoto->_dwID  = _dwNextID++;
      pGoto->_pData = NULL;
      
      // TRACE("Allocating State %d\n",pGoto->_dwID);

      pGoto->_dwDepth = pState->_dwDepth + 1;

      if (!InitTT(pGoto))
      {
         // Error !
         return false;
      }

      // Create transition
      Set(pState,pBuf[ii],pGoto);
      
      // TRACE("%u -> %c -> %u\n",pState->_dwID,pBuf[ii],pGoto->_dwID);
      
      pState = pGoto;
      
      pGoto->_wPatLen = 0;
      pGoto->_pFail   = NULL;
   }

   pGoto->_wPatLen = wItemCnt;
   pGoto->_pData   = (void*)pData;

   return true;
}

/* ******************************************************************** **
** @@ AC_Search::AddWords()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

bool AC_Search::AddWords
(
   WORD*                      pBuf,
   WORD                       wItemCnt,
   const void* const          pData
)
{
   return AddBytes((BYTE*)pBuf,(WORD)(wItemCnt * sizeof(WORD)),pData);
}

/* ******************************************************************** **
** @@ AC_Search::AddDWords()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

bool AC_Search::AddDWords
(
   DWORD*                     pBuf,
   WORD                       wItemCnt,
   const void* const          pData
)
{
   return AddBytes((BYTE*)pBuf,(WORD)(wItemCnt * sizeof(DWORD)),pData);
}

/* ******************************************************************** **
** @@ AC_Search::AddQWords()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

bool AC_Search::AddQWords
(
   QWORD*                     pBuf,
   WORD                       wItemCnt,
   const void* const          pData
)
{
   return AddBytes((BYTE*)pBuf,(WORD)(wItemCnt *  sizeof(QWORD)),pData);
}

/* ******************************************************************** **
** @@ AC_Search::AddCase()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Long form for Text append
** ******************************************************************** */

bool AC_Search::AddCase
(
   BYTE*                      pBuf,
   WORD                       wItemCnt,
   const void* const          pData
)
{
   if (!wItemCnt)
   {
      // Error !
      return false;
   }

   AC_STATE*      pState = _pRoot;
   AC_STATE*      pGoto  = NULL;

   WORD  ii = 0;
   
   // As long as we have transitions follow them
   for (ii = 0; ii < wItemCnt; ++ii)
   {
      pGoto = Get(pState,pBuf[ii]);

      if (!pGoto)
      {
         break;
      }
      
      pState = pGoto;
   }

   if (ii == wItemCnt) 
   {
      // dyoo: added so that if a keyword ends up AC a prefix of another, 
      // we still mark that as a match.
      // Fix: Don't allow duplicates !
      if (!pGoto->_wPatLen)
      {
         pGoto->_wPatLen = wItemCnt;
         pGoto->_pData   = (void*)pData;

         return true;
      }

      // duplicated keys!
      return false;
   }

   for ( ; ii < wItemCnt; ++ii)
   {
      // Create new State
      pGoto = new AC_STATE;

      if (!pGoto)
      {
         // Error !
         return false;
      }

      pGoto->_dwID  = _dwNextID++;
      pGoto->_pData = NULL;
      
      // TRACE("Allocating State %d\n",pGoto->_dwID);

      pGoto->_dwDepth = pState->_dwDepth + 1;

      if (!InitTT(pGoto))
      {
         // Error !
         return false;
      }

      // Create transition
      Set(pState,pBuf[ii],pGoto);
      
      // TRACE("%u -> %c -> %u\n",pState->_dwID,pBuf[ii],pGoto->_dwID);
      
      pState = pGoto;
      
      pGoto->_wPatLen = 0;
      pGoto->_pFail   = NULL;
   }

   pGoto->_wPatLen = wItemCnt;
   pGoto->_pData   = (void*)pData;

   return true;
}

/* ******************************************************************** **
** @@ AC_Search::_FindFirstShortest()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Internal use only !
** ******************************************************************** */

void* AC_Search::_FindFirstShortest
(
   AC_STATE*      pStart,
   BYTE*          pBuf,
   DWORD          dwSize,
   DWORD          dwStartPos,
   DWORD&         rFoundAt
)
{
   for (DWORD ii = dwStartPos; ii < dwSize; ++ii)
   {
      while (!Get(pStart,pBuf[ii])) 
      {
         pStart = pStart->_pFail;
      }

      pStart = Get(pStart,pBuf[ii]);

      if (pStart->_wPatLen) 
      {
         _pLast = pStart;

         rFoundAt = ii - (pStart->_wPatLen - 1);
   
         return pStart->_pData;
      }
   }

   rFoundAt = 0;

   return NULL;
}

/* ******************************************************************** **
** @@ AC_Search::_FindFirstLongest()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Internal use only !
** @  Notes  : Similar to the FindFirstShortest function, but tries to return the longest match.
** ******************************************************************** */

void* AC_Search::_FindFirstLongest
(
   AC_STATE*      pStart,
   BYTE*          pBuf,
   DWORD          dwSize,
   DWORD          dwStartPos,
   DWORD&         rFoundAt
)
{
   rFoundAt = 0;
   
   int      iNextPos = -1;

   AC_STATE*   pFound = NULL;

   for (DWORD ii = dwStartPos; ii < dwSize; ++ii)
   {
      if (!Get(pStart,pBuf[ii]) && (iNextPos != -1)) 
      {
         _pLast = pStart;

         return pFound->_pData;
      }

      while (!Get(pStart,pBuf[ii]))
      {
         pStart = pStart->_pFail;
      }
      
      pStart = Get(pStart,pBuf[ii]);
      
      if (pStart->_wPatLen) 
      {
         pFound = pStart;

         rFoundAt = ii - (pFound->_wPatLen - 1);
         iNextPos = ii + 1;
      }
   }

   // If we reach the end of the pBuf, we still have to double check if we had a longest match queued up. 
   if (iNextPos != -1) 
   {
      return pStart->_pData;
   }
   
   return NULL;
}

/* ******************************************************************** **
** @@                   End of File
** ******************************************************************** */
