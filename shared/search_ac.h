/*
  implementation of the Aho-Corasick pattern matching algorithm.
    
  most of this code was taken from another module by Georgios Portokalidis
  with his permission. Since I don't really know how he has implemented the code
  I haven't tried to document it. 
  
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

#ifndef _AHO_CORASICK_HPP_
#define _AHO_CORASICK_HPP_

#ifndef QWORD
typedef unsigned __int64   QWORD;
#endif

#define ASCII_SIZE         (256)

typedef void (*AS_Callback)(void* pFound,DWORD dwFoundAt);

// Defines the two types of transition table implementations we might want to use.
enum AC_TRANS_TYPE
{ 
   TT_DENSE_TRANSITIONS, 
   TT_SPARSE_TRANSITIONS 
};

union TRANSITIONS
{
   // StateArr is a pointer to an StateArr of states.
   struct AC_STATE**       _pStateArr;
   slist_t*                _pList;
};

struct AC_STATE
{                          
   DWORD                _dwID;
   DWORD                _dwDepth;
   struct AC_STATE*     _pFail;
   AC_TRANS_TYPE        _TType;
   TRANSITIONS          _TData;
   void*                _pData;
   WORD                 _wPatLen;
};

// Represents a single labeled edge.
struct AC_LABELED_EDGE 
{  
   BYTE                 _byLabel;
   struct AC_STATE*     _pState;
};

// type of any function that helps with search. 
typedef DWORD (*Searcher)(AC_STATE* pState,BYTE* pBuf,DWORD dwSize,int iStartPos,int& riFoundAt,AC_STATE** pLastState);

// A transition table has two possible implementations:
// 1. A "dense" _pStateArr.  Constant-time access, but expensive in terms of memory.
// 2. A "sparse" linked list.  Linear time access, but not too expensive in terms of memory.

class AC_Search
{
   private:

      DWORD          _dwNextID;

   public:
   
      AC_STATE*      _pRoot;
      AC_STATE*      _pLast;

   public:
   
       AC_Search();
      ~AC_Search(); 

      bool     AddBytes (BYTE*  pBuf,WORD wItemCnt,const void* const pData);
      bool     AddWords (WORD*  pBuf,WORD wItemCnt,const void* const pData);
      bool     AddDWords(DWORD* pBuf,WORD wItemCnt,const void* const pData);
      bool     AddQWords(QWORD* pBuf,WORD wItemCnt,const void* const pData);
      bool     AddCase  (BYTE*  pBuf,WORD wItemCnt,const void* const pData);
      bool     Finalize();

      void*    FindFirstShortest(BYTE* pBuf,DWORD dwSize,DWORD dwStartPos,DWORD& rFoundAt);
      void*    FindFirstLongest (BYTE* pBuf,DWORD dwSize,DWORD dwStartPos,DWORD& rFoundAt);

      // NEVER use bOverlaps = true !!
      // Smth buggy in this implementation...
      // For repeated patterns (like 'aaa') it's lead to INFINITE LOOP !!
      void     FindAllShortest(BYTE* pBuf,DWORD dwSize,DWORD dwStartPos,AS_Callback pCallback,bool bOverlaps = false);
      void     FindAllLongest (BYTE* pBuf,DWORD dwSize,DWORD dwStartPos,AS_Callback pCallback,bool bOverlaps = false);

   private:
               
      void     Zero(); 
                           
      bool     InitTT  (AC_STATE* pState);
      void     Destroy (AC_STATE* pState);
      void     Dealloc (AC_STATE* pState);
      void     Free    (AC_STATE* pState);

      inline void*   _FindFirstShortest(AC_STATE* pState,BYTE* pBuf,DWORD dwSize,DWORD dwStartPos,DWORD& rFoundAt);
      inline void*   _FindFirstLongest (AC_STATE* pState,BYTE* pBuf,DWORD dwSize,DWORD dwStartPos,DWORD& rFoundAt);

      void        Set(AC_STATE* pFrom, BYTE bySign,AC_STATE* pTo);
      AC_STATE*   Get(AC_STATE* pState,BYTE bySign);
};

#endif

/* ******************************************************************** **
** @@                   End of File
** ******************************************************************** */
