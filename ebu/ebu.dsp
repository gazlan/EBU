# Microsoft Developer Studio Project File - Name="ebu" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ebu - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ebu.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ebu.mak" CFG="ebu - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ebu - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ebu - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ebu - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /Zp1 /MD /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x419 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /nodefaultlib:"libc.lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ebu - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /Zp1 /MDd /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x419 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libc.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "ebu - Win32 Release"
# Name "ebu - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\e_unp.cpp
# End Source File
# Begin Source File

SOURCE=.\e_unp_coin.cpp
# End Source File
# Begin Source File

SOURCE=.\e_unp_eep.cpp
# End Source File
# Begin Source File

SOURCE=.\e_unp_jssb.cpp
# End Source File
# Begin Source File

SOURCE=.\e_unp_tatu.cpp
# End Source File
# Begin Source File

SOURCE=.\ebu.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\file.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\file_walker.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\hash_crc32.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\hash_md5.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\hex_dump.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\mmf.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\pelib.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\search_ac.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\search_bmh.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\search_quick.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\slist.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\Shared\text.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\pack_zlibstat.lib
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\e_unp.h
# End Source File
# Begin Source File

SOURCE=.\e_unp_coin.h
# End Source File
# Begin Source File

SOURCE=.\e_unp_eep.h
# End Source File
# Begin Source File

SOURCE=.\e_unp_jssb.h
# End Source File
# Begin Source File

SOURCE=.\e_unp_tatu.h
# End Source File
# Begin Source File

SOURCE=..\Shared\file.h
# End Source File
# Begin Source File

SOURCE=..\Shared\file_walker.h
# End Source File
# Begin Source File

SOURCE=..\Shared\hash_crc32.h
# End Source File
# Begin Source File

SOURCE=..\Shared\hash_md5.h
# End Source File
# Begin Source File

SOURCE=..\Shared\hex_dump.h
# End Source File
# Begin Source File

SOURCE=..\Shared\mmf.h
# End Source File
# Begin Source File

SOURCE=..\Shared\pack_zconf.h
# End Source File
# Begin Source File

SOURCE=..\Shared\pack_zip.h
# End Source File
# Begin Source File

SOURCE=..\Shared\pack_zlib.h
# End Source File
# Begin Source File

SOURCE=..\Shared\pelib.h
# End Source File
# Begin Source File

SOURCE=..\Shared\search_ac.h
# End Source File
# Begin Source File

SOURCE=..\Shared\search_bmh.h
# End Source File
# Begin Source File

SOURCE=..\Shared\search_quick.h
# End Source File
# Begin Source File

SOURCE=..\Shared\slist.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\Shared\text.h
# End Source File
# End Group
# End Target
# End Project
