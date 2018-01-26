/* ******************************************************************** **
** @@ ZIP Hdr file
** @ Copyrt : 
** @ Author : 
** @ Modify :
** @ Update :
** @ Notes  :
** ******************************************************************** */

#ifndef _ZIP_HPP_
#define _ZIP_HPP_

#if _MSC_VER > 1000
#pragma once
#endif

/* ******************************************************************** **
** @@ internal defines
** ******************************************************************** */

#define  ZIP_STREAM_BEST            (0xDA78)
#define  ZIP_STREAM_GOOD            (0x9C78)
#define  ZIP_STREAM_FAST            (0x0178)

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
** @@ Prototypes
** ******************************************************************** */

/* ******************************************************************** **
** @@ struct ZIP_LOCAL_FILE_HEADER
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Dscr   :
** ******************************************************************** */

#pragma pack(push, 1)
struct ZIP_LOCAL_FILE_HEADER
{
   DWORD    _dwSignature;  // 0x04034B50
   WORD     _wVersion;
   WORD     _wFlag;
   WORD     _wCompression;
   WORD     _wFileTime;
   WORD     _wFileDate;
   DWORD    _dwCRC32;
   DWORD    _dwPackSize;
   DWORD    _dwOrgSize;
   WORD     _wFilenameLen;
   WORD     _wExtraLen;
   // filename (variable size)
   // extra field (variable size)
};
#pragma pack(pop)

/* ******************************************************************** **
** @@ struct ZIP_DATA_DESCRIPTOR
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Dscr   :
** ******************************************************************** */

// This descriptor exists only if bit 3 of the general
// purpose bit flag is set (see below). It is byte aligned
// and immediately follows the last byte of compressed data.
// This descriptor is used only when it was not possible to
// seek in the output zip file, e.g., when the output zip file
// was standard output or a non seekable device.

#pragma pack(push, 1)
struct ZIP_DATA_DESCRIPTOR
{
   DWORD    _dwCRC32;
   DWORD    _dwPackSize;
   DWORD    _dwOrgSize;
};
#pragma pack(pop)

/* ******************************************************************** **
** @@ struct ZIP_CENTRAL_FILE_HEADER
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Dscr   :
** ******************************************************************** */

#pragma pack(push, 1)
struct ZIP_CENTRAL_FILE_HEADER
{
   DWORD                      _dwSignature;  // 0x02014B50
   ZIP_LOCAL_FILE_HEADER      _Info;
   WORD                       _wFileCommentLen;
   WORD                       _DiskNum;
   WORD                       _FileAttrInt;
   DWORD                      _FileAttrExt;
   DWORD                      _Relative;
   // filename (variable size)
   // extra field (variable size)
   // file comment (variable size)
};
#pragma pack(pop)

/* ******************************************************************** **
** @@ struct ZIP_END_CENTRAL_DIR
** @  Copyrt : 
** @  Author : 
** @  Modify :
** @  Update :
** @  Dscr   :
** ******************************************************************** */

#pragma pack(push, 1)
struct ZIP_END_CENTRAL_DIR
{
   DWORD    _dwSignature;  // 0x06054B50
   WORD     _wDiskNum;
   WORD     _wCentDirDiskNum;
   WORD     _wCentDirDiskEntries;
   WORD     _wCentDirTotalEntries;
   DWORD    _dwCentDirSize;
   DWORD    _dwCentDirOfs;
   WORD     _wCommentLen;
   // zipfile comment (variable size)
};
#pragma pack(pop)

#endif

/* ******************************************************************** **
** End of File
** ******************************************************************** */
