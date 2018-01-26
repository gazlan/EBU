/* ******************************************************************** **
** @@ EBU
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Dscr   : e-book Identifier & Unpacker
** ******************************************************************** */

/* ******************************************************************** **
**                uses pre-compiled headers
** ******************************************************************** */

#include "stdafx.h"

#include <time.h>

#include "..\shared\file.h"
#include "..\shared\hex_dump.h"
#include "..\shared\mmf.h"
#include "..\shared\pelib.h"
#include "..\shared\file_walker.h"
#include "..\shared\hash_crc32.h"
#include "..\shared\hash_md5.h"
#include "..\shared\slist.h"
#include "..\shared\search_bmh.h"
#include "..\shared\search_ac.h"

#include "e_unp.h"
#include "e_unp_eep.h"
#include "e_unp_tatu.h"
#include "e_unp_jssb.h"
#include "e_unp_coin.h"

/* ******************************************************************** **
** @@                   internal defines
** ******************************************************************** */

#define EBU_VERSION                 "1.64"
#define OVERLAY_SIZE_MIN            (64)
#define OVERLAY_SIZE_LOOKUP         (MAX_PATH + 32)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef NDEBUG
#pragma optimize("gsy",on)
#pragma comment(linker,"/FILEALIGN:512 /MERGE:.rdata=.text /MERGE:.data=.text /SECTION:.text,EWR /IGNORE:4078")
#endif

///////////////////////////////////////////////////////////////////////////////

#define MAX_NAME_SIZE            (80)
#define MAX_VERS_SIZE            (16)
#define MAX_SIGN_SIZE            (32)
#define MAX_COMMENT_SIZE         (64)
#define MAX_APPENDIX_SIZE        (32)

///////////////////////////////////////////////////////////////////////////////

enum  SSF_HINT
{
   HINT_NONE,
   HINT_FILE_BEGIN,
   HINT_FILE_END,
   HINT_RICH_MARKER,
   HINT_ENRTY_POINT,
   HINT_OVERLAY_BEGIN,
   HINT_OVERLAY_END
};

///////////////////////////////////////////////////////////////////////////////

#define STR_7Z                                     STR_OVERLAY_START " 7z archiver" STR_HEADER_MARKER
#define STR_7Z_STUB_1                              "7z SFX New archive stub (by Oleg Scherbakov)"
#define STR_7Z_STUB_2                              STR_SELF_MADE " 7z SFX (based on 7z Lib DLL by Igor Pavlov)"
#define STR_ACE                                    STR_OVERLAY_START " ACE archver" STR_HEADER_MARKER
#define STR_ACKER_PACK                             "AckerPack SFX archve"
#define STR_ACTIV_EBOOK_COMPILER                   "Activ E-Book Compiler"
#define STR_ACTIV_EBOOK_COMPILER_REBRANDER         STR_ACTIV_EBOOK_COMPILER " Rebrander"
#define STR_ADLIB_SAMPLE_SOUND                     STR_OVERLAY_START " Adlib Sample sound" STR_HEADER_MARKER
#define STR_ADOBE_PHOTO_SHOP_IMAGE                 STR_OVERLAY_START " Adobe PhotoShop image" STR_HEADER_MARKER
#define STR_ALPHA_BMP_IMAGE                        STR_OVERLAY_START " Alpha BMP image" STR_HEADER_MARKER
#define STR_AMGC                                   STR_OVERLAY_START " AMGC archver" STR_HEADER_MARKER
#define STR_ANTECHINUS_EBOOK_WIZARD                "Antechinus eBook Wizard"
#define STR_ARAFASOFT_EBOOK_CREATOR                "Arafasoft E-Book Creator"
#define STR_ASYLUM_SOUND                           STR_OVERLAY_START " ASYLUM sound" STR_HEADER_MARKER
#define STR_AUTO_LOGIC_IMAGE                       STR_OVERLAY_START " AutoLogic image" STR_HEADER_MARKER
#define STR_AVHRR_IMAGE                            STR_OVERLAY_START " AVHRR image" STR_HEADER_MARKER
#define STR_AVI                                    STR_OVERLAY_START " AVI video" STR_HEADER_MARKER
#define STR_BAFF_BMP_LIB_IMAGE                     STR_OVERLAY_START " BAFF BMP Library image" STR_HEADER_MARKER
#define STR_BOOK_BIZ_MASTER                        "BookBiz Protect Master"
#define STR_BOOK_BIZ_MASTER_ENCRYPTED              STR_BOOK_BIZ_MASTER " * Encrypted"
#define STR_CALS_RASTER_IMAGE                      STR_OVERLAY_START " CALS Raster image" STR_HEADER_MARKER
#define STR_CGM_IMAGE                              STR_OVERLAY_START " CGM image" STR_HEADER_MARKER
#define STR_CHM                                    STR_OVERLAY_START " CHM compiled help" STR_HEADER_MARKER
#define STR_COINSOFT_COPYRIGHT                     "Coinsoft EBook"
#define STR_COINSOFT_EBOOK_MAKER                   STR_COINSOFT_COPYRIGHT " Maker"
#define STR_COINSOFT_EBOOK_PROTECT_MASTER          STR_COINSOFT_COPYRIGHT " Protect Master"
#define STR_COREL_PHOTO_PAINT_IMAGE                STR_OVERLAY_START " CorelPhotoPaint image" STR_HEADER_MARKER
#define STR_CREATIVE_SOUND                         STR_OVERLAY_START " Creative sound" STR_HEADER_MARKER
#define STR_CUBICOMP_PICTURE_MAKER_IMAGE           STR_OVERLAY_START " CubiComp PictureMaker image" STR_HEADER_MARKER
#define STR_CYBER_ARTICLE_PRO                      "CyberArticle Pro"
#define STR_DANGEROUS                              "Dangerous! Possible VX-stuff!"
#define STR_DATA_BEAM_IMAGE                        STR_OVERLAY_START " DataBeam image" STR_HEADER_MARKER
#define STR_DELPI_JUNK                             "Some Delphi Junk"
#define STR_DESKTOP_AUTHOR                         "DesktopAuthor"
#define STR_DR_EXPLAIN                             "Dr. Explain"
#define STR_DR_HALO_IMAGE                          STR_OVERLAY_START " DrHalo or DrGenius Palette Graphics image" STR_HEADER_MARKER
#define STR_EBOOKS_COMPILER                        "eBooks Compiler"
#define STR_EBOOKS_WRITER                          "eBooksWriter"
#define STR_EBOOK_BLASTER                          "eBook Blaster"
#define STR_EBOOK_BUILDER                          "eBook Builder"
#define STR_EBOOK_EDIT_PRO                         "e-Book Edit Pro"
#define STR_EBOOK_EDIT_PRO_CRACKED                 STR_EBOOK_EDIT_PRO " * Cracked EXE"
#define STR_EBOOK_EDIT_PRO_NP                      STR_EBOOK_EDIT_PRO " (Unpacked)"
#define STR_EBOOK_EDIT_PRO_OLD                     STR_EBOOK_EDIT_PRO
#define STR_EBOOK_GOLD                             "eBookGold"
#define STR_EBOOK_HTML_COMPILER_PRO                "eBook HTML Compiler Pro"
#define STR_EBOOK_MAESTRO                          "eBook Maestro"
#define STR_EBOOK_MAKER                            "eBook Maker"
#define STR_EBOOK_PACK_EXPRESS                     "eBook Pack Express"
#define STR_EBOOK_PDF_WRITER                       "eBook pdfWriter"
#define STR_EBOOK_PRODUCER                         "eBook Producer"
#define STR_EBOOK_PUBLICANT                        "E-Book Publicant"
#define STR_EBOOK_SNAP                             "eBookSnap"
#define STR_EBOOK_STARTER                          "eBook Starter"
#define STR_EBOOK_WORKSHOP                         "eBook Workshop"
#define STR_EMAG_CREATOR                           "eMagCreator"
#define STR_EMZ                                    STR_OVERLAY_START " EMZ image" STR_HEADER_MARKER
#define STR_EPSF_IMAGE                             STR_OVERLAY_START " Encapsulated Postscript image" STR_HEADER_MARKER
#define STR_ERDAS_LAN_GIS_IMAGE                    STR_OVERLAY_START " Erdas LAN/GIS image" STR_HEADER_MARKER
#define STR_EVRSOFT_FIRST_PAGE                     "Evrsoft First Page"
#define STR_EXE_BOOK_SELF_PUBLISHER                "ExeBook Self-Publisher"
#define STR_EXE_EBOOK_CREATOR_FOR_TREEPAD          "exe-eBook creator for TreePad"
#define STR_EXE_OUTPUT_FOR_PHP                     "ExeOutput for PHP"
#define STR_E_DITOR_EBOOK_COMPILER                 "E-ditor eBook Compiler"
#define STR_FAKE_PAY_ARCHIVE                       "Fake archive. SMS extortioner"
#define STR_FAR_HTML                               "FAR HTML"
#define STR_FAST_EBOOK_COMPILER                    "Fast eBook Compiler"
#define STR_FIZ                                    STR_OVERLAY_START " FIZ archver" STR_HEADER_MARKER
#define STR_GEM_IMAGE                              STR_OVERLAY_START " GEM image" STR_HEADER_MARKER
#define STR_GENERIC_COMPILER                       "Fast eBook Compiler or Jimmy Brown E-Book Creator"
#define STR_GIF87                                  STR_OVERLAY_START " GIF87 image" STR_HEADER_MARKER
#define STR_GIF89                                  STR_OVERLAY_START " GIF89 image" STR_HEADER_MARKER
#define STR_GOES_IMAGE                             STR_OVERLAY_START " GOES image" STR_HEADER_MARKER
#define STR_GZIP                                   STR_OVERLAY_START " GZIP archver" STR_HEADER_MARKER
#define STR_HEADER_MARKER                          " header marker"
#define STR_HELPINATOR                             "Helpinator"
#define STR_HELP_AND_MANUAL                        "Help & Manual"
#define STR_HELP_MAKER                             "HelpMaker"
#define STR_HIJAAK_IMAGE_DRAW_IMAGE                STR_OVERLAY_START " HiJaak Image Draw image" STR_HEADER_MARKER
#define STR_HISTOGRAM_IMAGE                        STR_OVERLAY_START " Histogram image" STR_HEADER_MARKER
#define STR_HITACHI_RASTER_IMAGE                   STR_OVERLAY_START " Hitachi Raster image" STR_HEADER_MARKER
#define STR_HPACK                                  STR_OVERLAY_START " HPACK archver" STR_HEADER_MARKER
#define STR_HP_48_SX_IMAGE                         STR_OVERLAY_START " HP-48sx image" STR_HEADER_MARKER
#define STR_HSC_SOUND                              STR_OVERLAY_START " HSC sound" STR_HEADER_MARKER
#define STR_HSI_JPEG_IMAGE                         STR_OVERLAY_START " HSI JPEG image" STR_HEADER_MARKER
#define STR_HSI_RAW_IMAGE                          STR_OVERLAY_START " HSI Raw image" STR_HEADER_MARKER
#define STR_HTML_2_EXE                             "html2exe"
#define STR_HTML_EBOOK_MAKER_AND_ENCRYPTER         "HTML eBook Maker and Encrypter"
#define STR_HTML_EXECUTABLE                        "HTML Executable"
#define STR_HYPER_MAKER                            "HyperMaker"
#define STR_HYPER_PUBLISH_PRO                      "HyperPublish Pro"
#define STR_IBM_IOCA_IMAGE                         STR_OVERLAY_START " IBM IOCA image" STR_HEADER_MARKER
#define STR_IMAGING_TECHNOLOGY_IMAGE               STR_OVERLAY_START " Imaging Technology image" STR_HEADER_MARKER
#define STR_IMG_SOFTWARE_SET_IMAGE                 STR_OVERLAY_START " Img Software Set image" STR_HEADER_MARKER
#define STR_INSET_SYSTEMS_IGF_IMAGE                STR_OVERLAY_START " Inset Systems IGF image" STR_HEADER_MARKER
#define STR_INSTALL_SHIELD                         STR_OVERLAY_START " InstallShield archver" STR_HEADER_MARKER
#define STR_INTEL_DCX_IMAGE                        STR_OVERLAY_START " Intel DCX image" STR_HEADER_MARKER
#define STR_INTERPLAY_MVE                          STR_OVERLAY_START " Interplay's MVE" STR_HEADER_MARKER
#define STR_INTER_GRAPH_IMAGE                      STR_OVERLAY_START " InterGraph image" STR_HEADER_MARKER
#define STR_JANSOFT_SBOOK_BUILDER                  "JanSoft SbookBuilder"
#define STR_JAR                                    STR_OVERLAY_START " JAR archver"  STR_HEADER_MARKER
#define STR_JFIF                                   STR_OVERLAY_START " JFIF image" STR_HEADER_MARKER
#define STR_JIMMY_BROWN_EBOOK_CREATOR              "Jimmy Brown E-Book Creator"
#define STR_JRC                                    STR_OVERLAY_START " JRC archver" STR_HEADER_MARKER
#define STR_KE_EBOOK_CREATOR                       "KeeBook Creator"
#define STR_KOFAX_GROUP_4_IMAGE                    STR_OVERLAY_START " Kofax Group 4 image" STR_HEADER_MARKER
#define STR_LEITHAUSER                             "Leithauser Research Ebook setup"
#define STR_LOTUS_IMAGE                            STR_OVERLAY_START " Lotus image" STR_HEADER_MARKER
#define STR_LTC_EBOOK_CREATOR                      "LTC eBook Creator"
#define STR_MACROMEDIA_SHOCKWAVE_VIDEO             STR_OVERLAY_START " MacroMedia ShockWave video" STR_HEADER_MARKER
#define STR_MAC_PAINT_IMAGE                        STR_OVERLAY_START " MacPaint image" STR_HEADER_MARKER
#define STR_MAC_PICT_IMAGE                         STR_OVERLAY_START " Mac PICT image" STR_HEADER_MARKER
#define STR_MAC_STUFFIT                            STR_OVERLAY_START " Mac StuffIt archver" STR_HEADER_MARKER
#define STR_MICROGRAFIX_DRAW_IMAGE                 STR_OVERLAY_START " Micrografix Draw image" STR_HEADER_MARKER
#define STR_MPEG                                   STR_OVERLAY_START " MPEG video" STR_HEADER_MARKER
#define STR_MPEG_VIDEO                             STR_OVERLAY_START " MPEG video" STR_HEADER_MARKER
#define STR_MSVC_STUB                              "M$ VC stub"
#define STR_MSVC_STUB_MFC                          "M$ VC stub (MFC)"
#define STR_MS_ACCESS_DATABASE                     STR_OVERLAY_START " M$ Access Database" STR_HEADER_MARKER
#define STR_MS_BITMAP_IMAGE                        STR_OVERLAY_START " M$ Bitmap image" STR_HEADER_MARKER
#define STR_MS_COMPRESS                            STR_OVERLAY_START " M$ Compress archver" STR_HEADER_MARKER
#define STR_MS_ENHANCED_METAFILE_IMAGE             STR_OVERLAY_START " M$ Enhanced Metafile image" STR_HEADER_MARKER
#define STR_MS_METAFILE_IMAGE                      STR_OVERLAY_START " M$ Metafile image" STR_HEADER_MARKER
#define STR_MS_PAINT_IMAGE                         STR_OVERLAY_START " M$ Paint image" STR_HEADER_MARKER
#define STR_MS_WIN_CABINET                         STR_OVERLAY_START " M$ Win Cabinet" STR_HEADER_MARKER
#define STR_MZ                                     STR_DANGEROUS " " STR_OVERLAY_START " MZ EXE-header marker"
#define STR_NATATA_EBOOK_COMPILER                  "NATATA eBook Compiler"
#define STR_NATATA_V_BOOK_EBOOK_COMPILER           "NATATA (V-Book) eBook Compiler"
#define STR_NEO_BOOK                               "Neo Book"
#define STR_NEXT_SUN_SOUND                         STR_OVERLAY_START " Next/Sun sound" STR_HEADER_MARKER
#define STR_OAKLEY_WEB_COMPILER                    "Oakley WebCompiler"
#define STR_OAZ_FAX_IMAGE                          STR_OVERLAY_START " OAZ Fax image" STR_HEADER_MARKER
#define STR_OS2_ICON_IMAGE                         STR_OVERLAY_START " OS/2 Icon image" STR_HEADER_MARKER
#define STR_OVERLAY_START                          "Overlay start with"
#define STR_PCI_ENC_CRYPTOR                        STR_OVERLAY_START " PCIENC Cryptor archver" STR_HEADER_MARKER
#define STR_PDF                                    STR_OVERLAY_START " Adobe PDF" STR_HEADER_MARKER
#define STR_PDF_11                                 STR_OVERLAY_START " Adobe PDF-1.1" STR_HEADER_MARKER
#define STR_PDF_12                                 STR_OVERLAY_START " Adobe PDF-1.2" STR_HEADER_MARKER
#define STR_PDF_13                                 STR_OVERLAY_START " Adobe PDF-1.3" STR_HEADER_MARKER
#define STR_PDF_14                                 STR_OVERLAY_START " Adobe PDF-1.4" STR_HEADER_MARKER
#define STR_PDF_15                                 STR_OVERLAY_START " Adobe PDF-1.5" STR_HEADER_MARKER
#define STR_PDF_16                                 STR_OVERLAY_START " Adobe PDF-1.6" STR_HEADER_MARKER
#define STR_PDF_17                                 STR_OVERLAY_START " Adobe PDF-1.7" STR_HEADER_MARKER
#define STR_PDF_2_EXE                              "PDF2EXE"
#define STR_PDS_IMAGE                              STR_OVERLAY_START " PDS image" STR_HEADER_MARKER
#define STR_PNG                                    STR_OVERLAY_START " PiNG image" STR_HEADER_MARKER
#define STR_PS                                     STR_OVERLAY_START " Abobe PS" STR_HEADER_MARKER
#define STR_PUBLICANT_EXE_BOOK                     "Publicant (ExeBook)"
#define STR_PYTHON_JUNK                            "Some Python Junk"
#define STR_QUICKLINK_II_FAX_IMAGE                 STR_OVERLAY_START " QuickLink II Fax image" STR_HEADER_MARKER
#define STR_RAR                                    "Rar archiver"
#define STR_RAR_SFX                                STR_RAR " SFX (Asm)"
#define STR_RAR_WIN32_SFX_320                      STR_RAR " Win32 SFX (Asm)"
#define STR_RAR_ZIP_SFX                            STR_RAR " (Zip Marker) SFX (Asm)"
#define STR_REAL_NETWORKS_SOUND_VIDEO              STR_OVERLAY_START " Real Networks sound/video" STR_HEADER_MARKER
#define STR_RIX_IMAGE                              STR_OVERLAY_START " RIX image" STR_HEADER_MARKER
#define STR_ROBO_HELP_OFFICE                       "RoboHELP Office"
#define STR_SA2_ADLIB_SOUND                        STR_OVERLAY_START " SA2 Adlib Module sound" STR_HEADER_MARKER
#define STR_SCI_FAX_IMAGE                          STR_OVERLAY_START " SciFax image" STR_HEADER_MARKER
#define STR_SECURE_BOOK_PRO                        "SecureBook Pro"
#define STR_SELF_MADE                              "Self-made"
#define STR_SGI_IMAGE                              STR_OVERLAY_START " SGI image" STR_HEADER_MARKER
#define STR_SIERRA_SOUND                           STR_OVERLAY_START " Sierra sound" STR_HEADER_MARKER
#define STR_SITE_IN_FILE_COMPILER                  "SiteInFile Compiler"
#define STR_SMART_INSTALL_MAKER                    "Smart Install Maker"
#define STR_SMART_PUB                              "SmartPub"
#define STR_SMK_VIDEO                              STR_OVERLAY_START " SMK video" STR_HEADER_MARKER
#define STR_SQZ                                    STR_OVERLAY_START " SQZ archver" STR_HEADER_MARKER
#define STR_STIRLING                               STR_OVERLAY_START " Stirling archver" STR_HEADER_MARKER
#define STR_SUNRAV_BOOK_OFFICE                     "SunRav BookOffice"
#define STR_SUN_ICON_IMAGE                         STR_OVERLAY_START " Sun Icon image" STR_HEADER_MARKER
#define STR_SUN_RASTER_IMAGE                       STR_OVERLAY_START " Sun Raster image" STR_HEADER_MARKER
#define STR_SWAG                                   STR_OVERLAY_START " SWAG archver" STR_HEADER_MARKER
#define STR_TEACH_SHOP                             "TeachShop Setup"
#define STR_TETERIN                                "Teterin's self-made e-book"
#define STR_TEX_NOTES_PRO                          "TexNotes Pro"
#define STR_TPACK_17                               STR_OVERLAY_START " TPACK archver" STR_HEADER_MARKER
#define STR_TRILOBYTE_ICON_IMAGE                   STR_OVERLAY_START " Trilobyte Icon image" STR_HEADER_MARKER
#define STR_TRILOBYTE_JPEG_IMAGE                   STR_OVERLAY_START " Trilobyte JPEG image" STR_HEADER_MARKER
#define STR_TRUEVISION_TARGA_IMAGE                 STR_OVERLAY_START " TrueVision Targa image" STR_HEADER_MARKER
#define STR_TXT_2_EXE                              "Txt-2-Exe"
#define STR_UFA                                    STR_OVERLAY_START " UFA archver" STR_HEADER_MARKER
#define STR_ULTRA_COMPRESSOR                       STR_OVERLAY_START " Ultra Compressor archver" STR_HEADER_MARKER
#define STR_UNKNOWN_ARCHIVER                       STR_OVERLAY_START " Unknown archver" STR_HEADER_MARKER
#define STR_US_PATENT_IMAGE                        STR_OVERLAY_START " US Patent image" STR_HEADER_MARKER
#define STR_VB_JUNK                                "Some VB Junk"
#define STR_VIRTUAL_IMAGE_MAKER_IMAGE              STR_OVERLAY_START " Virtual Image Maker image" STR_HEADER_MARKER
#define STR_VI_TEC_IMAGE                           STR_OVERLAY_START " VITec image" STR_HEADER_MARKER
#define STR_V_BOOK_COMPILER                        "V-Book Compiler"
#define STR_WEB_EXE                                "WebExe"
#define STR_WEB_EXE_ACME                           STR_WEB_EXE_CRACKED " * by team ACME"
#define STR_WEB_EXE_AZRAEL                         STR_WEB_EXE_CRACKED " * by AZRAEL"
#define STR_WEB_EXE_COA                            STR_WEB_EXE_CRACKED " * by L0RD KYR0N [COA]"
#define STR_WEB_EXE_CRACKED                        STR_WEB_EXE " * Cracked"
#define STR_WEB_PACKER                             "WebPacker"
#define STR_WEB_SITE_ZIP_PACKER                    "WebSiteZip Packer"
#define STR_WICAT_GED_IMAGE                        STR_OVERLAY_START " Wicat GED image" STR_HEADER_MARKER
#define STR_WIN_EBOOK_COMPILER                     "WinEbook Compiler"
#define STR_WORDPERFECT                            STR_OVERLAY_START " WordPerfect" STR_HEADER_MARKER
#define STR_WORDPERFECT_IMAGE                      STR_OVERLAY_START " WordPerfect image" STR_HEADER_MARKER
#define STR_XM_SOUND                               STR_OVERLAY_START " XM sound" STR_HEADER_MARKER
#define STR_XWD_IMAGE                              STR_OVERLAY_START " XWD image" STR_HEADER_MARKER
#define STR_X_2_NET_WEB_COMPILER                   "X2Net WebCompiler"
#define STR_X_READER                               "XReader"
#define STR_ZIP_LOCAL_HEADER                       STR_OVERLAY_START " Zip Local" STR_HEADER_MARKER
#define STR_ZIP_MARKER_DELPHI                      STR_OVERLAY_START " Zip Marker (BCB/Delphi)" STR_HEADER_MARKER
#define STR_ZIP_PYTHON_DISTUTILS                   "Python Distutils"
#define STR_ZIP_SFX_1                              STR_OVERLAY_START " Zip Marker (SFX)" STR_HEADER_MARKER
#define STR_ZIP_SFX_2                              STR_SELF_MADE " Zip SFX"
#define STR_ZIP_SFX_DISTUTILS                      STR_OVERLAY_START " " STR_ZIP_PYTHON_DISTUTILS STR_HEADER_MARKER
#define STR_ZM                                     STR_DANGEROUS " " STR_OVERLAY_START " ZM EXE-header marker"

///////////////////////////////////////////////////////////////////////////////

const DWORD    COMMENT_FLAG_7Z                     = 0x00000001;
const DWORD    COMMENT_FLAG_RAR                    = 0x00000002;
const DWORD    COMMENT_FLAG_ZIP                    = 0x00000004;
const DWORD    COMMENT_FLAG_ZIP_LOCAL_HEADER       = 0x00000008;

///////////////////////////////////////////////////////////////////////////////

enum E_DETECT_TYPE
{
   DT_NONE,
   DT_NORMAL,
   DT_HASH,
   DT_EXTRA
};

///////////////////////////////////////////////////////////////////////////////
// ---[ Should be sorted !!
///////////////////////////////////////////////////////////////////////////////

// bsearch returns a pointer to an occurrence of key in the array pointed to by base.
// If key is not found, the function returns NULL.
// If the array is not in ascending sort order or contains duplicate records
// with identical keys, the result is unpredictable.

// Should be in ascending sort order for binary search !
// Will be CRASH if not !
// !! Resort if smth modified !!

enum E_BOOK_TYPE
{
   ET_NONE,
   ET_UNKNOWN_EBOOK,
   ET_UNKNOWN_EXE_STUB,

// Sort from Here !

   ET_7Z,                                    // 7z
   ET_7Z_02,                                 // 7z 0.2
   ET_7Z_03,                                 // 7z 0.3
   ET_7Z_STUB_1,                             // 7z SFX
   ET_7Z_STUB_2,                             // 7z SFX
   ET_ACE,
   ET_ACKER_PACK,                            // AckerPack
   ET_ACKER_PACK_121,                        // AckerPack 121
   ET_ACTIV_EBOOK_COMPILER,                  // Activ E-Book Compiler
   ET_ACTIV_EBOOK_COMPILER_302,              // Activ E-Book Compiler 3.02
   ET_ACTIV_EBOOK_COMPILER_401,              // Activ E-Book Compiler 4.01
   ET_ACTIV_EBOOK_COMPILER_422,              // Activ E-Book Compiler 4.22
   ET_ACTIV_EBOOK_COMPILER_REBRANDER,        // Activ E-Book Compiler Rebrander
   ET_ADLIB_SAMPLE_SOUND,
   ET_ADOBE_PHOTO_SHOP_IMAGE,
   ET_ALPHA_BMP_IMAGE,
   ET_AMGC,
   ET_ANTECHINUS_EBOOK_WIZARD,               // Antechinus eBook Wizard 3.x
   ET_ANTECHINUS_EBOOK_WIZARD_31,            // Antechinus eBook Wizard 3.1
   ET_ANTECHINUS_EBOOK_WIZARD_32,            // Antechinus eBook Wizard 3.2
   ET_ARAFASOFT_EBOOK_CREATOR,               // Arafasoft E-Book Creator 1.x
   ET_ASYLUM_SOUND,
   ET_AUTO_LOGIC_IMAGE,
   ET_AVHRR_IMAGE,
   ET_AVI,                                   // AVI
   ET_BAFF_BMP_LIB_IMAGE,
   ET_BOOK_BIZ_MASTER_20,                    // BookBiz Master 2.0
   ET_BOOK_BIZ_MASTER_20_ENCRYPTED,          // BookBiz Master 2.0 * Encrypted
   ET_CALS_RASTER_IMAGE,
   ET_CGM_IMAGE,
   ET_CHM,                                   // CHM
   ET_COINSOFT_COPYRIGHT,                    // Coinsoft EBook
   ET_COINSOFT_EBOOK_MAKER,                  // Coinsoft EBook Maker 1.0
   ET_COINSOFT_EBOOK_MAKER_1,                // Coinsoft EBook Maker 1.0 <1>
   ET_COINSOFT_EBOOK_MAKER_2,                // Coinsoft EBook Maker 1.0 <2>
   ET_COINSOFT_EBOOK_PROTECT_MASTER,         // Coinsoft EBook Protect Master 3.0
   ET_COREL_PHOTO_PAINT_IMAGE,
   ET_CREATIVE_SOUND,
   ET_CUBICOMP_PICTUREMAKER_GREEN_IMAGE,
   ET_CUBICOMP_PICTUREMAKER_RED_IMAGE,
   ET_CUBICOMP_PICTURE_MAKER_BLUE_IMAGE,
   ET_CUPDAT,                                //
   ET_CYBER_ARTICLE_PRO,                     // CyberArticle Pro
   ET_DATA_BEAM_IMAGE,
   ET_DESKTOP_AUTHOR,                        // DesktopAuthor
   ET_DR_EXPLAIN,                            // Dr. Explain
   ET_DR_HALO_IMAGE,
   ET_EBOOKS_COMPILER,                       // eBooks Compiler
   ET_EBOOKS_WRITER,                         // eBooksWriter
   ET_EBOOK_BLASTER,                         // eBook Blaster
   ET_EBOOK_BUILDER,                         // eBook Builder
   ET_EBOOK_EDIT_PRO_3200,                   // e-Book Edit Pro 3.200
   ET_EBOOK_EDIT_PRO_3210,                   // e-Book Edit Pro 3.210
   ET_EBOOK_EDIT_PRO_3210_1,                 // e-Book Edit Pro 3.210
   ET_EBOOK_EDIT_PRO_3210_NP_1,              // e-Book Edit Pro 3.210
   ET_EBOOK_EDIT_PRO_3210_NP_2,              // e-Book Edit Pro 3.210
   ET_EBOOK_EDIT_PRO_3210_NP_3,              // e-Book Edit Pro 3.210
   ET_EBOOK_EDIT_PRO_3300,                   // e-Book Edit Pro 3.300
   ET_EBOOK_EDIT_PRO_3310,                   // e-Book Edit Pro 3.310
   ET_EBOOK_EDIT_PRO_3310_1,                 // e-Book Edit Pro 3.310
   ET_EBOOK_EDIT_PRO_3310_2,                 // e-Book Edit Pro 3.310
   ET_EBOOK_EDIT_PRO_3310_NP,                // e-Book Edit Pro 3.310 Not Packed
   ET_EBOOK_EDIT_PRO_3340_1,                 // e-Book Edit Pro 3.340
   ET_EBOOK_EDIT_PRO_3340_2,                 // e-Book Edit Pro 3.340
   ET_EBOOK_EDIT_PRO_3341,                   // e-Book Edit Pro 3.341
   ET_EBOOK_EDIT_PRO_3X,                     // e-Book Edit Pro 3.x
   ET_EBOOK_EDIT_PRO_OLD,                    // e-Book Edit Pro (Very Old)
   ET_EBOOK_GOLD_30,                         // eBookGold 3.x
   ET_EBOOK_HTML_COMPILER_PRO,               // eBook HTML Compiler Pro
   ET_EBOOK_MAESTRO,                         // eBook Maestro
   ET_EBOOK_MAESTRO_FREE_1,                  // eBook Maestro Free <1>
   ET_EBOOK_MAESTRO_FREE_2,                  // eBook Maestro Free <2>
   ET_EBOOK_MAESTRO_FREE_3,                  // eBook Maestro Free <3>
   ET_EBOOK_MAESTRO_PRO,                     // eBook Maestro Pro
   ET_EBOOK_MAESTRO_PRO_DEMO,                // eBook Maestro Pro (Demo)
   ET_EBOOK_MAKER_21,                        // eBook Maker 2.1
   ET_EBOOK_PACK_EXPRESS,                    // eBook Pack Express
   ET_EBOOK_PDFWRITER,                       // eBook pdfWriter
   ET_EBOOK_PRODUCER,                        // eBook Producer
   ET_EBOOK_PUBLICANT,                       // E-Book Publicant
   ET_EBOOK_SNAP,                            // eBookSnap
   ET_EBOOK_STARTER,                         // eBook Starter
   ET_EBOOK_WORKSHOP,                        // eBook Workshop
   ET_EMAG_CREATOR,                          // eMagCreator
   ET_EMZ,                                   // EMZ
   ET_EPSF_IMAGE,
   ET_ERDAS_LAN_GIS_IMAGE,
   ET_EVRSOFT_FIRST_PAGE,                    // Evrsoft First Page
   ET_EXE_BOOK_SELF_PUBLISHER,               // ExeBook Self-Publisher
   ET_EXE_EBOOK_CREATOR_FOR_TREEPAD,         // exe-eBook creator for TreePad
   ET_EXE_OUTPUT_FOR_PHP,                    // ExeOutput for PHP
   ET_E_DITOR_2,                             // E-ditor eBook Compiler 2.0
   ET_E_DITOR_2_1,                           // E-ditor eBook Compiler 2.0
   ET_E_DITOR_2_2,                           // E-ditor eBook Compiler 2.0
   ET_FAKE_PAY_ARCHIVE_1,                    // Fake archive. SMS extortioner
   ET_FAKE_PAY_ARCHIVE_2,                    // Fake archive. SMS extortioner
   ET_FAKE_PAY_ARCHIVE_3,                    // Fake archive. SMS extortioner
   ET_FAKE_PAY_ARCHIVE_4,                    // Fake archive. SMS extortioner
   ET_FAKE_PAY_ARCHIVE_5,                    // Fake archive. SMS extortioner
   ET_FAKE_PAY_ARCHIVE_6,                    // Fake archive. SMS extortioner
   ET_FAKE_PAY_ARCHIVE_7,                    // Fake archive. SMS extortioner
   ET_FAR_HTML,                              // FAR HTML
   ET_FAST_EBOOK_COMPILER,                   // Fast eBook Compiler
   ET_FAST_EBOOK_COMPILER_14,                // Fast eBook Compiler 1.4x
   ET_FAST_EBOOK_COMPILER_1X,                // Fast eBook Compiler 1.x
   ET_FIZ,
   ET_GEM_IMAGE,
   ET_GIF87,                                 //
   ET_GIF89,                                 //
   ET_GOES_IMAGE,
   ET_GZIP,
   ET_HELPINATOR,                            // Helpinator
   ET_HELP_AND_MANUAL,                       // Help & Manual
   ET_HELP_MAKER,                            // HelpMaker
   ET_HIJAAK_IMAGE_DRAW_IMAGE,
   ET_HISTOGRAM_IMAGE,
   ET_HITACHI_RASTER_IMAGE,
   ET_HPACK,
   ET_HP_48_SX_IMAGE,
   ET_HSC_SOUND,
   ET_HSI_JPEG_IMAGE,
   ET_HSI_RAW_IMAGE,
   ET_HTML_2_EXE,                            // html2exe
   ET_HTML_EBOOK_MAKER_AND_ENCRYPTER,        // HTML eBook Maker and Encrypter
   ET_HTML_EXECUTABLE,                       // HTML Executable 3.x
   ET_HYPER_MAKER,                           // HyperMaker
   ET_HYPER_PUBLISH_PRO,                     // HyperPublish Pro
   ET_IBM_IOCA_IMAGE,
   ET_IDSKA32,
   ET_IMAGING_TECHNOLOGY_IMAGE,
   ET_IMG_SOFTWARE_SET_IMAGE,
   ET_INSET_SYSTEMS_IGF_IMAGE,
   ET_INSTALL_SHIELD,
   ET_INTEL_DCX_IMAGE,
   ET_INTERPLAY_MVE,
   ET_INTER_GRAPH_IMAGE,
   ET_JANSOFT_SBOOKBUILDER,                  // JanSoft SbookBuilder
   ET_JAR,
   ET_JFIF_1,                                // JFIF
   ET_JFIF_2,                                // JFIF
   ET_JIMMY_EBOOK_CREATOR,                   // Jimmy Brown E-Book Creator
   ET_JIMMY_EBOOK_CREATOR_2000,              // Jimmy Brown E-Book Creator 2000
   ET_JIMMY_EBOOK_CREATOR_2000_1,            // Jimmy Brown E-Book Creator 2000 <1>
   ET_JIMMY_EBOOK_CREATOR_2000_2,            // Jimmy Brown E-Book Creator 2000 <2>
   ET_JIMMY_EBOOK_CREATOR_2000_3,            // Jimmy Brown E-Book Creator 2000 <3>
   ET_JIMMY_EBOOK_CREATOR_2000_4,            // Jimmy Brown E-Book Creator 2000 <4>
   ET_JRC,
   ET_KE_EBOOK_CREATOR,                      // KeeBook Creator
   ET_KOFAX_GROUP_4_IMAGE,
   ET_LEITHAUSER,                            // Leithauser Research Ebook setup
   ET_LOTUS_IMAGE,
   ET_LTC_EBOOK_CREATOR,                     // LTC eBook Creator
   ET_MACROMEDIA_SHOCKWAVE_1_VIDEO,
   ET_MACROMEDIA_SHOCKWAVE_2_VIDEO,
   ET_MAC_PAINT_IMAGE,
   ET_MAC_PICT_IMAGE,
   ET_MAC_STUFFIT,
   ET_MICROGRAFIX_DRAW_IMAGE,
   ET_MPEG_1,
   ET_MPEG_2,
   ET_MPEG_VIDEO,
   ET_MS_ACCESS_DATABASE,
   ET_MS_BITMAP_IMAGE,
   ET_MS_COMPRESS,
   ET_MS_ENHANCED_METAFILE_IMAGE,
   ET_MS_METAFILE_IMAGE,
   ET_MS_PAINT_IMAGE,
   ET_MS_WIN_CABINET,
   ET_MZ_1,                                  // MZ
   ET_MZ_2,                                  // MZ
   ET_MZ_3,                                  // MZKERNEL32.DLL
   ET_NATATA_EBOOK_COMPILER_2004,            // NATATA eBook Compiler (2004)
   ET_NATATA_EBOOK_COMPILER_2005,            // NATATA eBook Compiler (2005)
   ET_NATATA_EBOOK_COMPILER_2009,            // NATATA eBook Compiler (2009)
   ET_NATATA_EBOOK_COMPILER_2010,            // NATATA eBook Compiler (2010)
   ET_NATATA_EBOOK_COMPILER_21,              // NATATA eBook Compiler 2.1 [1]
   ET_NATATA_EBOOK_COMPILER_21_2,            // NATATA eBook Compiler 2.1 [2]
   ET_NATATA_EBOOK_COMPILER_21_DEMO,         // NATATA eBook Compiler 2.1 (Demo)
   ET_NATATA_EBOOK_COMPILER_30,              // NATATA eBook Compiler 3.0
   ET_NATATA_V_BOOK_EBOOK_COMPILER,          // NATATA (V-Book) eBook Compiler
   ET_NEO_BOOK,                              // Neo Book
   ET_NEXT_SUN_SOUND,
   ET_OAKLEY_WEB_COMPILER,                   // Oakley WebCompiler
   ET_OAZ_FAX_IMAGE,
   ET_OS2_ICON_IMAGE,
   ET_PCI_ENC_CRYPTOR,
   ET_PDF,                                   // %PDF
   ET_PDF_11,                                // %PDF-1.1
   ET_PDF_12,                                // %PDF-1.2
   ET_PDF_13,                                // %PDF-1.3
   ET_PDF_14,                                // %PDF-1.4
   ET_PDF_15,                                // %PDF-1.5
   ET_PDF_16,                                // %PDF-1.6
   ET_PDF_17,                                // %PDF-1.7
   ET_PDF_2_EXE,                             // PDF2EXE
   ET_PDS_IMAGE,
   ET_PNG,                                   // PNG
   ET_PS,                                    // %!PS-Adobe-
   ET_PS_20,                                 // %!PS-Adobe-2.0
   ET_PS_30,                                 // %!PS-Adobe-2.0
   ET_PUBLICANT_EXE_BOOK,                    // Publicant (ExeBook)
   ET_PUBLICANT_EXE_BOOK_01,                 // Publicant (ExeBook) <01>
   ET_PUBLICANT_EXE_BOOK_02,                 // Publicant (ExeBook) <02>
   ET_PUBLICANT_EXE_BOOK_03,                 // Publicant (ExeBook) <03>
   ET_PUBLICANT_EXE_BOOK_04,                 // Publicant (ExeBook) <04>
   ET_PUBLICANT_EXE_BOOK_05,                 // Publicant (ExeBook) <05>
   ET_PUBLICANT_EXE_BOOK_06,                 // Publicant (ExeBook) <06>
   ET_PUBLICANT_EXE_BOOK_07,                 // Publicant (ExeBook) <07>
   ET_PUBLICANT_EXE_BOOK_08,                 // Publicant (ExeBook) <08>
   ET_PUBLICANT_EXE_BOOK_09,                 // Publicant (ExeBook) <09>
   ET_PUBLICANT_EXE_BOOK_10,                 // Publicant (ExeBook) <10>
   ET_PUBLICANT_EXE_BOOK_2D,                 // Publicant (ExeBook) <$2D>
   ET_PUBLICANT_EXE_BOOK_2E,                 // Publicant (ExeBook) <$2E>
   ET_PUBLICANT_EXE_BOOK_31,                 // Publicant (ExeBook) <$31>
   ET_PUBLICANT_EXE_BOOK_38,                 // Publicant (ExeBook) <$38>
   ET_PUBLICANT_EXE_BOOK_39,                 // Publicant (ExeBook) <$39>
   ET_PUBLICANT_EXE_BOOK_3A,                 // Publicant (ExeBook) <$3A>
   ET_PUBLICANT_EXE_BOOK_3C,                 // Publicant (ExeBook) <$3C>
   ET_QUICKLINK_II_FAX_IMAGE,
   ET_RAR,                                   // RAR
   ET_RAR_40,                                // RAR
   ET_RAR_SFX,                               // RAR SFX (Asm)
   ET_RAR_WIN32_SFX_320,                     // RAR Win32 SFX 3.20
   ET_RAR_ZIP_SFX_1,                         // RAR SFX (Asm)
   ET_RAR_ZIP_SFX_2,                         // RAR SFX (Asm)
   ET_RAR_ZIP_SFX_3,                         // RAR SFX (Asm)
   ET_RAR_ZIP_SFX_4,                         // RAR SFX (Asm)
   ET_RAR_ZIP_SFX_5,                         // RAR SFX (Asm)
   ET_REAL_NETWORKS_SOUND_VIDEO,
   ET_RIX_IMAGE,
   ET_ROBO_HELP_OFFICE,                      // RoboHELP Office
   ET_SA2_ADLIB_SOUND,
   ET_SCI_FAX_IMAGE,
   ET_SECURE_BOOK_PRO,                       // SecureBook Pro
   ET_SGI_IMAGE,
   ET_SIERRA_SOUND,
   ET_SITE_IN_FILE_COMPILER,                 // SiteInFile Compiler
   ET_SMART_INSTALL_MAKER,                   // Smart Install Maker
   ET_SMART_PUB,                             // SmartPub
   ET_SMK_VIDEO,
   ET_SQZ,
   ET_STIRLING,
   ET_SUNRAV_BOOK_OFFICE,                    // SunRav BookOffice
   ET_SUN_ICON_IMAGE,
   ET_SUN_RASTER_IMAGE,
   ET_SWAG,
   ET_TEACH_SHOP,                            // TeachShop Setup
   ET_TETERIN,
   ET_TEX_NOTES_PRO,                         // TexNotes Pro
   ET_TPACK_17,
   ET_TRILOBYTE_ICON_IMAGE,
   ET_TRILOBYTE_JPEG_IMAGE,
   ET_TRUEVISION_TARGA_IMAGE,
   ET_TXT_2_EXE,                             // Txt-2-Exe
   ET_UFA,
   ET_ULTRA_COMPRESSOR,
   ET_US_PATENT_IMAGE,
   ET_VIRTUAL_IMAGE_MAKER_IMAGE,
   ET_VI_TEC_IMAGE,
   ET_V_BOOK_COMPILER,                       // V-Book Compiler
   ET_WAW,
   ET_WEB_EXE,                               // WebExe
   ET_WEB_EXE_01,                            // WebExe
   ET_WEB_EXE_03,                            // WebExe
   ET_WEB_EXE_04,                            // WebExe
   ET_WEB_EXE_2000_1,                        // WebExe
   ET_WEB_EXE_2000_2,                        // WebExe
   ET_WEB_EXE_ACME,                          // WebExe
   ET_WEB_EXE_AZRAEL,                        // WebExe
   ET_WEB_EXE_COA,                           // WebExe
   ET_WEB_EXE_CRACKED,                       // WebExe
   ET_WEB_EXE_CRACKED_2,                     // WebExe
   ET_WEB_EXE_CRACKED_3,                     // WebExe
   ET_WEB_PACKER,                            // WebPacker
   ET_WEB_SITE_ZIP_PACKER,                   // WebSiteZip Packer
   ET_WICAT_GED_IMAGE,
   ET_WIN_EBOOK_COMPILER,                    // WinEbook Compiler
   ET_WORDPERFECT,
   ET_WORDPERFECT_IMAGE,
   ET_XM_SOUND,
   ET_XWD_IMAGE,
   ET_X_2_NET_WEB_COMPILER,                  // X2Net WebCompiler
   ET_X_READER,                              // XReader
   ET_ZIP_BCB_DELPHI,                        //                                                                                                       STR_ZIP_SFX_DELPHI,                       "",            ""                }
   ET_ZIP_DELPHI,                            // ZIP (Delphi)
   ET_ZIP_LOCAL_HEADER,                      //                                                                                                      STR_ZIP_DELPHI,                           "",            ""                },
   ET_ZIP_SFX_1,                             // ZIP SFX
   ET_ZIP_SFX_2,
   ET_ZIP_SFX_DISTUTILS,
   ET_ZIP_SFX_DISTUTILS_264,
   ET_ZM                                     // ZM
};

// Should be in ascending sort order for binary search !
// Will be CRASH if not !
// !! Resort if smth modified !!

///////////////////////////////////////////////////////////////////////////////
// ---] Should be sorted !!
///////////////////////////////////////////////////////////////////////////////

struct SSF_TEXT_RECORD
{
   bool              _bUnpack;
   bool              _bDetect;
   bool              _bPrint;
   BYTE              _DetectType;   // E_DETECT_TYPE
   E_BOOK_TYPE       _BookType;
   char              _pszSignature [MAX_SIGN_SIZE + 1];
   BYTE              _SignHint;     // SSF_HINT
   short             _iSignHintOfs;
   char              _pszHash      [MD5_SIGNATURE_SIZE];
   BYTE              _bySectNum;   // One based, 0 for None.
   char              _pszName      [MAX_NAME_SIZE + 1];
   char              _pszVersion   [MAX_VERS_SIZE + 1];
   char              _pszComment   [MAX_COMMENT_SIZE + 1];
   char              _pszAppendix  [MAX_APPENDIX_SIZE + 1];
   short             _iFoundAt;
};

const int   E_TEXT_DB_SIZE = 249;

///////////////////////////////////////////////////////////////////////////////
// ---[ Should be sorted !!
///////////////////////////////////////////////////////////////////////////////

// bsearch returns a pointer to an occurrence of key in the array pointed to by base.
// If key is not found, the function returns NULL.
// If the array is not in ascending sort order or contains duplicate records
// with identical keys, the result is unpredictable.

// Should be in ascending sort order for binary search !
// Will be CRASH if not !
// !! Resort if smth modified !!
SSF_TEXT_RECORD   pTextDB[E_TEXT_DB_SIZE] =
{
// UNP DET  PR DET_TYPE    ID                                        SIGNATURE                     SIGN. HINT           OFS   HASH                                   SECT  NAME                                      VER            STUB                    PACKER
   {  0, 0, 0, DT_NONE,    ET_NONE,                                  "",                           HINT_NONE,             0,  "",                                    0,    "Isn't an e-book",                        "",            "",                  ""          },
   {  0, 0, 0, DT_NONE,    ET_UNKNOWN_EBOOK,                         "",                           HINT_NONE,             0,  "",                                    0,    "Unknown e-book type",                    "",            "",                  ""          },
   {  0, 0, 0, DT_NONE,    ET_UNKNOWN_EXE_STUB,                      "",                           HINT_NONE,             0,  "",                                    0,    "Unknown EXE-stub type",                  "",            "",                  ""          },
// UNP DET  PR DET_TYPE    ID                                        SIGNATURE                     SIGN. HINT           OFS   HASH                                   SECT  NAME                                      VER            STUB                    PACKER
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
// UNP DET  PR DET_TYPE    ID                                        SIGNATURE                     SIGN. HINT           OFS   HASH                                   SECT  NAME                                      VER            STUB                    PACKER
   {  0, 0, 0, DT_NONE,    ET_CYBER_ARTICLE_PRO,                                                                                                                           STR_CYBER_ARTICLE_PRO,                    "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_DESKTOP_AUTHOR,                                                                                                                              STR_DESKTOP_AUTHOR,                       "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_DR_EXPLAIN,                                                                                                                                  STR_DR_EXPLAIN,                           "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EBOOKS_COMPILER,                                                                                                                             STR_EBOOKS_COMPILER,                      "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EBOOK_BLASTER,                                                                                                                               STR_EBOOK_BLASTER,                        "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EBOOK_BUILDER,                                                                                                                               STR_EBOOK_BUILDER,                        "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EBOOK_HTML_COMPILER_PRO,                                                                                                                     STR_EBOOK_HTML_COMPILER_PRO,              "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EBOOK_PACK_EXPRESS,                                                                                                                          STR_EBOOK_PACK_EXPRESS,                   "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EBOOK_PDFWRITER,                                                                                                                             STR_EBOOK_PDF_WRITER,                     "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EBOOK_PRODUCER,                                                                                                                              STR_EBOOK_PRODUCER,                       "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EBOOK_SNAP,                                                                                                                                  STR_EBOOK_SNAP,                           "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EBOOK_STARTER,                                                                                                                               STR_EBOOK_STARTER,                        "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EBOOK_WORKSHOP,                                                                                                                              STR_EBOOK_WORKSHOP,                       "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EMAG_CREATOR,                                                                                                                                STR_EMAG_CREATOR,                         "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EVRSOFT_FIRST_PAGE,                                                                                                                          STR_EVRSOFT_FIRST_PAGE,                   "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EXE_BOOK_SELF_PUBLISHER,                                                                                                                     STR_EXE_BOOK_SELF_PUBLISHER,              "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EXE_EBOOK_CREATOR_FOR_TREEPAD,                                                                                                               STR_EXE_EBOOK_CREATOR_FOR_TREEPAD,        "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_EXE_OUTPUT_FOR_PHP,                                                                                                                          STR_EXE_OUTPUT_FOR_PHP,                   "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_FAR_HTML,                                                                                                                                    STR_FAR_HTML,                             "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_HELPINATOR,                                                                                                                                  STR_HELPINATOR,                           "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_HELP_MAKER,                                                                                                                                  STR_HELP_MAKER,                           "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_HTML_2_EXE,                                                                                                                                  STR_HTML_2_EXE,                           "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_HTML_EBOOK_MAKER_AND_ENCRYPTER,                                                                                                              STR_HTML_EBOOK_MAKER_AND_ENCRYPTER,       "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_HYPER_MAKER,                                                                                                                                 STR_HYPER_MAKER,                          "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_HYPER_PUBLISH_PRO,                                                                                                                           STR_HYPER_PUBLISH_PRO,                    "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_LTC_EBOOK_CREATOR,                                                                                                                           STR_LTC_EBOOK_CREATOR,                    "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_NEO_BOOK,                                                                                                                                    STR_NEO_BOOK,                             "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_PDF_2_EXE,                                                                                                                                   STR_PDF_2_EXE,                            "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_ROBO_HELP_OFFICE,                                                                                                                            STR_ROBO_HELP_OFFICE,                     "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_SECURE_BOOK_PRO,                                                                                                                             STR_SECURE_BOOK_PRO,                      "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_SITE_IN_FILE_COMPILER,                                                                                                                       STR_SITE_IN_FILE_COMPILER,                "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_SMART_PUB,                                                                                                                                   STR_SMART_PUB,                            "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_SUNRAV_BOOK_OFFICE,                                                                                                                          STR_SUNRAV_BOOK_OFFICE,                   "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_TEX_NOTES_PRO,                                                                                                                               STR_TEX_NOTES_PRO,                        "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_TXT_2_EXE,                                                                                                                                   STR_TXT_2_EXE,                            "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_WEB_PACKER,                                                                                                                                  STR_WEB_PACKER,                           "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_WEB_SITE_ZIP_PACKER,                                                                                                                         STR_WEB_SITE_ZIP_PACKER,                  "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_WIN_EBOOK_COMPILER,                                                                                                                          STR_WIN_EBOOK_COMPILER,                   "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_X_2_NET_WEB_COMPILER,                                                                                                                        STR_X_2_NET_WEB_COMPILER,                 "",            ""                },
   {  0, 0, 0, DT_NONE,    ET_X_READER,                                                                                                                                    STR_X_READER,                             "",            ""                },
// UNP DET  PR DET_TYPE    ID                                        SIGNATURE                     SIGN. HINT           OFS   HASH                                   SECT  NAME                                      VER            STUB                    PACKER
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Sort from Here !
                           
// UNP DET  PR DET_TYPE    ID                                        SIGNATURE                           SIGN. HINT            OFS  HASH                                   SECT  NAME                                      VER            STUB                    PACKER
{  0,    1, 0, DT_NORMAL,  ET_7Z,                                    "377ABCAF271C",                     HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_7Z,                                   "",            "",                     "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_7Z_02,                                 "377ABCAF271C0002",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_7Z,                                   "0.2",         "",                     "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_7Z_03,                                 "377ABCAF271C0003",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_7Z,                                   "0.3",         "",                     "",         0  },
{  0,    1, 0, DT_HASH,    ET_7Z_STUB_1,                             "",                                 HINT_NONE,              0, "3F5EFC4663FA97A47C59C97F3103F68E",    1,    STR_7Z_STUB_1,                            "1.2.0.715",   STR_MSVC_STUB,          "",         0  },
{  0,    1, 0, DT_HASH,    ET_7Z_STUB_2,                             "",                                 HINT_NONE,              0, "5E65187EB680E4AE9DEE8341908DAD77",    1,    STR_7Z_STUB_2,                            "4.42",        STR_DELPI_JUNK,         "",         0  }, // with 7zLib DLL
{  0,    1, 0, DT_NORMAL,  ET_ACE,                                   "2A2A4143452A2A",                   HINT_OVERLAY_BEGIN,     7, "",                                    0,    STR_ACE,                                  "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 1, DT_EXTRA,   ET_ACKER_PACK,                            "425A6839314159265359",             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ACKER_PACK,                           "",            "",                     "",         0  }, // BZh9
{  0,    1, 0, DT_HASH,    ET_ACKER_PACK_121,                        "",                                 HINT_NONE,              0, "D6830F4175E97DC1007CE18FCC44A47F",    1,    STR_ACKER_PACK,                           "121",         "",                     "ASPack",   0  }, // BZh9
{  0,    1, 1, DT_NORMAL,  ET_ACTIV_EBOOK_COMPILER,                  "AB0BA1CB",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ACTIV_EBOOK_COMPILER,                 "",            STR_MSVC_STUB_MFC,      "",         0  },
{  0,    1, 0, DT_HASH,    ET_ACTIV_EBOOK_COMPILER_302,              "",                                 HINT_NONE,              0, "3297713011DA461CC6A29A6DC19AE759",    1,    STR_ACTIV_EBOOK_COMPILER,                 "3.02",        STR_MSVC_STUB_MFC,      "",         0  },
{  0,    1, 0, DT_HASH,    ET_ACTIV_EBOOK_COMPILER_401,              "",                                 HINT_NONE,              0, "373442A70FF30BDEA5EC5F355832F060",    1,    STR_ACTIV_EBOOK_COMPILER,                 "4.01",        STR_MSVC_STUB_MFC,      "",         0  },
{  0,    1, 0, DT_HASH,    ET_ACTIV_EBOOK_COMPILER_422,              "",                                 HINT_NONE,              0, "89C426FB5240425E86456FD251A2DFA3",    1,    STR_ACTIV_EBOOK_COMPILER,                 "4.22",        STR_MSVC_STUB_MFC,      "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_ACTIV_EBOOK_COMPILER_REBRANDER,        "5ABEB070",                         HINT_OVERLAY_BEGIN,     0, "D6C7E3BBAB967FD95C5FED186B646489",    1,    STR_ACTIV_EBOOK_COMPILER_REBRANDER,       "",            "",                     "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_ADLIB_SAMPLE_SOUND,                    "474F4C442053414D504C45",           HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ADLIB_SAMPLE_SOUND,                   "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_ADOBE_PHOTO_SHOP_IMAGE,                "3842505300010000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ADOBE_PHOTO_SHOP_IMAGE,               "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_ALPHA_BMP_IMAGE,                       "FFFF000164000000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ALPHA_BMP_IMAGE,                      "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_AMGC,                                  "AD362200",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_AMGC,                                 "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 1, DT_HASH,    ET_ANTECHINUS_EBOOK_WIZARD_32,            "",                                 HINT_NONE,              0, "701E8848FAC2FBC321DD2C3FD6BD30B5",    1,    STR_ANTECHINUS_EBOOK_WIZARD,              "3.2",         STR_MSVC_STUB_MFC,      "",         0  },
{  0,    1, 1, DT_NORMAL,  ET_ARAFASOFT_EBOOK_CREATOR,               "2815CCFF2815CCFF2815CCFF",         HINT_OVERLAY_BEGIN,     0, "CEF733AB4C46297099F201D0144ED18B",    1,    STR_ARAFASOFT_EBOOK_CREATOR,              "1.x",         STR_VB_JUNK,            "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_ASYLUM_SOUND,                          "4153594C554D204D7573696320466F72", HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ASYLUM_SOUND,                         "1.0",         "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_AUTO_LOGIC_IMAGE,                      "FF040007",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_AUTO_LOGIC_IMAGE,                     "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_AVHRR_IMAGE,                           "D5C8000100030001",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_AVHRR_IMAGE,                          "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_AVI,                                   "52494646",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_AVI,                                  "",            "",                     "",         0  }, // RIFF
{  0,    1, 0, DT_NORMAL,  ET_BAFF_BMP_LIB_IMAGE,                    "4241464601000000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_BAFF_BMP_LIB_IMAGE,                   "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 1, DT_HASH,    ET_BOOK_BIZ_MASTER_20,                    "",                                 HINT_NONE,              0, "BC785DE33D9E15085B5FD447FD988834",    1,    STR_BOOK_BIZ_MASTER,                      "2.0",         STR_DELPI_JUNK,         "",         0  },
{  0,    1, 1, DT_HASH,    ET_BOOK_BIZ_MASTER_20_ENCRYPTED,          "",                                 HINT_NONE,              0, "56782FA2D10815ABC7F1012C2EEA75E1",    1,    STR_BOOK_BIZ_MASTER_ENCRYPTED,            "2.0",         STR_DELPI_JUNK,         "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_CALS_RASTER_IMAGE,                     "737263646F6369643A20",             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_CALS_RASTER_IMAGE,                    "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_CGM_IMAGE,                             "002A0848694A61616B2032",           HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_CGM_IMAGE,                            "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_CHM,                                   "49545346030000006000000001000000", HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_CHM,                                  "",            "",                     "",         0  }, // ITSF
{  0,    1, 0, DT_NORMAL,  ET_COINSOFT_COPYRIGHT,                    "436F70797269676874",               HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_COINSOFT_COPYRIGHT,                   "",            STR_DELPI_JUNK,         "",         0  }, // Copyright
{  0,    1, 1, DT_NORMAL,  ET_COINSOFT_EBOOK_MAKER,                  "534F2123",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_COINSOFT_EBOOK_MAKER,                 "1.0",         STR_DELPI_JUNK,         "",         0  }, // SO!#
{  0,    1, 0, DT_HASH,    ET_COINSOFT_EBOOK_MAKER_1,                "",                                 HINT_NONE,              0, "155305BE7C0E349F9E936B00F0DFC1F2",    1,    STR_COINSOFT_EBOOK_MAKER,                 "1.0 <1>",     STR_DELPI_JUNK,         "",         0  }, // SO!#
{  0,    1, 0, DT_HASH,    ET_COINSOFT_EBOOK_MAKER_2,                "",                                 HINT_NONE,              0, "5E420463949AB72E57DF92A8D1A2E7C6",    1,    STR_COINSOFT_EBOOK_MAKER,                 "1.0 <2>",     STR_DELPI_JUNK,         "",         0  }, // SO!#
{  0,    1, 0, DT_NONE,    ET_COINSOFT_EBOOK_PROTECT_MASTER,         "",                                 HINT_NONE,              0, "8871C217EC27EA5C17F55665E0B8B6D1",    1,    STR_COINSOFT_EBOOK_PROTECT_MASTER,        "3.0",         STR_DELPI_JUNK,         "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_COREL_PHOTO_PAINT_IMAGE,               "4350543746494C45",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_COREL_PHOTO_PAINT_IMAGE,              "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_CREATIVE_SOUND,                        "437265617469766520566F6963652046", HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_CREATIVE_SOUND,                       "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_CUBICOMP_PICTUREMAKER_GREEN_IMAGE,     "260CFF0200000000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_CUBICOMP_PICTURE_MAKER_IMAGE,         "<green>",     "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_CUBICOMP_PICTUREMAKER_RED_IMAGE,       "160CFF0200000000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_CUBICOMP_PICTURE_MAKER_IMAGE,         "<red>",       "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_CUBICOMP_PICTURE_MAKER_BLUE_IMAGE,     "360CFF0200000000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_CUBICOMP_PICTURE_MAKER_IMAGE,         "<blue>",      "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_CUPDAT,                                "00000200FFFF01000A00435570646174", HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_GENERIC_COMPILER,                     "",            STR_MSVC_STUB_MFC,      "",         0  }, // ....ÿÿ....CUpdat
{  0,    1, 0, DT_NORMAL,  ET_DATA_BEAM_IMAGE,                       "30312E3031000004",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_DATA_BEAM_IMAGE,                      "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_DR_HALO_IMAGE,                         "4148E30000000A00",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_DR_HALO_IMAGE,                        "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 1, DT_EXTRA,   ET_EBOOKS_WRITER,                         "",                                 HINT_OVERLAY_BEGIN,     8, "",                                    0,    STR_EBOOKS_WRITER,                        "",            "",                     "",         0  }, // HINT = 8 for Extra check !
{  1,    1, 0, DT_NORMAL,  ET_EBOOK_EDIT_PRO_3200,                   "9A9999999999094097130000",         HINT_OVERLAY_END,     -12, "1570DB2EC29D0C154870FF59709893D0",    1,    STR_EBOOK_EDIT_PRO,                       "3.200",       STR_DELPI_JUNK,         "ASPack",   0  },
{  1,    1, 0, DT_NORMAL,  ET_EBOOK_EDIT_PRO_3210,                   "AE47E17A14AE094097130000",         HINT_OVERLAY_END,     -12, "",                                    0,    STR_EBOOK_EDIT_PRO,                       "3.210",       STR_DELPI_JUNK,         "",         0  },
{  1,    1, 0, DT_HASH,    ET_EBOOK_EDIT_PRO_3210_1,                 "",                                 HINT_NONE,              0, "B06C14790B3949C42ED17B89054D262B",    1,    STR_EBOOK_EDIT_PRO,                       "3.210",       STR_DELPI_JUNK,         "ASPack",   0  },
{  1,    1, 0, DT_HASH,    ET_EBOOK_EDIT_PRO_3210_NP_1,              "",                                 HINT_NONE,              0, "8F7906F8800D9103129F7307394A9D8C",    1,    STR_EBOOK_EDIT_PRO_NP,                    "3.210 <1>",   STR_DELPI_JUNK,         "",         0  },
{  1,    1, 0, DT_HASH,    ET_EBOOK_EDIT_PRO_3210_NP_2,              "",                                 HINT_NONE,              0, "EBC3B2891FBD0C68A29097643B942784",    1,    STR_EBOOK_EDIT_PRO_NP,                    "3.210 <2>",   STR_DELPI_JUNK,         "",         0  },
{  1,    1, 0, DT_HASH,    ET_EBOOK_EDIT_PRO_3210_NP_3,              "",                                 HINT_NONE,              0, "D99E8BD0CAED2CB52B8CC853763FAA1E",    1,    STR_EBOOK_EDIT_PRO_NP,                    "3.210 <3>",   STR_DELPI_JUNK,         "",         0  },
{  1,    1, 0, DT_NORMAL,  ET_EBOOK_EDIT_PRO_3300,                   "6666666666660A4097130000",         HINT_OVERLAY_END,     -12, "BE0770B6CDAF702451D5E5A1C32E94D3",    2,    STR_EBOOK_EDIT_PRO,                       "3.300",       STR_DELPI_JUNK,         "UPX",      0  },
{  1,    1, 0, DT_NORMAL,  ET_EBOOK_EDIT_PRO_3310,                   "7B14AE47E17A0A4097130000",         HINT_OVERLAY_END,     -12, "",                                    0,    STR_EBOOK_EDIT_PRO,                       "3.310",       STR_DELPI_JUNK,         "",         0  },
{  1,    1, 0, DT_HASH,    ET_EBOOK_EDIT_PRO_3310_1,                 "",                                 HINT_NONE,              0, "2430C22660F87563F85B9AB9CBF8E286",    2,    STR_EBOOK_EDIT_PRO_CRACKED,               "3.310",       STR_DELPI_JUNK,         "UPX",      0  },
{  1,    1, 0, DT_HASH,    ET_EBOOK_EDIT_PRO_3310_2,                 "",                                 HINT_NONE,              0, "FB7CD6D3C985370DAFF2F5D885316E57",    2,    STR_EBOOK_EDIT_PRO,                       "3.310",       STR_DELPI_JUNK,         "UPX",      0  },
{  1,    1, 0, DT_HASH,    ET_EBOOK_EDIT_PRO_3310_NP,                "",                                 HINT_NONE,              0, "CBEFA6DC4F0C71F12FAEAC5CD78D14F4",    1,    STR_EBOOK_EDIT_PRO_NP,                    "3.310",       STR_DELPI_JUNK,         "",         0  },
{  1,    1, 0, DT_NORMAL,  ET_EBOOK_EDIT_PRO_3340_1,                 "F1F44A5986B80A4097130000",         HINT_OVERLAY_END,     -12, "5E5E579E40E5F296C4B5336AD35302A6",    1,    STR_EBOOK_EDIT_PRO,                       "3.340 <1>",   STR_DELPI_JUNK,         "ASPack",   0  },
{  1,    1, 0, DT_NORMAL,  ET_EBOOK_EDIT_PRO_3340_2,                 "0B24287E8CB90A4097130000",         HINT_OVERLAY_END,     -12, "4DE9223F7231965CC7EA069C474BEF5D",    1,    STR_EBOOK_EDIT_PRO,                       "3.340 <2>",   STR_DELPI_JUNK,         "ASPack",   0  },
{  1,    1, 0, DT_NORMAL,  ET_EBOOK_EDIT_PRO_3341,                   "EE7C3F355EBA0A4097130000",         HINT_OVERLAY_END,     -12, "2D63C44C3AC423405CCB0E7BB774069A",    1,    STR_EBOOK_EDIT_PRO,                       "3.341",       STR_DELPI_JUNK,         "ASPack",   0  },
{  1,    1, 1, DT_NORMAL,  ET_EBOOK_EDIT_PRO_3X,                     "97130000",                         HINT_OVERLAY_END,      -4, "",                                    0,    STR_EBOOK_EDIT_PRO,                       "3.x",         STR_DELPI_JUNK,         "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_EBOOK_EDIT_PRO_OLD,                    "19330000",                         HINT_OVERLAY_END,      -4, "",                                    0,    STR_EBOOK_EDIT_PRO_OLD,                   "<Very Old>",  STR_DELPI_JUNK,         "",         0  },
{  0,    1, 1, DT_NORMAL,  ET_EBOOK_GOLD_30,                         "4542473330",                       HINT_OVERLAY_BEGIN,     0, "AF486490ED217ADDA7F71D511B0D8142",    2,    STR_EBOOK_GOLD,                           "3.x",         STR_DELPI_JUNK,         "",         0  }, // EBG30
{  0,    1, 1, DT_NORMAL,  ET_EBOOK_MAESTRO,                         "7E434F4E54524547494F4E7E",         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_EBOOK_MAESTRO,                        "",            STR_DELPI_JUNK,         "",         0  }, // ~CONTREGION~
{  0,    1, 0, DT_HASH,    ET_EBOOK_MAESTRO_FREE_1,                  "",                                 HINT_NONE,              0, "24F5B78A86734D3B4601F3FC3D9B0361",    2,    STR_EBOOK_MAESTRO,                        "Free <1>",    STR_DELPI_JUNK,         "",         0  }, // ~CONTREGION~
{  0,    1, 0, DT_HASH,    ET_EBOOK_MAESTRO_FREE_2,                  "",                                 HINT_NONE,              0, "6E130B96A678B211BC2EFEE99EB7FC3F",    2,    STR_EBOOK_MAESTRO,                        "Free <2>",    STR_DELPI_JUNK,         "",         0  }, // ~CONTREGION~
{  0,    1, 0, DT_HASH,    ET_EBOOK_MAESTRO_FREE_3,                  "",                                 HINT_NONE,              0, "814CB460F799F3624B20D5F065022302",    2,    STR_EBOOK_MAESTRO,                        "Free <3>",    STR_DELPI_JUNK,         "",         0  }, // ~CONTREGION~
{  0,    1, 0, DT_HASH,    ET_EBOOK_MAESTRO_PRO,                     "",                                 HINT_NONE,              0, "F09DBB3A5CD7A123DCEF5DE4454409B2",    2,    STR_EBOOK_MAESTRO,                        "Pro",         STR_DELPI_JUNK,         "",         0  }, // ~CONTREGION~
{  0,    1, 0, DT_HASH,    ET_EBOOK_MAESTRO_PRO_DEMO,                "",                                 HINT_NONE,              0, "C38F4A6054C78555EBBE8B77F46F3FD9",    2,    STR_EBOOK_MAESTRO,                        "Pro (Demo)",  STR_DELPI_JUNK,         "",         0  }, // ~CONTREGION~
{  0,    1, 1, DT_EXTRA,   ET_EBOOK_MAKER_21,                        "",                                 HINT_NONE,              0, "0EAB839426172828CE32620D1C475BD8",    2,    STR_EBOOK_MAKER,                          "2.1",         STR_DELPI_JUNK,         "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_EMZ,                                   "1F8B080000000000020B",             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_EMZ,                                  "",            "",                     "",         0  }, // EMZ
{  0,    1, 0, DT_NORMAL,  ET_EPSF_IMAGE,                            "252150532D41646F62652D322E3020",   HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_EPSF_IMAGE,                           "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_ERDAS_LAN_GIS_IMAGE,                   "48454144373400000300",             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ERDAS_LAN_GIS_IMAGE,                  "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 1, DT_EXTRA,   ET_E_DITOR_2,                             "",                                 HINT_NONE,              0, "",                                    0,    STR_E_DITOR_EBOOK_COMPILER,               "2.0",         STR_DELPI_JUNK,         "",         0  },
{  0,    1, 0, DT_HASH,    ET_E_DITOR_2_1,                           "",                                 HINT_NONE,              0, "7829A84F7DC5649643330B6B988DE79D",    1,    STR_E_DITOR_EBOOK_COMPILER,               "2.0 <1>",     STR_DELPI_JUNK,         "ASPack",   0  },
{  0,    1, 0, DT_HASH,    ET_E_DITOR_2_2,                           "",                                 HINT_NONE,              0, "7829A84F7DC5649643330B6B988DE79D",    1,    STR_E_DITOR_EBOOK_COMPILER,               "2.0 <2>",     STR_DELPI_JUNK,         "ASPack",   0  },
{  0,    1, 0, DT_NORMAL,  ET_FAKE_PAY_ARCHIVE_1,                    "3B21405041594152434849564540213B", HINT_OVERLAY_BEGIN,     0, "1B4F9B309C068819D8709CC345F2C382",    1,    STR_FAKE_PAY_ARCHIVE,                     "PayArchive",  STR_DELPI_JUNK,         "",         0  }, // ;!@PAYARCHIVE@!;
{  0,    1, 0, DT_HASH,    ET_FAKE_PAY_ARCHIVE_2,                    "",                                 HINT_NONE,              0, "0D0CC91E5B4EDC21D75F6F399A3959F8",    1,    STR_FAKE_PAY_ARCHIVE,                     "<1>",         STR_DELPI_JUNK,         "",         0  },
{  0,    1, 0, DT_HASH,    ET_FAKE_PAY_ARCHIVE_3,                    "",                                 HINT_NONE,              0, "11C4AD7CE56CCE439F69564F8AAB08A8",    2,    STR_FAKE_PAY_ARCHIVE,                     "<2>",         "Qt 4.7.0",             "UPX",      0  },
{  0,    1, 0, DT_HASH,    ET_FAKE_PAY_ARCHIVE_4,                    "",                                 HINT_NONE,              0, "289DBF66A141682A7398FD077935FA68",    2,    STR_FAKE_PAY_ARCHIVE,                     "<3>",         "",                     "UPX",      0  },
{  0,    1, 0, DT_HASH,    ET_FAKE_PAY_ARCHIVE_5,                    "",                                 HINT_NONE,              0, "7A87F8CC27CC13E7D5E9162809BE473B",    4,    STR_FAKE_PAY_ARCHIVE,                     "<4>",         "",                     "",         0  },
{  0,    1, 0, DT_HASH,    ET_FAKE_PAY_ARCHIVE_6,                    "",                                 HINT_NONE,              0, "BA3F7E1549C75F1E76F717789040F67B",    1,    STR_FAKE_PAY_ARCHIVE,                     "<5>",         "",                     "",         0  },
{  0,    1, 0, DT_HASH,    ET_FAKE_PAY_ARCHIVE_7,                    "",                                 HINT_NONE,              0, "F2404AD191E187D321225F9C061742FA",    2,    STR_FAKE_PAY_ARCHIVE,                     "<6>",         "",                     "UPX",      0  },
{  0,    1, 1, DT_EXTRA,   ET_FAST_EBOOK_COMPILER,                   "",                                 HINT_NONE,              0, "",                                    0,    STR_FAST_EBOOK_COMPILER,                  "",            STR_MSVC_STUB_MFC,      "",         0  }, // ....ÿÿ....CUpdat
{  0,    1, 0, DT_EXTRA,   ET_FAST_EBOOK_COMPILER_14,                "",                                 HINT_NONE,              0, "F5A63D369AD2D8D2A587858F8A994079",    1,    STR_FAST_EBOOK_COMPILER,                  "1.4x",        STR_MSVC_STUB_MFC,      "",         0  }, // ....ÿÿ....CUpdat
{  0,    1, 0, DT_EXTRA,   ET_FAST_EBOOK_COMPILER_1X,                "",                                 HINT_NONE,              0, "491D10C8B837B6F01754F35FF8D8C7AF",    1,    STR_FAST_EBOOK_COMPILER,                  "1.x",         STR_MSVC_STUB_MFC,      "",         0  }, // ....ÿÿ....CUpdat
{  0,    1, 0, DT_NORMAL,  ET_FIZ,                                   "46495A1A01",                       HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_FIZ,                                  "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_GEM_IMAGE,                             "0001000800040002",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_GEM_IMAGE,                            "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_GIF87,                                 "474946383761",                     HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_GIF87,                                "",            "",                     "",         0  }, // GIF87a
{  0,    1, 0, DT_NORMAL,  ET_GIF89,                                 "474946383961",                     HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_GIF89,                                "",            "",                     "",         0  }, // GIF89a
{  0,    1, 0, DT_NORMAL,  ET_GOES_IMAGE,                            "C8C4D940C1D9C5C1",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_GOES_IMAGE,                           "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_GZIP,                                  "1F8B0808",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_GZIP,                                 "",            "",                     "",         0  }, // Stolen from FA 2000  // 1F8B0808......35
{  0,    1, 1, DT_NORMAL,  ET_HELP_AND_MANUAL,                       "00484D46",                         HINT_OVERLAY_END,      -4, "A2B8CEFC5195E4DC95DBB495128D90CB",    2,    STR_HELP_AND_MANUAL,                      "",            STR_DELPI_JUNK,         "UPX",      0  }, // .HMF
{  0,    1, 0, DT_NORMAL,  ET_HIJAAK_IMAGE_DRAW_IMAGE,               "4753443102001100",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_HIJAAK_IMAGE_DRAW_IMAGE,              "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_HISTOGRAM_IMAGE,                       "6D6877616E68000401020102",         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_HISTOGRAM_IMAGE,                      "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_HITACHI_RASTER_IMAGE,                  "434144432F4B5220525354",           HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_HITACHI_RASTER_IMAGE,                 "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_HPACK,                                 "4850414B",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_HPACK,                                "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_HP_48_SX_IMAGE,                        "4850485034382D41",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_HP_48_SX_IMAGE,                       "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_HSC_SOUND,                             "C8E20420",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_HSC_SOUND,                            "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_HSI_JPEG_IMAGE,                        "687369310000",                     HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_HSI_JPEG_IMAGE,                       "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_HSI_RAW_IMAGE,                         "6D6877616E680004",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_HSI_RAW_IMAGE,                        "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 1, DT_NORMAL,  ET_HTML_EXECUTABLE,                       "47484533320609590737521C3D474447", HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_HTML_EXECUTABLE,                      "3.x",         STR_DELPI_JUNK,         "",         0  }, // GHE32..Y.7R.=GDG
{  0,    1, 0, DT_NORMAL,  ET_IBM_IOCA_IMAGE,                        "0011D3A6FB",                       HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_IBM_IOCA_IMAGE,                       "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_IDSKA32,                               "6964736B613332",                   HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_UNKNOWN_ARCHIVER,                     "<IDSKA32>",   "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_IMAGING_TECHNOLOGY_IMAGE,              "494D0000",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_IMAGING_TECHNOLOGY_IMAGE,             "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_IMG_SOFTWARE_SET_IMAGE,                "53434D49202020314154",             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_IMG_SOFTWARE_SET_IMAGE,               "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_INSET_SYSTEMS_IGF_IMAGE,               "0180040001005800",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_INSET_SYSTEMS_IGF_IMAGE,              "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_INSTALL_SHIELD,                        "2AAB79D8",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_INSTALL_SHIELD,                       "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_INTEL_DCX_IMAGE,                       "B168DE3A041000",                   HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_INTEL_DCX_IMAGE,                      "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_INTERPLAY_MVE,                         "496E746572706C6179204D56452046",   HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_INTERPLAY_MVE,                        "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_INTER_GRAPH_IMAGE,                     "0809FE0118000000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_INTER_GRAPH_IMAGE,                    "",            "",                     "",         0  }, // Stolen from FA 2000
{  1,    1, 1, DT_NORMAL,  ET_JANSOFT_SBOOKBUILDER,                  "0500000053626F6F6B",               HINT_OVERLAY_BEGIN,     0, "6E477E5766DD1DBB6A32BD97330A2A88",    2,    STR_JANSOFT_SBOOK_BUILDER,                "1.0",         STR_DELPI_JUNK,         "",         0  }, // Sbook
{  0,    1, 0, DT_NORMAL,  ET_JAR,                                   "1A4A61721B",                       HINT_OVERLAY_BEGIN,    14, "",                                    0,    STR_JAR,                                  "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_JFIF_1,                                "FFD8FFE0",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_JFIF,                                 "<1>",         "",                     "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_JFIF_2,                                "FFD8FFE000104A464946",             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_JFIF,                                 "<2>",         "",                     "",         0  }, // JFIF
{  0,    1, 1, DT_EXTRA,   ET_JIMMY_EBOOK_CREATOR,                   "",                                 HINT_NONE,              0, "1110655E02D123395B3B3B85F159FB43",    1,    STR_JIMMY_BROWN_EBOOK_CREATOR,            "",            STR_MSVC_STUB_MFC,      "",         0  }, // KG
{  0,    1, 0, DT_EXTRA,   ET_JIMMY_EBOOK_CREATOR_2000,              "",                                 HINT_NONE,              0, "",                                    0,    STR_JIMMY_BROWN_EBOOK_CREATOR,            "2000",        STR_MSVC_STUB_MFC,      "",         0  }, // KG
{  0,    1, 0, DT_HASH,    ET_JIMMY_EBOOK_CREATOR_2000_1,            "",                                 HINT_NONE,              0, "5F1918EEF124D83859F647F0B814431D",    1,    STR_JIMMY_BROWN_EBOOK_CREATOR,            "2000 <1>",    STR_MSVC_STUB_MFC,      "",         0  }, // KG
{  0,    1, 0, DT_HASH,    ET_JIMMY_EBOOK_CREATOR_2000_2,            "",                                 HINT_NONE,              0, "7623CB1AEA13725466CCF04B883A3B99",    1,    STR_JIMMY_BROWN_EBOOK_CREATOR,            "2000 <2>",    STR_MSVC_STUB_MFC,      "",         0  }, // KG
{  0,    1, 0, DT_HASH,    ET_JIMMY_EBOOK_CREATOR_2000_3,            "",                                 HINT_NONE,              0, "9A0DA664EA36B04D54DC477F5E40BC2A",    1,    STR_JIMMY_BROWN_EBOOK_CREATOR,            "2000 <3>",    STR_MSVC_STUB_MFC,      "",         0  }, // KG
{  0,    1, 0, DT_HASH,    ET_JIMMY_EBOOK_CREATOR_2000_4,            "",                                 HINT_NONE,              0, "89353EAF9E4F2AACC260D5C6316D9B26",    1,    STR_JIMMY_BROWN_EBOOK_CREATOR,            "2000 <4>",    STR_MSVC_STUB_MFC,      "",         0  }, // KG
{  0,    1, 0, DT_NORMAL,  ET_JRC,                                   "4A526368697665",                   HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_JRC,                                  "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 1, DT_HASH,    ET_KE_EBOOK_CREATOR,                      "",                                 HINT_NONE,              0, "1CF3F7CB92ACA2AEEDCFEAE553297445",    2,    STR_KE_EBOOK_CREATOR,                     "",            "",                     "UPX",      0  },
{  0,    1, 0, DT_NORMAL,  ET_KOFAX_GROUP_4_IMAGE,                   "2E4B466880000100",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_KOFAX_GROUP_4_IMAGE,                  "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_LEITHAUSER,                            "455345CE",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_LEITHAUSER,                           "",            STR_VB_JUNK,            "",         0  }, // ESEÎ
{  0,    1, 0, DT_NORMAL,  ET_LOTUS_IMAGE,                           "0100000001000800",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_LOTUS_IMAGE,                          "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MACROMEDIA_SHOCKWAVE_1_VIDEO,          "465753",                           HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MACROMEDIA_SHOCKWAVE_VIDEO,           "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MACROMEDIA_SHOCKWAVE_2_VIDEO,          "43575308",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MACROMEDIA_SHOCKWAVE_VIDEO,           "<1>",         "",                     "",         0  }, // CWS
{  0,    1, 0, DT_NORMAL,  ET_MAC_PAINT_IMAGE,                       "00000002FFFFFFFF",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MAC_PAINT_IMAGE,                      "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MAC_PICT_IMAGE,                        "50494354",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MAC_PICT_IMAGE,                       "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MAC_STUFFIT,                           "5349542100",                       HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MAC_STUFFIT,                          "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MICROGRAFIX_DRAW_IMAGE,                "01FF020403020002",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MICROGRAFIX_DRAW_IMAGE,               "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MPEG_1,                                "000001BA",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MPEG,                                 "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MPEG_2,                                "000001B3",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MPEG,                                 "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MPEG_VIDEO,                            "000001BA2FFFFDE6C1801861000001BB", HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MPEG_VIDEO,                           "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MS_ACCESS_DATABASE,                    "000100005374616E64617264204A6574", HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MS_ACCESS_DATABASE,                   "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MS_BITMAP_IMAGE,                       "01000900",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MS_BITMAP_IMAGE,                      "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MS_COMPRESS,                           "535A4444",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MS_COMPRESS,                          "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MS_ENHANCED_METAFILE_IMAGE,            "0100000058",                       HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MS_ENHANCED_METAFILE_IMAGE,           "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MS_METAFILE_IMAGE,                     "D7CDC69A",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MS_METAFILE_IMAGE,                    "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MS_PAINT_IMAGE,                        "4C696E53",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MS_PAINT_IMAGE,                       "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MS_WIN_CABINET,                        "4D534346",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MS_WIN_CABINET,                       "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_MZ_1,                                  "4D5A",                             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MZ,                                   "<1>",         "",                     "",         0  }, // MZ
{  0,    1, 0, DT_NORMAL,  ET_MZ_2,                                  "4D5A9000",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MZ,                                   "<2>",         "",                     "",         0  }, // MZ
{  0,    1, 0, DT_NORMAL,  ET_MZ_3,                                  "4D5A4B45524E454C33322E444C4C0000", HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_MZ,                                   "<3>",         "",                     "Upack",    0  }, // MZKERNEL32.DLL..
{  1,    1, 0, DT_HASH,    ET_NATATA_EBOOK_COMPILER_2004,            "",                                 HINT_NONE,              0, "BA71829793F0D85A1BBDB91CE1A10B3B",    2,    STR_NATATA_EBOOK_COMPILER,                "2004",        STR_DELPI_JUNK,         "",         0  }, // #1!#
{  1,    1, 0, DT_HASH,    ET_NATATA_EBOOK_COMPILER_2005,            "",                                 HINT_NONE,              0, "880F06FCB7FF2ED0502D7071905343FC",    2,    STR_NATATA_EBOOK_COMPILER,                "2005",        STR_DELPI_JUNK,         "",         0  }, // #1!#
{  1,    1, 0, DT_HASH,    ET_NATATA_EBOOK_COMPILER_2009,            "",                                 HINT_NONE,              0, "CA26361BA43D25ED14763750BE348DDD",    2,    STR_NATATA_EBOOK_COMPILER,                "2009",        STR_DELPI_JUNK,         "",         0  }, // #1!#
{  1,    1, 0, DT_HASH,    ET_NATATA_EBOOK_COMPILER_2010,            "",                                 HINT_NONE,              0, "BF86005073914C49F9BCE264437B6446",    2,    STR_NATATA_EBOOK_COMPILER,                "2010",        STR_DELPI_JUNK,         "",         0  }, // #1!#
{  1,    1, 0, DT_HASH,    ET_NATATA_EBOOK_COMPILER_21,              "",                                 HINT_NONE,              0, "2C6A79378A37A051B9C879B56CFC3741",    2,    STR_NATATA_EBOOK_COMPILER,                "2.1 <1>",     STR_DELPI_JUNK,         "",         0  }, // #1!#
{  1,    1, 0, DT_HASH,    ET_NATATA_EBOOK_COMPILER_21_2,            "",                                 HINT_NONE,              0, "2146BBEA421FD6801FFFA503C8ABCFC3",    2,    STR_NATATA_EBOOK_COMPILER,                "2.1 <2>",     STR_DELPI_JUNK,         "",         0  }, // #1!#
{  1,    1, 0, DT_HASH,    ET_NATATA_EBOOK_COMPILER_21_DEMO,         "",                                 HINT_NONE,              0, "D8BDF24E606B380512E3201C73F19988",    2,    STR_NATATA_EBOOK_COMPILER,                "2.1 (Demo)",  STR_DELPI_JUNK,         "",         0  }, // #1!#
{  1,    1, 0, DT_HASH,    ET_NATATA_EBOOK_COMPILER_30,              "",                                 HINT_NONE,              0, "37F4895BF3494D743F5D30183D6BEF63",    1,    STR_NATATA_EBOOK_COMPILER,                "3.0",         STR_DELPI_JUNK,         "",         0  }, // #1!#
{  1,    1, 1, DT_NORMAL,  ET_NATATA_V_BOOK_EBOOK_COMPILER,          "23312123",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_NATATA_V_BOOK_EBOOK_COMPILER,         "",            STR_DELPI_JUNK,         "",         0  }, // #1!#
{  0,    1, 0, DT_NORMAL,  ET_NEXT_SUN_SOUND,                        "2E736E64",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_NEXT_SUN_SOUND,                       "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 1, DT_NORMAL,  ET_OAKLEY_WEB_COMPILER,                   "535049531A4C4835",                 HINT_OVERLAY_BEGIN,     0, "176FEF514D04F4EE3D07F070C431483B",    1,    STR_OAKLEY_WEB_COMPILER,                  "1.13",        STR_DELPI_JUNK,         "",         0  }, // (c) Oakley Data Services, 1998
{  0,    1, 0, DT_NORMAL,  ET_OAZ_FAX_IMAGE,                         "0F0F0F0F01000000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_OAZ_FAX_IMAGE,                        "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_OS2_ICON_IMAGE,                        "43494E000000",                     HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_OS2_ICON_IMAGE,                       "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_PCI_ENC_CRYPTOR,                       "06504349454E",                     HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PCI_ENC_CRYPTOR,                      "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_PDF,                                   "25504446",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PDF,                                  "",            "",                     "",         0  }, // %PDF
{  0,    1, 0, DT_NORMAL,  ET_PDF_11,                                "255044462D312E31",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PDF_11,                               "",            "",                     "",         0  }, // %PDF-1.1
{  0,    1, 0, DT_NORMAL,  ET_PDF_12,                                "255044462D312E32",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PDF_12,                               "",            "",                     "",         0  }, // %PDF-1.2
{  0,    1, 0, DT_NORMAL,  ET_PDF_13,                                "255044462D312E33",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PDF_13,                               "",            "",                     "",         0  }, // %PDF-1.3
{  0,    1, 0, DT_NORMAL,  ET_PDF_14,                                "255044462D312E34",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PDF_14,                               "",            "",                     "",         0  }, // %PDF-1.4
{  0,    1, 0, DT_NORMAL,  ET_PDF_15,                                "255044462D312E35",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PDF_15,                               "",            "",                     "",         0  }, // %PDF-1.5
{  0,    1, 0, DT_NORMAL,  ET_PDF_16,                                "255044462D312E36",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PDF_16,                               "",            "",                     "",         0  }, // %PDF-1.6
{  0,    1, 0, DT_NORMAL,  ET_PDF_17,                                "255044462D312E37",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PDF_17,                               "",            "",                     "",         0  }, // %PDF-1.7
{  0,    1, 0, DT_NORMAL,  ET_PDS_IMAGE,                             "494D4147454944454E544946494552",   HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PDS_IMAGE,                            "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_PNG,                                   "89504E470D0A1A0A0000000D49484452", HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PNG,                                  "",            "",                     "",         0  }, // PNG
{  0,    1, 0, DT_NORMAL,  ET_PS,                                    "252150532D41646F62652D",           HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PS,                                   "",            "",                     "",         0  }, // %!PS-Adobe-
{  0,    1, 0, DT_NORMAL,  ET_PS_20,                                 "252150532D41646F62652D322E30",     HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PS,                                   "2.0",         "",                     "",         0  }, // %!PS-Adobe-2.0
{  0,    1, 0, DT_NORMAL,  ET_PS_30,                                 "252150532D41646F62652D332E30",     HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_PS,                                   "3.0",         "",                     "",         0  }, // %!PS-Adobe-3.0
{  0,    1, 1, DT_EXTRA,   ET_PUBLICANT_EXE_BOOK,                    "",                                 HINT_NONE,              0, "",                                    0,    STR_PUBLICANT_EXE_BOOK,                   "",            "",                     "",         0  }, // zreader
{  0,    1, 0, DT_HASH,    ET_PUBLICANT_EXE_BOOK_01,                 "",                                 HINT_NONE,              0, "2D1369D5C73D5725A517D8186BA10A9B",    1,    STR_PUBLICANT_EXE_BOOK,                   "<01>",        "",                     "",         0  }, // zreader
{  0,    1, 0, DT_HASH,    ET_PUBLICANT_EXE_BOOK_02,                 "",                                 HINT_NONE,              0, "35654CA47B9DA8ECB531BE2BE7FF0364",    1,    STR_PUBLICANT_EXE_BOOK,                   "<02>",        "",                     "",         0  }, // zreader
{  0,    1, 0, DT_HASH,    ET_PUBLICANT_EXE_BOOK_03,                 "",                                 HINT_NONE,              0, "4D742CBED91CFE87F80F6A17B01732DB",    1,    STR_PUBLICANT_EXE_BOOK,                   "<03>",        "",                     "",         0  }, // zreader
{  0,    1, 0, DT_HASH,    ET_PUBLICANT_EXE_BOOK_04,                 "",                                 HINT_NONE,              0, "5398D92E0258FE8FCF0727501FF1B208",    1,    STR_PUBLICANT_EXE_BOOK,                   "<04>",        "",                     "",         0  }, // zreader
{  0,    1, 0, DT_HASH,    ET_PUBLICANT_EXE_BOOK_05,                 "",                                 HINT_NONE,              0, "8154C100424946E83665ED06EA1DFD15",    1,    STR_PUBLICANT_EXE_BOOK,                   "<05>",        "",                     "",         0  }, // zreader
{  0,    1, 0, DT_HASH,    ET_PUBLICANT_EXE_BOOK_06,                 "",                                 HINT_NONE,              0, "8A6766BDC0547005F874FABE4F37C57E",    1,    STR_PUBLICANT_EXE_BOOK,                   "<06>",        "",                     "",         0  }, // zreader
{  0,    1, 0, DT_HASH,    ET_PUBLICANT_EXE_BOOK_07,                 "",                                 HINT_NONE,              0, "B45507A67F862AC473EFC526AF29A774",    1,    STR_PUBLICANT_EXE_BOOK,                   "<07>",        "",                     "",         0  }, // zreader
{  0,    1, 0, DT_HASH,    ET_PUBLICANT_EXE_BOOK_08,                 "",                                 HINT_NONE,              0, "C1DDB92608EB097F88593257002164BD",    1,    STR_PUBLICANT_EXE_BOOK,                   "<08>",        "",                     "",         0  }, // zreader
{  0,    1, 0, DT_HASH,    ET_PUBLICANT_EXE_BOOK_09,                 "",                                 HINT_NONE,              0, "C7A08A0ADE7212FCCD6CDE16CB9273ED",    1,    STR_PUBLICANT_EXE_BOOK,                   "<09>",        "",                     "",         0  }, // zreader
{  0,    1, 0, DT_HASH,    ET_PUBLICANT_EXE_BOOK_10,                 "",                                 HINT_NONE,              0, "EAD7F89779E7AA3DA1F7544746B9BC89",    1,    STR_PUBLICANT_EXE_BOOK,                   "<10>",        "",                     "",         0  }, // zreader
{  0,    1, 0, DT_EXTRA,   ET_PUBLICANT_EXE_BOOK_2D,                 "",                                 HINT_NONE,              0, "",                                    0,    STR_PUBLICANT_EXE_BOOK,                   "<$2D>",       "",                     "",         0  }, // zreader
{  0,    1, 0, DT_EXTRA,   ET_PUBLICANT_EXE_BOOK_2E,                 "",                                 HINT_NONE,              0, "",                                    0,    STR_PUBLICANT_EXE_BOOK,                   "<$2E>",       "",                     "",         0  }, // zreader
{  0,    1, 0, DT_EXTRA,   ET_PUBLICANT_EXE_BOOK_31,                 "",                                 HINT_NONE,              0, "",                                    0,    STR_PUBLICANT_EXE_BOOK,                   "<$31>",       "",                     "",         0  }, // zreader
{  0,    1, 0, DT_EXTRA,   ET_PUBLICANT_EXE_BOOK_38,                 "",                                 HINT_NONE,              0, "",                                    0,    STR_PUBLICANT_EXE_BOOK,                   "<$38>",       "",                     "",         0  }, // zreader
{  0,    1, 0, DT_EXTRA,   ET_PUBLICANT_EXE_BOOK_39,                 "",                                 HINT_NONE,              0, "",                                    0,    STR_PUBLICANT_EXE_BOOK,                   "<$39>",       "",                     "",         0  }, // zreader
{  0,    1, 0, DT_EXTRA,   ET_PUBLICANT_EXE_BOOK_3A,                 "",                                 HINT_NONE,              0, "",                                    0,    STR_PUBLICANT_EXE_BOOK,                   "<$3A>",       "",                     "",         0  }, // zreader
{  0,    1, 0, DT_EXTRA,   ET_PUBLICANT_EXE_BOOK_3C,                 "",                                 HINT_NONE,              0, "",                                    0,    STR_PUBLICANT_EXE_BOOK,                   "<$3C>",       "",                     "",         0  }, // zreader
{  0,    1, 0, DT_NORMAL,  ET_QUICKLINK_II_FAX_IMAGE,                "514C494946415820",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_QUICKLINK_II_FAX_IMAGE,               "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_RAR,                                   "52617221",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_RAR,                                  "",            "",                     "",         0  }, // Rar!
{  0,    1, 0, DT_NORMAL,  ET_RAR_40,                                "526172211A0700",                   HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_RAR,                                  "4.0",         "",                     "",         0  }, // Rar!
{  0,    1, 0, DT_HASH,    ET_RAR_SFX,                               "",                                 HINT_NONE,              0, "061DD9B707558CA592C0EACED74FFB78",    2,    STR_RAR_SFX,                              "",            "",                     "",         0  },
{  0,    1, 0, DT_HASH,    ET_RAR_WIN32_SFX_320,                     "",                                 HINT_NONE,              0, "CFF268651764062455AEEA090703F318",    2,    STR_RAR_WIN32_SFX_320,                    "3.20",        "",                     "",         0  },
{  0,    1, 0, DT_HASH,    ET_RAR_ZIP_SFX_1,                         "",                                 HINT_NONE,              0, "1211BCC4411D11DC6ECA5901C4459D11",    1,    STR_RAR_ZIP_SFX,                          "<1>",         "",                     "",         0  },
{  0,    1, 0, DT_HASH,    ET_RAR_ZIP_SFX_2,                         "",                                 HINT_NONE,              0, "50C2094638E9CAADF2ACC8A81418FC8B",    2,    STR_RAR_ZIP_SFX,                          "<2>",         "",                     "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_RAR_ZIP_SFX_3,                         "504B03040A0009000000",             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_RAR_ZIP_SFX,                          "<3>",         "",                     "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_RAR_ZIP_SFX_4,                         "504B03040A0000000000",             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_RAR_ZIP_SFX,                          "<4>",         "",                     "",         0  },
{  0,    1, 0, DT_HASH,    ET_RAR_ZIP_SFX_5,                         "",                                 HINT_NONE,              0, "489777BC8B382311C3741E5A824325D9",    2,    STR_RAR_ZIP_SFX,                          "<5>",         "",                     "UPX",      0  },
{  0,    1, 0, DT_NORMAL,  ET_REAL_NETWORKS_SOUND_VIDEO,             "2E524D46",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_REAL_NETWORKS_SOUND_VIDEO,            "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_RIX_IMAGE,                             "52495833",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_RIX_IMAGE,                            "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_SA2_ADLIB_SOUND,                       "53416454",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_SA2_ADLIB_SOUND,                      "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_SCI_FAX_IMAGE,                         "44543D00",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_SCI_FAX_IMAGE,                        "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_SGI_IMAGE,                             "01DA00010003",                     HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_SGI_IMAGE,                            "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_SIERRA_SOUND,                          "8D0C534F4C0022560D",               HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_SIERRA_SOUND,                         "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_SMART_INSTALL_MAKER,                   "536D61727420496E7374616C6C204D61", HINT_OVERLAY_BEGIN,     0, "F4ACDC9D84A2590B19509E660E05D16A",    1,    STR_SMART_INSTALL_MAKER,                  "",            STR_DELPI_JUNK,         "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_SMK_VIDEO,                             "534D4B32",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_SMK_VIDEO,                            "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_SQZ,                                   "484C53515A",                       HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_SQZ,                                  "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_STIRLING,                              "135D658C3A",                       HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_STIRLING,                             "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_SUN_ICON_IMAGE,                        "2F2A20466F726D61745F7665727369",   HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_SUN_ICON_IMAGE,                       "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_SUN_RASTER_IMAGE,                      "59A66A95",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_SUN_RASTER_IMAGE,                     "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_SWAG,                                  "2D7377312D",                       HINT_OVERLAY_BEGIN,     2, "",                                    0,    STR_SWAG,                                 "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_TEACH_SHOP,                            "0098010056",                       HINT_OVERLAY_BEGIN,     0, "A5BBBDA512505C3AB0B7633E6AC24DFB",    1,    STR_TEACH_SHOP,                           "",            "",                     "",         0  },
{  0,    1, 1, DT_HASH,    ET_TETERIN,                               "",                                 HINT_NONE,              0, "3C2E1846C1AB3ADB8EAAFF03EBC18878",    1,    STR_TETERIN,                              "",            STR_DELPI_JUNK,         "",         0  }, // found in the "Twins Oracle" e-book
{  0,    1, 0, DT_NORMAL,  ET_TPACK_17,                              "54504143",                         HINT_OVERLAY_BEGIN,     1, "",                                    0,    STR_TPACK_17,                             "1.7",         "",                     "",         0  }, // Stolen from FA 2000  // ..54504143..312E37
{  0,    1, 0, DT_NORMAL,  ET_TRILOBYTE_ICON_IMAGE,                  "69636F6E0100100010",               HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_TRILOBYTE_ICON_IMAGE,                 "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_TRILOBYTE_JPEG_IMAGE,                  "8410FFFFFFFF1E0001100800000000",   HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_TRILOBYTE_JPEG_IMAGE,                 "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_TRUEVISION_TARGA_IMAGE,                "0000020000000000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_TRUEVISION_TARGA_IMAGE,               "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_UFA,                                   "554641C6D2C10000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_UFA,                                  "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_ULTRA_COMPRESSOR,                      "5543321A",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ULTRA_COMPRESSOR,                     "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_US_PATENT_IMAGE,                       "55532050544F2000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_US_PATENT_IMAGE,                      "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_VIRTUAL_IMAGE_MAKER_IMAGE,             "534F4D56",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_VIRTUAL_IMAGE_MAKER_IMAGE,            "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_VI_TEC_IMAGE,                          "005B07200000002C",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_VI_TEC_IMAGE,                         "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_WAW,                                   "61574157",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_UNKNOWN_ARCHIVER,                     "<WAW>",       "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 1, DT_NORMAL,  ET_WEB_EXE,                               "2A4D32452A",                       HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_WEB_EXE,                              "1.x",         STR_DELPI_JUNK,         "",         0  },
{  0,    1, 0, DT_HASH,    ET_WEB_EXE_01,                            "",                                 HINT_NONE,              0, "264333D949E2D0997EE9D5483EF99438",    2,    STR_WEB_EXE,                              "1.x <01>",    STR_DELPI_JUNK,         "UPX",      0  },
{  0,    1, 0, DT_HASH,    ET_WEB_EXE_03,                            "",                                 HINT_NONE,              0, "624AA253A845960CDBAE2385DB8DFAAB",    1,    STR_WEB_EXE_CRACKED,                      "1.x",         STR_DELPI_JUNK,         "ASPack",   0  },
{  0,    1, 0, DT_HASH,    ET_WEB_EXE_04,                            "",                                 HINT_NONE,              0, "673D6E93ADA9016A7908DDC2912AD808",    2,    STR_WEB_EXE,                              "1.x <04>",    STR_DELPI_JUNK,         "UPX",      0  },
{  0,    1, 0, DT_HASH,    ET_WEB_EXE_2000_1,                        "",                                 HINT_NONE,              0, "4FB6007F0ECD7CDDECE4E84AB8B13329",    2,    STR_WEB_EXE,                              "2000 <1>",    STR_DELPI_JUNK,         "UPX",      0  },
{  0,    1, 0, DT_HASH,    ET_WEB_EXE_2000_2,                        "",                                 HINT_NONE,              0, "E86C8C20D6020AEC6375F22B7833DDC1",    2,    STR_WEB_EXE,                              "2000 <2>",    STR_DELPI_JUNK,         "UPX",      0  },
{  0,    1, 0, DT_HASH,    ET_WEB_EXE_ACME,                          "",                                 HINT_NONE,              0, "BDDE189311CD1D3EB8285C7315587C2F",    1,    STR_WEB_EXE_ACME,                         "1.x",         STR_DELPI_JUNK,         "ASPack",   0  },
{  0,    1, 0, DT_HASH,    ET_WEB_EXE_AZRAEL,                        "",                                 HINT_NONE,              0, "B8078054517BBF3484E3913097B0B369",    1,    STR_WEB_EXE_AZRAEL,                       "1.x",         STR_DELPI_JUNK,         "ASPack",   0  },
{  0,    1, 0, DT_HASH,    ET_WEB_EXE_COA,                           "",                                 HINT_NONE,              0, "88A30962757B25A6B0EE2B326505A172",    1,    STR_WEB_EXE_COA,                          "1.x",         STR_DELPI_JUNK,         "ASPack",   0  },
{  0,    1, 0, DT_HASH,    ET_WEB_EXE_CRACKED,                       "",                                 HINT_NONE,              0, "C0EFBA4991F4325796A4B601E7D49F18",    1,    STR_WEB_EXE_CRACKED,                      "1.x",         STR_DELPI_JUNK,         "ASPack",   0  },
{  0,    1, 0, DT_HASH,    ET_WEB_EXE_CRACKED_2,                     "",                                 HINT_NONE,              0, "F215B3B47274BF925D9735727ECA74F8",    1,    STR_WEB_EXE_CRACKED,                      "1.x <10>",    STR_DELPI_JUNK,         "ASPack",   0  },
{  0,    1, 0, DT_HASH,    ET_WEB_EXE_CRACKED_3,                     "",                                 HINT_NONE,              0, "FDC35BE6859794AE41002CF78F0E9043",    1,    STR_WEB_EXE_CRACKED,                      "1.x <11>",    STR_DELPI_JUNK,         "ASPack",   0  },
{  0,    1, 0, DT_NORMAL,  ET_WICAT_GED_IMAGE,                       "0D004000",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_WICAT_GED_IMAGE,                      "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_WORDPERFECT,                           "FF575043",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_WORDPERFECT,                          "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_WORDPERFECT_IMAGE,                     "FF57504310000000",                 HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_WORDPERFECT_IMAGE,                    "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_XM_SOUND,                              "457874656E646564204D6F64756C653A", HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_XM_SOUND,                             "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_XWD_IMAGE,                             "000000710000000700000002000000",   HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_XWD_IMAGE,                            "",            "",                     "",         0  }, // Stolen from FA 2000
{  0,    1, 0, DT_NORMAL,  ET_ZIP_BCB_DELPHI,                        "504B0304140002000800",             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ZIP_MARKER_DELPHI,                    "",            "",                     "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_ZIP_LOCAL_HEADER,                      "504B0304",                         HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ZIP_LOCAL_HEADER,                     "",            "",                     "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_ZIP_SFX_1,                             "504B0304140000000800",             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ZIP_SFX_1,                            "",            "",                     "",         0  },
{  0,    1, 0, DT_HASH,    ET_ZIP_SFX_2,                             "",                                 HINT_NONE,              0, "BDCD1B2AA56739FEE1D0F7887586F302",    2,    STR_ZIP_SFX_2,                            "",            STR_DELPI_JUNK,         "UPX",      0  },
{  0,    1, 0, DT_NORMAL,  ET_ZIP_SFX_DISTUTILS,                     "6D657461646174615D0A617574686F72", HINT_OVERLAY_BEGIN,     1, "",                                    0,    STR_ZIP_SFX_DISTUTILS,                    "",            STR_MSVC_STUB,          "",         0  }, // [metadata].author
{  0,    1, 0, DT_HASH,    ET_ZIP_SFX_DISTUTILS_264,                 "",                                 HINT_NONE,              0, "8BD0E5D9B85DBCED9F72D9853E5F28FF",    1,    STR_ZIP_PYTHON_DISTUTILS,                 "2.6.4",       STR_PYTHON_JUNK,        "",         0  },
{  0,    1, 0, DT_NORMAL,  ET_ZM,                                    "5A4D",                             HINT_OVERLAY_BEGIN,     0, "",                                    0,    STR_ZM,                                   "",            "",                     "",         0  }  // ZM
// UNP DET  PR DET_TYPE    ID                                        SIGNATURE                           SIGN. HINT            OFS  HASH                                   SECT  NAME                                      VER            STUB                    PACKER
};             
// Should be in ascending sort order for binary search !
// Will be CRASH if not !
// !! Resort if smth modified !!

///////////////////////////////////////////////////////////////////////////////
// ---] Should be sorted !!
///////////////////////////////////////////////////////////////////////////////

/* ******************************************************************** **
** @@                   internal prototypes
** ******************************************************************** */

/* ******************************************************************** **
** @@                   external global variables
** ******************************************************************** */

extern DWORD   dwKeepError = 0;

/* ******************************************************************** **
** @@                   static global variables
** ******************************************************************** */

static MMF              _MF;
static PELib            _TheFile;
static AC_Search*       _pACS = NULL;

static int              _iSectNum = 0;
static char             _pszSignature[MD5_SIGNATURE_SIZE];

static int              _iCnt = 0;

static bool             _bInfoOnly         = false;
static bool             _bJustOverlay      = false;
static bool             _bKeepOverlay      = false;
static bool             _bIgnoreValidation = false;

static FILE*            _pOut = NULL;

static E_Unpacker*      _pUnpacker = NULL;

static DWORD            _dwComment  = 0;
static DWORD            _dwStubSize = 0;
static DWORD            _dwOverSize = 0;

/* ******************************************************************** **
** @@                   real code
** ******************************************************************** */

CWinApp theApp;

/* ******************************************************************** **
** @@ E_Compare()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static int E_Compare(const void* p1,const void* p2)
{
   SSF_TEXT_RECORD*  pKey1 = (SSF_TEXT_RECORD*)p1;
   SSF_TEXT_RECORD*  pKey2 = (SSF_TEXT_RECORD*)p2;

   return pKey1->_BookType - pKey2->_BookType;
}

/* ******************************************************************** **
** @@ PrintHeader()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void PrintHeader(const char* const pszFileName)
{
   int   iCnt = _TheFile.GetSectCnt();

   fprintf(_pOut,"\n + [%s] File Header Info:\n",pszFileName);
   fprintf(_pOut,"\n##      Name       VirtSize   RVA        PhysSize   Offset     Attr");
   fprintf(_pOut,"\n-----   --------   --------   --------   --------   --------   --------");

   for (int ii = 0; ii < iCnt; ++ii)
   {
      PIMAGE_SECTION_HEADER   pSect = _TheFile.GetSection((WORD)ii);

      fprintf
      (
         _pOut,"\n%5d   %-8s   %08X   %08X   %08X   %08X   %08X",
         ii + 1,
         _TheFile.GetSectionName((WORD)ii),
         pSect->Misc.VirtualSize,
         pSect->VirtualAddress,
         pSect->SizeOfRawData,
         pSect->PointerToRawData,
         pSect->Characteristics
      );
   }

   int      iEP      = _TheFile.GetEntryPointSectNum() + 1;
   int      iSectNum = 0;

   DWORD    dwEP = _TheFile.GetEntryPointOffset(iSectNum);

   fprintf(_pOut,"\n\nEntryPoint [Sect: %d] %08X / .%08X\n",iEP,dwEP,_TheFile.GetEntryPointVA());

   if (!dwEP)
   {
      fprintf(_pOut,"\n!! Strange: EP out of file. Packed/Encrypted/Infected?\n");
   }
   else
   {
      if (iEP != 1)
      {
         fprintf(_pOut,"\n!! Strange: EP is NOT in the First Section. Packed/Encrypted/Infected?\n");
      }

      if (iEP == 2 && !stricmp(_TheFile.GetSectionName(0),"CODE") && !stricmp(_TheFile.GetSectionName(1),"DATA"))
      {
         fprintf(_pOut,"\nOh, yeah. It is infected by Delphi!\n");
      }
   }

   fprintf(_pOut,"\n");

   Ruller(_pOut);
   HexDumpQLine(_MF.Buffer(),_MF.Size(),dwEP,_pOut);
   Ruller(_pOut);

   fprintf(_pOut,"\n");
}

/* ******************************************************************** **
** @@ IsBookMaker_21()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static bool IsBookMaker_21()
{
   BYTE*    pBuf   = _MF.Buffer() + _dwStubSize;

   if (_dwOverSize < 0x100)
   {
      return false;
   }

   int      iSize = *pBuf;

   if (!iSize)
   {
      return false;
   }

   if ((pBuf[2] != ':') || (pBuf[3] != '\\') || (pBuf[iSize] != '\\'))
   {
      return false;
   }

   for (int ii = 0; ii < (0x100 - iSize - 1); ++ii)
   {
      if (pBuf[iSize + 1 + ii])
      {
         return false;
      }
   }

   return true;
}

/* ******************************************************************** **
** @@ IsActivEbookCompiler()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static bool IsActivEbookCompiler()
{
   BYTE*    pBuf = _MF.Buffer() + _dwStubSize;

   DWORD    dwTail = *(DWORD*)(pBuf + sizeof(DWORD));

   if ((dwTail & 0xFFFFFF00) == 0xAAAAAA00)
   {
      return true;
   }

   return false;
}

/* ******************************************************************** **
** @@ IsActivEbookCompilerRebrander()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static bool IsActivEbookCompilerRebrander()
{
   BYTE*    pBuf = _MF.Buffer() + _dwStubSize;

   DWORD    dwTail = *(DWORD*)(pBuf + sizeof(DWORD));

   if ((dwTail & 0xFFFFFF00) == 0xAAAAAA00)
   {
      return true;
   }

   return false;
}

/* ******************************************************************** **
** @@ IsEbooksWriter()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static bool IsEbooksWriter()
{
   BYTE*    pBuf = _MF.Buffer() + _dwStubSize;

   // BooksReader
   const char*    pszPattern = "eBooksReader_";

   int      iPatSize = 13;

   int   iSeekSize = min(OVERLAY_SIZE_LOOKUP,_dwOverSize);

   int   iFoundAt  = BMH_Search(pBuf,iSeekSize,(BYTE*)pszPattern,iPatSize);

   return (iFoundAt == 0x26);
}

/* ******************************************************************** **
** @@ FastEbookCompilerCheck()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static E_BOOK_TYPE FastEbookCompilerCheck()
{
   DWORD    dwTail = *(DWORD*)(_MF.Buffer() + _MF.Size() - sizeof(DWORD));

   if (dwTail == 0x484B0009)
   {
      return ET_FAST_EBOOK_COMPILER_1X;
   }
   else if (dwTail == 0x484B000B)
   {
      return ET_FAST_EBOOK_COMPILER_14;
   }
   else if ((dwTail & 0xFFFFFF00) == 0x484B0000)
   {
      return ET_FAST_EBOOK_COMPILER;
   }

   return ET_UNKNOWN_EBOOK;
}

/* ******************************************************************** **
** @@ EbookCreatorCheck()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static E_BOOK_TYPE EbookCreatorCheck()
{
   DWORD    dwTail = *(DWORD*)(_MF.Buffer() + _MF.Size() - sizeof(DWORD));

   if (dwTail == 0x474B0004)
   {
      return ET_JIMMY_EBOOK_CREATOR;
   }
   else if (dwTail == 0x474B0005)
   {
      return ET_JIMMY_EBOOK_CREATOR_2000;
   }
   else if ((dwTail & 0xFFFFFF00) == 0x474B0000)
   {
      return ET_FAST_EBOOK_COMPILER;
   }

   return ET_UNKNOWN_EBOOK;
}

/* ******************************************************************** **
** @@ GetRecord()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static const SSF_TEXT_RECORD* const GetRecord(int iIdx)
{
   return &pTextDB[iIdx];
}

/* ******************************************************************** **
** @@ FindRecord()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static const SSF_TEXT_RECORD* const FindRecord(E_BOOK_TYPE BookType)
{
// Optimized BinSearch version
// bsearch returns a pointer to an occurrence of key in the array pointed to by base.
// If key is not found, the function returns NULL.
// If the array is not in ascending sort order or contains duplicate records
// with identical keys, the result is unpredictable.

   SSF_TEXT_RECORD      Test;

   memset(&Test,0,sizeof(Test));

   Test._BookType = BookType;

   return (SSF_TEXT_RECORD*)bsearch(&Test,pTextDB,E_TEXT_DB_SIZE,sizeof(SSF_TEXT_RECORD),E_Compare);
}

/* ******************************************************************** **
** @@ EnsureBookType()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static E_BOOK_TYPE EnsureBookType(const SSF_TEXT_RECORD* const pFound)
{
   if (pFound && (pFound->_iSignHintOfs == pFound->_iFoundAt))
   {
      switch (pFound->_BookType)
      {
         case ET_ACTIV_EBOOK_COMPILER:
         {
            if (*_pszSignature && (_iSectNum == 1))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_ACTIV_EBOOK_COMPILER_302)->_pszHash))
               {
                  return ET_ACTIV_EBOOK_COMPILER_302;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_ACTIV_EBOOK_COMPILER_401)->_pszHash))
               {
                  return ET_ACTIV_EBOOK_COMPILER_401;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_ACTIV_EBOOK_COMPILER_422)->_pszHash))
               {
                  return ET_ACTIV_EBOOK_COMPILER_422;
               }
            }

            if (IsActivEbookCompiler())
            {
               return pFound->_BookType;
            }

            return ET_UNKNOWN_EBOOK;
         }
         case ET_ACTIV_EBOOK_COMPILER_REBRANDER:
         {
            if (IsActivEbookCompilerRebrander())
            {
               return pFound->_BookType;
            }

            return ET_UNKNOWN_EBOOK;
         }
         case ET_ZIP_SFX_1:
         {
            if (*_pszSignature && (_iSectNum == 1))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_ANTECHINUS_EBOOK_WIZARD_32)->_pszHash))
               {
                  return ET_ANTECHINUS_EBOOK_WIZARD_32;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_ANTECHINUS_EBOOK_WIZARD_32)->_pszHash))
               {
                  return ET_ANTECHINUS_EBOOK_WIZARD_32;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_RAR_ZIP_SFX_1)->_pszHash))
               {
                  return ET_RAR_ZIP_SFX_1;
               }

               return pFound->_BookType;
            }
         }
         case ET_COINSOFT_EBOOK_MAKER:
         {
            if (*_pszSignature && (_iSectNum == 1))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_COINSOFT_EBOOK_MAKER_1)->_pszHash))
               {
                  return ET_COINSOFT_EBOOK_MAKER_1;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_COINSOFT_EBOOK_MAKER_2)->_pszHash))
               {
                  return ET_COINSOFT_EBOOK_MAKER_2;
               }

               return pFound->_BookType;
            }
         }
         case ET_COINSOFT_COPYRIGHT:
         {
            if (*_pszSignature && (_iSectNum == 1))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_COINSOFT_EBOOK_PROTECT_MASTER)->_pszHash))
               {
                  return ET_COINSOFT_EBOOK_PROTECT_MASTER;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_BOOK_BIZ_MASTER_20)->_pszHash))
               {
                  return ET_BOOK_BIZ_MASTER_20;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_BOOK_BIZ_MASTER_20_ENCRYPTED)->_pszHash))
               {
                  return ET_BOOK_BIZ_MASTER_20_ENCRYPTED;
               }

               return pFound->_BookType;
            }
         }
         case ET_EBOOK_EDIT_PRO_3210:
         {
            if (*_pszSignature && (_iSectNum == 1))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_EDIT_PRO_3210_1)->_pszHash))
               {
                  return ET_EBOOK_EDIT_PRO_3210_1;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_EDIT_PRO_3210_NP_1)->_pszHash))
               {
                  return ET_EBOOK_EDIT_PRO_3210_NP_1;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_EDIT_PRO_3210_NP_2)->_pszHash))
               {
                  return ET_EBOOK_EDIT_PRO_3210_NP_2;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_EDIT_PRO_3210_NP_3)->_pszHash))
               {
                  return ET_EBOOK_EDIT_PRO_3210_NP_3;
               }

               return pFound->_BookType;
            }
         }
         case ET_EBOOK_EDIT_PRO_3310:
         {
            if (*_pszSignature && (_iSectNum == 1))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_EDIT_PRO_3310_NP)->_pszHash))
               {
                  return ET_EBOOK_EDIT_PRO_3310_NP;
               }
            }
            
            if (*_pszSignature && (_iSectNum == 2))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_EDIT_PRO_3310_1)->_pszHash))
               {
                  return ET_EBOOK_EDIT_PRO_3310_1;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_EDIT_PRO_3310_2)->_pszHash))
               {
                  return ET_EBOOK_EDIT_PRO_3310_2;
               }
            }

            return pFound->_BookType;
         }
         case ET_EBOOK_MAESTRO:
         {
            if (*_pszSignature && (_iSectNum == 2))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_MAESTRO_FREE_1)->_pszHash))
               {
                  return ET_EBOOK_MAESTRO_FREE_1;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_MAESTRO_FREE_2)->_pszHash))
               {
                  return ET_EBOOK_MAESTRO_FREE_2;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_MAESTRO_FREE_3)->_pszHash))
               {
                  return ET_EBOOK_MAESTRO_FREE_3;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_MAESTRO_PRO)->_pszHash))
               {
                  return ET_EBOOK_MAESTRO_PRO;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_EBOOK_MAESTRO_PRO_DEMO)->_pszHash))
               {
                  return ET_EBOOK_MAESTRO_PRO_DEMO;
               }

               return pFound->_BookType;
            }
         }
         case ET_RAR_ZIP_SFX_4:
         {
            if (*_pszSignature && (_iSectNum == 1))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_E_DITOR_2_1)->_pszHash))
               {
                  return ET_E_DITOR_2_1;
               }
            }

            if (*_pszSignature && (_iSectNum == 2))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_KE_EBOOK_CREATOR)->_pszHash))
               {
                  return ET_KE_EBOOK_CREATOR;
               }
            }
            
            return pFound->_BookType;
         }
         case ET_ZIP_BCB_DELPHI:
         {
            if (*_pszSignature && (_iSectNum == 1))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_E_DITOR_2_2)->_pszHash))
               {
                  return ET_E_DITOR_2_2;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_TETERIN)->_pszHash))
               {
                  return ET_TETERIN;
               }
            }

            if (*_pszSignature && (_iSectNum == 2))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_RAR_ZIP_SFX_2)->_pszHash))
               {
                  return ET_RAR_ZIP_SFX_2;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_RAR_ZIP_SFX_5)->_pszHash))
               {
                  return ET_RAR_ZIP_SFX_5;
               }
            }

            return pFound->_BookType;
         }
         case ET_CUPDAT:
         {
            if (*_pszSignature && (_iSectNum == 1))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_FAST_EBOOK_COMPILER_1X)->_pszHash))
               {
                  return ET_FAST_EBOOK_COMPILER_1X;
               }
               
               if (!strcmp(_pszSignature,FindRecord(ET_FAST_EBOOK_COMPILER_14)->_pszHash))
               {
                  return ET_FAST_EBOOK_COMPILER_14;
               }
               
               if (!strcmp(_pszSignature,FindRecord(ET_JIMMY_EBOOK_CREATOR)->_pszHash))
               {
                  return ET_JIMMY_EBOOK_CREATOR;
               }
               
               if (!strcmp(_pszSignature,FindRecord(ET_JIMMY_EBOOK_CREATOR_2000_1)->_pszHash))
               {
                  return ET_JIMMY_EBOOK_CREATOR_2000_1;
               }
               
               if (!strcmp(_pszSignature,FindRecord(ET_JIMMY_EBOOK_CREATOR_2000_2)->_pszHash))
               {
                  return ET_JIMMY_EBOOK_CREATOR_2000_2;
               }
               
               if (!strcmp(_pszSignature,FindRecord(ET_JIMMY_EBOOK_CREATOR_2000_3)->_pszHash))
               {
                  return ET_JIMMY_EBOOK_CREATOR_2000_3;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_JIMMY_EBOOK_CREATOR_2000_4)->_pszHash))
               {
                  return ET_JIMMY_EBOOK_CREATOR_2000_4;
               }

               E_BOOK_TYPE    BookType = FastEbookCompilerCheck();

               if (BookType != ET_UNKNOWN_EBOOK)
               {
                  return BookType;
               }

               BookType = EbookCreatorCheck();

               if (BookType != ET_UNKNOWN_EBOOK)
               {
                  return BookType;
               }
            }

            return pFound->_BookType;
         }
         case ET_NATATA_V_BOOK_EBOOK_COMPILER:
         {
            if (*_pszSignature && (_iSectNum == 1))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_NATATA_EBOOK_COMPILER_30)->_pszHash))
               {
                  return ET_NATATA_EBOOK_COMPILER_30;
               }
            }

            if (*_pszSignature && (_iSectNum == 2))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_NATATA_EBOOK_COMPILER_21)->_pszHash))
               {
                  return ET_NATATA_EBOOK_COMPILER_21;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_NATATA_EBOOK_COMPILER_21_2)->_pszHash))
               {
                  return ET_NATATA_EBOOK_COMPILER_21_2;
               }
            
               if (!strcmp(_pszSignature,FindRecord(ET_NATATA_EBOOK_COMPILER_21_DEMO)->_pszHash))
               {
                  return ET_NATATA_EBOOK_COMPILER_21_DEMO;
               }
            
               if (!strcmp(_pszSignature,FindRecord(ET_NATATA_EBOOK_COMPILER_2004)->_pszHash))
               {
                  return ET_NATATA_EBOOK_COMPILER_2004;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_NATATA_EBOOK_COMPILER_2005)->_pszHash))
               {
                  return ET_NATATA_EBOOK_COMPILER_2005;
               }
            
               if (!strcmp(_pszSignature,FindRecord(ET_NATATA_EBOOK_COMPILER_2009)->_pszHash))
               {
                  return ET_NATATA_EBOOK_COMPILER_2009;
               }
            
               if (!strcmp(_pszSignature,FindRecord(ET_NATATA_EBOOK_COMPILER_2010)->_pszHash))
               {
                  return ET_NATATA_EBOOK_COMPILER_2010;
               }
            }
            
            return pFound->_BookType;
         }
         case ET_WEB_EXE:
         {
            if (*_pszSignature && (_iSectNum == 1))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_WEB_EXE_03)->_pszHash))
               {
                  return ET_WEB_EXE_03;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_WEB_EXE_ACME)->_pszHash))
               {
                  return ET_WEB_EXE_ACME;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_WEB_EXE_AZRAEL)->_pszHash))
               {
                  return ET_WEB_EXE_AZRAEL;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_WEB_EXE_COA)->_pszHash))
               {
                  return ET_WEB_EXE_COA;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_WEB_EXE_CRACKED)->_pszHash))
               {
                  return ET_WEB_EXE_CRACKED;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_WEB_EXE_CRACKED_2)->_pszHash))
               {
                  return ET_WEB_EXE_CRACKED_2;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_WEB_EXE_CRACKED_3)->_pszHash))
               {
                  return ET_WEB_EXE_CRACKED_3;
               }
            }

            if (*_pszSignature && (_iSectNum == 2))
            {
               if (!strcmp(_pszSignature,FindRecord(ET_WEB_EXE_01)->_pszHash))
               {
                  return ET_WEB_EXE_01;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_WEB_EXE_04)->_pszHash))
               {
                  return ET_WEB_EXE_04;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_WEB_EXE_2000_1)->_pszHash))
               {
                  return ET_WEB_EXE_2000_1;
               }

               if (!strcmp(_pszSignature,FindRecord(ET_WEB_EXE_2000_2)->_pszHash))
               {
                  return ET_WEB_EXE_2000_2;
               }
            }

            return pFound->_BookType;
         }
#if 0
///////////////////////////////////////////////////////////////////////////////
         case :
         {
   /*
   {  0, 1, 0, DT_HASH,    ET_RAR_WIN32_SFX_320,                     "",                                 HINT_NONE,             0,  "CFF268651764062455AEEA090703F318",    2,    STR_RAR_WIN32_SFX_320,                    "3.20",        "",                     "",         0  },
   {  0, 1, 0, DT_HASH,    ET_RAR_ZIP_SFX_2,                         "",                                 HINT_NONE,             0,  "50C2094638E9CAADF2ACC8A81418FC8B",    2,    STR_RAR_ZIP_SFX,                          "<2>",         "",                     "",         0  },
   */
            return pFound->_BookType;
         }
///////////////////////////////////////////////////////////////////////////////
#endif
         default:
         {
            return pFound->_BookType;
         }
      }
   }
   else if (pFound)
   {
      if ((pFound->_BookType == ET_ZIP_BCB_DELPHI) && (pFound->_iFoundAt == FindRecord(ET_EBOOKS_WRITER)->_iSignHintOfs) && IsEbooksWriter())
      {
         return ET_EBOOKS_WRITER;
      }

      if (pFound->_BookType == ET_ACKER_PACK)
      {
         if (!strcmp(_pszSignature,FindRecord(ET_ACKER_PACK_121)->_pszHash))
         {
            return ET_ACKER_PACK_121;
         }

         return ET_ACKER_PACK;
      }

      return ET_UNKNOWN_EBOOK;
   }
   else  // NULL !
   {
      if (*_pszSignature && (_iSectNum == 1))
      {
         if (!strcmp(_pszSignature,FindRecord(ET_BOOK_BIZ_MASTER_20_ENCRYPTED)->_pszHash))
         {
            return ET_BOOK_BIZ_MASTER_20_ENCRYPTED;
         }
      }

      if (IsBookMaker_21())
      {
         return ET_EBOOK_MAKER_21;
      }
   
      return ET_UNKNOWN_EBOOK;
   }
}

/* ******************************************************************** **
** @@ GetCompilerOverlayType()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static E_BOOK_TYPE GetCompilerOverlayType(const BYTE* const pBuf,DWORD dwSize)
{
   int      iBufSize    = 0;
   int      iBufStart   = 0;
   BYTE*    pBufStart   = NULL;
   int      iMaxSignLen = 0;
   DWORD    dwFoundAt   = 0;

   AC_STATE*      pInitState = NULL;
   AC_STATE*      pLastFound = NULL;

   SSF_TEXT_RECORD*     pFound = NULL;

   ////////////////////////////////////////////////////////////////////////////
   // Start from Begin of Overlay    
   iMaxSignLen = OVERLAY_SIZE_MIN; // For possible shifted signatures

   iBufStart = 0;
   iBufSize  = iMaxSignLen;
   pBufStart = (BYTE*)(pBuf + iBufStart);

   if (IsBadReadPtr(pBufStart,iBufStart))
   {
      // Error !
      fprintf(_pOut,"\nErr: Can't read from Overlay at offset %08X.\n",iBufStart);
      _MF.Close();
      fclose(_pOut);
      _pOut = NULL;
      return ET_NONE;
   }

   pInitState = _pACS->_pRoot;
   pLastFound = NULL;

   pFound = (SSF_TEXT_RECORD*)_pACS->FindFirstLongest(pBufStart,iBufSize,0,dwFoundAt);

   if (pFound && (pFound->_BookType > ET_UNKNOWN_EBOOK))
   {
      // printf("In the file %s found %s %s at offset %08X\n",pszFilename,pEntry->_pszName,pEntry->_pszVers,dwSizeStub + dwSizeOver - MaxSignLen + iFoundAt);
      pFound->_iFoundAt = (short)dwFoundAt;

      E_BOOK_TYPE    BookType = EnsureBookType(pFound);

      if (BookType > ET_UNKNOWN_EBOOK)
      {
         return BookType;
      }
   }

   ////////////////////////////////////////////////////////////////////////////
   // Start from End of Overlay    
   iMaxSignLen = 12;

   iBufStart = dwSize - iMaxSignLen;
   iBufSize  = iMaxSignLen;
   pBufStart = (BYTE*)(pBuf + iBufStart);

   if (IsBadReadPtr(pBufStart,iBufSize))
   {
      // Error !
      fprintf(_pOut,"\nErr: Can't read from Overlay at offset %08X.\n",iBufStart);
      _MF.Close();
      fclose(_pOut);
      _pOut = NULL;
      return ET_NONE;
   }

   pInitState = _pACS->_pRoot;
   pLastFound = NULL;

   pFound = (SSF_TEXT_RECORD*)_pACS->FindFirstLongest(pBufStart,iBufSize,0,dwFoundAt);

   if (pFound)
   {
      // printf("In the file %s found %s %s at offset %08X\n",pszFilename,pEntry->_pszName,pEntry->_pszVers,dwSizeStub + dwSizeOver - MaxSignLen + iFoundAt);
      pFound->_iFoundAt = (short)(dwFoundAt - iMaxSignLen);

      E_BOOK_TYPE    BookType = EnsureBookType(pFound);

      if (BookType > ET_UNKNOWN_EBOOK)
      {
         return BookType;
      }
   }

   return EnsureBookType(NULL);
}

/* ******************************************************************** **
** @@ KeepOverlay()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void KeepOverlay(const char* const pszFilename,const BYTE* const pBuf,DWORD dwSize)
{
   char     pszTemp[MAX_PATH + 1];

   memset(pszTemp,0,sizeof(pszTemp));

   char     pszDrive   [_MAX_DRIVE];
   char     pszDir     [_MAX_DIR];
   char     pszFName   [_MAX_FNAME];
   char     pszExt     [_MAX_EXT];

   _splitpath(pszFilename,pszDrive,pszDir,pszFName,pszExt);
   _makepath (pszTemp,    pszDrive,pszDir,pszFName,"ovr");

   HANDLE      hOvr = CreateFile(pszTemp);

   if (hOvr == INVALID_HANDLE_VALUE)
   {
      // Error !
      printf("Err: Can't create overlay file.\n");
      return;
   }

   if (!WriteBuffer(hOvr,pBuf,dwSize))
   {
      // Error !
      CloseHandle(hOvr);
      hOvr = INVALID_HANDLE_VALUE;
      return;
   }

   CloseHandle(hOvr);
   hOvr = INVALID_HANDLE_VALUE;
   return;
}

/* ******************************************************************** **
** @@ PublicantCheck()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static E_BOOK_TYPE PublicantCheck()
{
   const int   iPubTailSize = 1536;

   if (_MF.Size() < iPubTailSize)
   {
      return ET_NONE;
   }

   BYTE*    pBuf = _MF.Buffer() + _MF.Size() - iPubTailSize;

   // zreader
   const char*    pszPattern1 = "\xCC\x00\x00\x00\x00";
   const char*    pszPattern2 = "\x00\x00\x00\x01\x7A\x72\x65\x61\x64\x65\x72\x00"; // zreader

   int      iPatSize1 = 5;
   int      iPatSize2 = 8;

   int   iFoundAt = BMH_Search(pBuf,iPubTailSize,(BYTE*)pszPattern1,iPatSize1);   

   if ((iFoundAt >= 0) && !memcmp(pBuf + iFoundAt + iPatSize1 + 1,pszPattern2,iPatSize2))
   {
      BYTE     byType = *(pBuf + iFoundAt + iPatSize1);

      if (*_pszSignature && (_iSectNum == 1))
      {
         if (!strcmp(_pszSignature,FindRecord(ET_PUBLICANT_EXE_BOOK_01)->_pszHash))
         {
            // 0x3A
            return ET_PUBLICANT_EXE_BOOK_01;
         }
       
         if (!strcmp(_pszSignature,FindRecord(ET_PUBLICANT_EXE_BOOK_02)->_pszHash))
         {
            // 0x39
            return ET_PUBLICANT_EXE_BOOK_02;
         }

         if (!strcmp(_pszSignature,FindRecord(ET_PUBLICANT_EXE_BOOK_03)->_pszHash))
         {
            // 0x2E
            return ET_PUBLICANT_EXE_BOOK_03;
         }
         
         if (!strcmp(_pszSignature,FindRecord(ET_PUBLICANT_EXE_BOOK_04)->_pszHash))
         {
            // 0x2E
            return ET_PUBLICANT_EXE_BOOK_04;
         }
         
         if (!strcmp(_pszSignature,FindRecord(ET_PUBLICANT_EXE_BOOK_05)->_pszHash))
         {
            // 0x2D
            return ET_PUBLICANT_EXE_BOOK_05;
         }
         
         if (!strcmp(_pszSignature,FindRecord(ET_PUBLICANT_EXE_BOOK_06)->_pszHash))
         {
            // 0x39
            return ET_PUBLICANT_EXE_BOOK_06;
         }
         
         if (!strcmp(_pszSignature,FindRecord(ET_PUBLICANT_EXE_BOOK_07)->_pszHash))
         {
            // 0x2E
            return ET_PUBLICANT_EXE_BOOK_07;
         }
         
         if (!strcmp(_pszSignature,FindRecord(ET_PUBLICANT_EXE_BOOK_08)->_pszHash))
         {
            // 0x3C
            return ET_PUBLICANT_EXE_BOOK_08;
         }
         
         if (!strcmp(_pszSignature,FindRecord(ET_PUBLICANT_EXE_BOOK_09)->_pszHash))
         {
            // 0x31
            return ET_PUBLICANT_EXE_BOOK_09;
         }
         
         if (!strcmp(_pszSignature,FindRecord(ET_PUBLICANT_EXE_BOOK_10)->_pszHash))
         {
            // 0x38
            return ET_PUBLICANT_EXE_BOOK_10;
         }
      }

      switch (byType)
      {
         case 0x2D:
         {
            return ET_PUBLICANT_EXE_BOOK_2D;
         }
         case 0x2E:
         {
            return ET_PUBLICANT_EXE_BOOK_2E;
         }
         case 0x31:
         {
            return ET_PUBLICANT_EXE_BOOK_31;
         }
         case 0x38:
         {
            return ET_PUBLICANT_EXE_BOOK_38;
         }
         case 0x39:
         {
            return ET_PUBLICANT_EXE_BOOK_39;
         }
         case 0x3A:
         {
            return ET_PUBLICANT_EXE_BOOK_3A;
         }
         case 0x3C:
         {
            return ET_PUBLICANT_EXE_BOOK_3C;
         }
         default:
         {
            return ET_PUBLICANT_EXE_BOOK;
         }
      }
   }

   return ET_NONE;
}

/* ******************************************************************** **
** @@ GetExeStubType()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static E_BOOK_TYPE GetExeStubType(const BYTE* const pBuf,DWORD dwSize)
{
   if (!*_pszSignature)
   {
      return ET_UNKNOWN_EXE_STUB;
   }

   for (int ii = 1; ii < E_TEXT_DB_SIZE; ++ii)
   {
      if (*pTextDB[ii]._pszHash && (_iSectNum == pTextDB[ii]._bySectNum))
      {
         if (!strcmp(_pszSignature,FindRecord(pTextDB[ii]._BookType)->_pszHash))
         {
            return pTextDB[ii]._BookType;
         }
      }
   }

   return ET_UNKNOWN_EXE_STUB;
}

/* ******************************************************************** **
** @@ ForEach()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void ForEach(const char* const pszFilename)
{
   char     pszTemp[MAX_PATH + 1];

   char     pszDrive   [_MAX_DRIVE];
   char     pszDir     [_MAX_DIR];
   char     pszFName   [_MAX_FNAME];
   char     pszExt     [_MAX_EXT];

   _dwStubSize = 0;  // Initialize
   _dwOverSize = 0;

   _iSectNum = 0;

   memset(_pszSignature,0,sizeof(_pszSignature));

   _splitpath(pszFilename,pszDrive,pszDir,pszFName,pszExt);

   strcpy(pszTemp,pszFilename);
   strcat(pszTemp,".info");

   if (!_MF.OpenReadOnly(pszFilename))
   {
      // Error !
      printf("\nErr: Can't open for RO. [%s]\n",pszFilename);
      return;
   }

   _pOut = fopen(pszTemp,"wt");

   if (!_pOut)
   {
      // Error !
      printf("\nErr: Can't create Info file. [%s]\n",pszTemp);
      return;
   }

   fprintf(_pOut,"-*-   EBU  " EBU_VERSION "  *  (c) gazlan@yanex.ru 2010, 2011   -*-\n\n");

   if (!_bJustOverlay)
   {
      if (_TheFile.Open(_MF.Buffer(),_MF.Size()))
      {
         PrintHeader(pszFilename);
      }
      else
      {
         // Error !
         fprintf(_pOut,"\nErr: Is not valid PE file. [%s]\n",pszFilename);

         if (!_bIgnoreValidation)
         {
            _MF.Close();
            fclose(_pOut);
            _pOut = NULL;
            return;
         }
      }

      PIMAGE_SECTION_HEADER   pSect = _TheFile.GetLastSection();

      if (!pSect)
      {
         fprintf(_pOut,"\nErr: File is corrupt. [%s]\n",pszFilename);
         _MF.Close();
         fclose(_pOut);
         _pOut = NULL;
         return;
      }

      _dwStubSize = pSect->PointerToRawData + pSect->SizeOfRawData;
   }
   else
   {
      _dwStubSize = 0;
   }

   _dwOverSize = _MF.Size() - _dwStubSize;;

   if (_MF.Size() < _dwStubSize)
   {
      // Error !
      fprintf(_pOut,"\nErr: Incorrect PE-header info. [%s]\n",pszFilename);
      _MF.Close();
      fclose(_pOut);
      _pOut = NULL;
      return;
   }

   if (!_bJustOverlay)
   {
      PIMAGE_SECTION_HEADER   pRealSect = NULL;

      // Find First Real section
      int   iSectCnt = _TheFile.GetSectCnt();

      for (int ii = 0; ii < iSectCnt; ++ii)
      {
         pRealSect = _TheFile.GetSection(ii) ;

         if (!pRealSect)
         {
            // Error !
            fprintf(_pOut,"\nErr: Can't get section %d info. [%s]\n",ii,pszFilename);
            _MF.Close();
            fclose(_pOut);
            _pOut = NULL;
            return;
         }

         if (pRealSect->SizeOfRawData > 0)
         {
            _iSectNum = ii + 1;

            MD5      _Hash;

            DWORD    dwSectStart = pRealSect->PointerToRawData;
            DWORD    dwSectSize  = pRealSect->SizeOfRawData;

            _Hash.Reset();
            _Hash.Update((BYTE*)(_MF.Buffer() + dwSectStart),dwSectSize);
            _Hash.Finalize();

            strcpy(_pszSignature,_Hash.Signature());

            break;
         }
      }
   }

   // Check for Nonenveloped files
   // Any way try first there very special checks
   char     pszComment[MAX_PATH + 1];

   memset(pszComment,0,sizeof(pszComment));

   BYTE*    pOverlay = NULL;

   E_BOOK_TYPE    BookType = ET_NONE;
   E_BOOK_TYPE    StubType = ET_NONE;

   BookType = PublicantCheck();

   if (!BookType)
   {
      if (_dwOverSize < OVERLAY_SIZE_MIN)
      {
         // Error !
         fprintf(_pOut,"\nErr: Isn't enveloped file. [%s] ! Overlay size: $%02X\n",pszFilename,_dwOverSize);
         _MF.Close();
         fclose(_pOut);
         _pOut = NULL;
         return;
      }

      pOverlay = _MF.Buffer() + _dwStubSize;

      if (IsBadReadPtr(pOverlay,_dwOverSize))
      {
         // Error !
         fprintf(_pOut,"\nErr: Incorrect PE-header info. [%s]\n",pszFilename);
         _MF.Close();
         fclose(_pOut);
         _pOut = NULL;
         return;
      }

      if (_dwStubSize & 0x000000FF)
      {
         fprintf(_pOut,"*   Overlay start: %08X   !!  Is Not Aligned  !!\n\n",_dwStubSize);
      }

      if (_dwOverSize > 0x10 * 5) // One extra line for padding (if any)
      {
      
         Ruller(_pOut);
         HexDumpQLine(_MF.Buffer(),_MF.Size(),_dwStubSize,_pOut);
   
         Ruller(_pOut);
         HexDumpQLine(_MF.Buffer(),_MF.Size(),_dwStubSize + _dwOverSize - 0x10 * 4,_pOut);
   
         Ruller(_pOut);
         fprintf(_pOut,"\n");
      }

      // Keep the overlay
      if (_bKeepOverlay)
      {
         KeepOverlay(pszFilename,pOverlay,_dwOverSize);
      }
   
      BookType = GetCompilerOverlayType(pOverlay,_dwOverSize);
      StubType = GetExeStubType(_MF.Buffer(),_dwStubSize);
   }

   WIN32_FILE_ATTRIBUTE_DATA     Attr;
   
   memset(&Attr,0,sizeof(Attr));
    
   char     pszTimeStamp[MAX_PATH + 1];

   memset(pszTimeStamp,0,sizeof(pszTimeStamp));

   if (GetFileAttributesEx(pszFilename,GetFileExInfoStandard,&Attr))
   {
      FILETIME       LocalTime;
      SYSTEMTIME     TimeStamp;

      if (FileTimeToLocalFileTime(&Attr.ftCreationTime,&LocalTime))
      {
         if (FileTimeToSystemTime(&LocalTime,&TimeStamp))
         {
            sprintf(pszTimeStamp,"%04d-%02d-%02d  %02d:%02d:%02d",TimeStamp.wYear,TimeStamp.wMonth,TimeStamp.wDay,TimeStamp.wHour,TimeStamp.wMinute,TimeStamp.wSecond);
         }
      }
   }

   DWORD       dwCRC32 = 0;

   CRC32*      pHash = new CRC32;

   if (pHash)
   {
      pHash->Reset();
   
      dwCRC32 = pHash->Calc(_MF.Buffer(),_MF.Size());

      delete pHash;
      pHash = NULL;
   }

   // Print
   printf("\nFile >>%12lu  %s  $%08X  |  %s\n",_MF.Size(),pszTimeStamp,dwCRC32,pszFilename);
   printf("$0%02X >> %s  |  Stub: $%08X  Over: $%08X\n",_iSectNum,_pszSignature,_dwStubSize,_dwOverSize);

   SSF_TEXT_RECORD*  pStub = (SSF_TEXT_RECORD*)FindRecord(StubType);

   if (pStub)
   {
      printf("Stub >> %s",pStub->_pszName);

      if (*pStub->_pszVersion)
      {
         printf("  [v. %s]",pStub->_pszVersion);
      }

      if (*pStub->_pszComment)
      {
         printf("  // %s",pStub->_pszComment);
      }

      printf("\n");
   }

   SSF_TEXT_RECORD*  pRecord = (SSF_TEXT_RECORD*)FindRecord(BookType);

   if (pRecord)
   {
      printf("Envl >> %s",pRecord->_pszName);

      if (*pRecord->_pszVersion)
      {
         printf("  [v. %s]",pRecord->_pszVersion);
      }

      if (*pRecord->_pszComment)
      {
         printf("  // %s",pRecord->_pszComment);
      }

      if (_bInfoOnly && pRecord->_bUnpack)
      {
         printf("\n[!] Unpackable\n");
      }
      else
      {
         printf("\n");
      }
   }

   if (*pszComment)
   {
      printf("%s\n",pszComment);
   }

   // Write
   fprintf(_pOut,"\nFile >>%12lu  %s  $%08X  |  %s\n",_MF.Size(),pszTimeStamp,dwCRC32,pszFilename);
   fprintf(_pOut,"$0%02X >> %s  |  Stub: $%08X  Over: $%08X\n",_iSectNum,_pszSignature,_dwStubSize,_dwOverSize);

   if (pStub)
   {
      fprintf(_pOut,"Stub >> %s",pStub->_pszName);

      if (*pStub->_pszVersion)
      {
         fprintf(_pOut,"  [v. %s]",pStub->_pszVersion);
      }

      if (*pStub->_pszComment)
      {
         fprintf(_pOut,"  // %s",pStub->_pszComment);
      }

      fprintf(_pOut,"\n");
   }

   if (pRecord)
   {
      fprintf(_pOut,"Envl >> %s",pRecord->_pszName);

      if (*pRecord->_pszVersion)
      {
         fprintf(_pOut,"  [v. %s]",pRecord->_pszVersion);
      }

      if (*pRecord->_pszComment)
      {
         fprintf(_pOut,"  // %s",pRecord->_pszComment);
      }

      if (_bInfoOnly && pRecord->_bUnpack)
      {
         fprintf(_pOut,"\n[!] Unpackable\n");
      }
         else
      {
         fprintf(_pOut,"\n");
      }
   }

   if (*pszComment)
   {
      fprintf(_pOut,"%s\n",pszComment);
   }

   if (_bInfoOnly)
   {
      // Don't unpack
      _MF.Close();
      fclose(_pOut);
      _pOut = NULL;
      return;
   }

   switch (BookType)
   {
      case ET_EBOOK_EDIT_PRO_3X:
      case ET_EBOOK_EDIT_PRO_3200:
      case ET_EBOOK_EDIT_PRO_3210:
      case ET_EBOOK_EDIT_PRO_3210_1:
      case ET_EBOOK_EDIT_PRO_3210_NP_1:
      case ET_EBOOK_EDIT_PRO_3210_NP_2:
      case ET_EBOOK_EDIT_PRO_3210_NP_3:
      case ET_EBOOK_EDIT_PRO_3300:
      case ET_EBOOK_EDIT_PRO_3310:
      case ET_EBOOK_EDIT_PRO_3310_1:
      case ET_EBOOK_EDIT_PRO_3310_2:
      case ET_EBOOK_EDIT_PRO_3310_NP:
      case ET_EBOOK_EDIT_PRO_3340_1:
      case ET_EBOOK_EDIT_PRO_3340_2:
      case ET_EBOOK_EDIT_PRO_3341:
      {
         _pUnpacker = new E_Unpacker_EEP(_dwStubSize,pOverlay,_dwOverSize,_pOut);

         if (!_pUnpacker)
         {
            SSF_TEXT_RECORD*     pRecord = (SSF_TEXT_RECORD*)FindRecord(BookType);

            fprintf(_pOut,"\nErr: Can't create Unpacker object %s\n",pRecord  ?  pRecord->_pszName  :  "");
            _MF.Close();
            fclose(_pOut);
            _pOut = NULL;
            return;
         }

         break;
      }
      case ET_NATATA_EBOOK_COMPILER_21:
      case ET_NATATA_EBOOK_COMPILER_21_2:
      case ET_NATATA_EBOOK_COMPILER_21_DEMO:
      case ET_NATATA_EBOOK_COMPILER_2004:
      case ET_NATATA_EBOOK_COMPILER_2005:
      case ET_NATATA_EBOOK_COMPILER_2009:
      case ET_NATATA_EBOOK_COMPILER_2010:
      case ET_NATATA_EBOOK_COMPILER_30:
      case ET_NATATA_V_BOOK_EBOOK_COMPILER:
      {
         _pUnpacker = new E_Unpacker_Tatu(_dwStubSize,pOverlay,_dwOverSize,_pOut);

         if (!_pUnpacker)
         {
            SSF_TEXT_RECORD*  pRecord = (SSF_TEXT_RECORD*)FindRecord(BookType);

            fprintf(_pOut,"\nErr: Can't create Unpacker object %s\n",pRecord  ?  pRecord->_pszName  :  "");
            _MF.Close();
            fclose(_pOut);
            _pOut = NULL;
            return;
         }

         break;
      }
      case ET_JANSOFT_SBOOKBUILDER:
      {
         _pUnpacker = new E_Unpacker_JSSB(_dwStubSize,pOverlay,_dwOverSize,_pOut);

         if (!_pUnpacker)
         {
            SSF_TEXT_RECORD*  pRecord = (SSF_TEXT_RECORD*)FindRecord(BookType);

            fprintf(_pOut,"\nErr: Can't create Unpacker object %s\n",pRecord  ?  pRecord->_pszName  :  "");
            _MF.Close();
            fclose(_pOut);
            _pOut = NULL;
            return;
         }

         break;
      }
#if 0
///////////////////////////////////////////////////////////////////////////////
      case ET_COINSOFT_EBOOK_MAKER_1:
      case ET_COINSOFT_EBOOK_MAKER_2:
      case ET_COINSOFT_EBOOK_MAKER:
      {
         _pUnpacker = new E_Unpacker_Coin(dwSizeStub,pOverlay,dwSizeOver,_pOut);

         if (!_pUnpacker)
         {
            SSF_TEXT_RECORD*  pRecord = (SSF_TEXT_RECORD*)FindRecord(BookType);

            fprintf(_pOut,"\nErr: Can't create Unpacker object %s\n",pRecord  ?  pRecord->_pszName  :  "");
            _MF.Close();
            fclose(_pOut);
            _pOut = NULL;
            return;
         }

         break;
      }
///////////////////////////////////////////////////////////////////////////////
#endif
      default:
      {
         SSF_TEXT_RECORD*  pRecord = (SSF_TEXT_RECORD*)FindRecord(BookType);

         fprintf(_pOut,"\nErr: Is not yet supported e-book type [%s]\n",pRecord  ?  pRecord->_pszName  :  "");
         _MF.Close();
         fclose(_pOut);
         _pOut = NULL;
         return;
      }
   }

   strcpy(E_Unpacker::_pszBook,pszFName);

   _pUnpacker->Go(pszFilename);

   delete _pUnpacker;
   _pUnpacker = NULL;

   _MF.Close();

   fclose(_pOut);
   _pOut = NULL;
}

/* ******************************************************************** **
** @@ Autotest()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

#ifdef _DEBUG
static bool Autotest()
{

// Check for Hash dups !!

   for (int ii = 1; ii < E_TEXT_DB_SIZE; ++ii)
   {
      // Ensure ASCIIZ
      pTextDB[ii]._pszName     [MAX_NAME_SIZE]     = 0;
      pTextDB[ii]._pszVersion  [MAX_VERS_SIZE]     = 0;
      pTextDB[ii]._pszSignature[MAX_SIGN_SIZE]     = 0;
      pTextDB[ii]._pszComment  [MAX_COMMENT_SIZE]  = 0;
      pTextDB[ii]._pszAppendix [MAX_APPENDIX_SIZE] = 0;

      if (pTextDB[ii]._BookType <= pTextDB[ii - 1]._BookType)
      {
         printf("!!  Autotest error in the line %d:  *%d <= %d*  ::  [%s] <= [%s]\n",ii,pTextDB[ii]._BookType,pTextDB[ii - 1]._BookType,pTextDB[ii]._pszName,pTextDB[ii - 1]._pszName);
         printf("*  Resort in ascending sort order & recompile *\n\n");
         return false;
      }

      if (*pTextDB[ii]._pszSignature && (((strlen(pTextDB[ii]._pszSignature) + 1) >> 1) < 4))
      {
         printf("!!  WARNING: in the line %d: *VERY_SHORT_SIGNATURE*  <%s>  [%s]\n",ii,pTextDB[ii]._pszSignature,pTextDB[ii]._pszName);
         printf("*  Correct as need & recompile *\n\n");
      }

      if (*pTextDB[ii]._pszSignature && (((strlen(pTextDB[ii]._pszSignature) + 1) >> 1) > 16))
      {
         printf("!!  Autotest error in the line %d: *VERY_LONG_SIGNATURE*  <%s>  [%s]\n",ii,pTextDB[ii]._pszSignature,pTextDB[ii]._pszName);
         printf("*  Correct as need & recompile *\n\n");
         return false;
      }

      if ((pTextDB[ii]._DetectType == DT_NORMAL) && !*pTextDB[ii]._pszSignature)
      {
         printf("!!  Autotest error in the line %d: *EMPTY_SIGNATURE*  [%s]\n",ii,pTextDB[ii]._pszName);
         printf("*  Correct as need & recompile *\n\n");
         return false;
      }

      if ((pTextDB[ii]._DetectType == DT_HASH) && !pTextDB[ii]._bySectNum)
      {
         printf("!!  Autotest error in the line %d: *EMPTY_SECTION_NUM*  [%s]\n",ii,pTextDB[ii]._pszName);
         printf("*  Correct as need & recompile *\n\n");
         return false;
      }

      if ((pTextDB[ii]._DetectType == DT_HASH) && *pTextDB[ii]._pszSignature)
      {
         printf("!!  Autotest error in the line %d: *NON_EMPTY_SIGNATURE*  [%s]\n",ii,pTextDB[ii]._pszName);
         printf("*  Correct as need & recompile *\n\n");
         return false;
      }

      if ((pTextDB[ii]._DetectType == DT_HASH) && (pTextDB[ii]._SignHint || pTextDB[ii]._iSignHintOfs))
      {
         printf("!!  Autotest error in the line %d: *NON_EMPTY_HINT*  [%s]\n",ii,pTextDB[ii]._pszName);
         printf("*  Correct as need & recompile *\n\n");
         return false;
      }

      if ((pTextDB[ii]._DetectType == DT_HASH) && !*pTextDB[ii]._pszHash)
      {
         printf("!!  Autotest error in the line %d: *EMPTY_HASH*  [%s]\n",ii,pTextDB[ii]._pszName);
         printf("*  Correct as need & recompile *\n\n");
         return false;
      }

      if ((pTextDB[ii]._DetectType == DT_NORMAL) && (pTextDB[ii]._SignHint == HINT_OVERLAY_BEGIN ) && (pTextDB[ii]._iSignHintOfs < 0))
      {
         printf("!!  Autotest error in the line %d: *HINT_OVERLAY_BEGIN < 0*  [%s]\n",ii,pTextDB[ii]._pszName);
         printf("*  Correct as need & recompile *\n\n");
         return false;
      }

      if ((pTextDB[ii]._DetectType == DT_NORMAL) && (pTextDB[ii]._SignHint == HINT_OVERLAY_END ) && (pTextDB[ii]._iSignHintOfs >= 0))
      {
         printf("!!  Autotest error in the line %d: *HINT_OVERLAY_END >= 0*  [%s]\n",ii,pTextDB[ii]._pszName);
         printf("*  Correct as need & recompile *\n\n");
         return false;
      }
   }

   return true;
}
#endif

/* ******************************************************************** **
** @@ ShowHelp()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void ShowHelp()
{
   printf("-*-   EBU  " EBU_VERSION "  *  (c) gazlan 2010, 2011   -*-\n");
   printf("\ne-book Unpacker\n");
   printf("\nUsage: ebu.exe [{-p|-o}|-i|-k] wildcards\n\n");
   printf("-i   - Don't try to unpack, show info only\n");
   printf("-k   - Keep the file's overlay on the disk\n");
   printf("-o   - Force treat the file as overlay, not an .exe\n");
   printf("-p   - Ignore PE validation check\n");
   printf("\ncomplains_n_suggestions direct to gazlan@yandex.ru\n\n");

   // DETECT ONLY
   printf("\nDetect:\n\n");

   for (int ii = 0; ii < E_TEXT_DB_SIZE; ++ii)
   {
      #ifdef _DEBUG
      if (pTextDB[ii]._bDetect)
      #else
      if (pTextDB[ii]._bDetect && pTextDB[ii]._bPrint)
      #endif
      {
         SSF_TEXT_RECORD*  pRecord = (SSF_TEXT_RECORD*)GetRecord(ii);

         printf("\t%s",pRecord->_pszName);

         if (*pRecord->_pszVersion)
         {  
            printf("  [v. %s]",pRecord->_pszVersion);
         }

         switch (pRecord->_DetectType)
         {
            case DT_NORMAL:
            {
               break;
            }
            case DT_HASH:
            {
               printf(" * Detect by HASH only!");
               break;
            }
            case DT_EXTRA:
            {
               printf(" * Detect by Extra checks!");
               break;
            }
         }

         printf("\n");
      }
   }

   // UNPACK
   printf("\nUnpack:\n\n");

   for (ii = 0; ii < E_TEXT_DB_SIZE; ++ii)
   {
      #ifdef _DEBUG
      if (pTextDB[ii]._bUnpack)
      #else
      if (pTextDB[ii]._bUnpack && pTextDB[ii]._bPrint)
      #endif
      {
         SSF_TEXT_RECORD*  pRecord = (SSF_TEXT_RECORD*)GetRecord(ii);

         printf("\t%s",pRecord->_pszName);

         if (*pRecord->_pszVersion)
         {
            printf("  [v. %s]\n",pRecord->_pszVersion);
         }
         else
         {
            printf("\n");
         }
      }
   }
}

/* ******************************************************************** **
** @@ main()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

int main(int argc,char** argv)
{
   // initialize MFC and print and error on failure
   if (!AfxWinInit(::GetModuleHandle(NULL),NULL,::GetCommandLine(),0))
   {
      return -1;
   }

   #ifdef _DEBUG
   if (!Autotest())
   {
      return 0;
   }
   #endif

   if ((argc < 2) || (argc > 4))
   {
      ShowHelp();
      return 0;
   }

   if (argc == 2 && ((!strcmp(argv[1],"?")) || (!strcmp(argv[1],"/?")) || (!strcmp(argv[1],"-?")) || (!stricmp(argv[1],"/h")) || (!stricmp(argv[1],"-h"))))
   {
      ShowHelp();
      return 0;
   }

   memset(_pszSignature,0,sizeof(_pszSignature));

   char     pszMask[MAX_PATH + 1];

   memset(pszMask,0,sizeof(pszMask));

   int   iParamCnt = 0;

   for (int ii = 1; ii < (argc - 1); ++ii)
   {
      if (!stricmp(argv[ii],"-i"))
      {
         // Info only, don't unpack
         _bInfoOnly = true;
         ++iParamCnt;
      }

      if (!stricmp(argv[ii],"-k"))
      {
         // Info only, don't unpack
         _bKeepOverlay = true;
         ++iParamCnt;
      }

      if (!stricmp(argv[ii],"-o"))
      {
         if (_bIgnoreValidation)
         {
            printf("Params conflict: -o incompatible with -p. Ignored.\n");
            continue;
         }
         else
         {
            // Just overlay, is not EXE
            _bJustOverlay = true;
         }

         ++iParamCnt;
      }
      else if (!stricmp(argv[ii],"-p"))
      {
         if (_bJustOverlay)
         {
            printf("Params conflict: -p incompatible with -o. Ignored.\n");
            continue;
         }
         else
         {
            // Ignore PE Validation check
            _bIgnoreValidation = true;
         }

         ++iParamCnt;
      }
   }

   strncpy(pszMask,argv[1 + iParamCnt],MAX_PATH);
   pszMask[MAX_PATH] = 0; // ASCIIZ

   _pACS = new AC_Search;

   if (!_pACS)
   {
      // Error !
      printf("Err: Can't create Search Dictionary.\n");
      delete _pACS;
      _pACS = NULL;
      return 0;
   }

   // Populate the Dix
   for (int jj = 0; jj < E_TEXT_DB_SIZE; ++jj)
   {
      SSF_TEXT_RECORD*     pRecord = &pTextDB[jj];

      if (!pRecord)
      {
         // Error !
         printf("Err: Error reading Internal Signatures DB.\n");
         delete _pACS;
         _pACS = NULL;
         return 0;
      }

      BYTE     pSignature[(MAX_SIGN_SIZE + 1) >> 1];  // Half of Text representation
      int      iSignSize = 0;

      memset(pSignature,0,sizeof(pSignature));

      int      iLen = strlen(pRecord->_pszSignature);

      if (iLen < 2)
      {
         // Too short to be valid!
         continue;
      }

      // Convert Text-2-Bin
      for (int ii = 0; ii < iLen; ii += 2)
      {
         int   Lo = toupper(pRecord->_pszSignature[ii + 1]);
         int   Hi = toupper(pRecord->_pszSignature[ii]);

         int   iSignLo = 0;

         if ((Lo >= '0') && (Lo <= '9'))
         {
            iSignLo = Lo - '0';
         }
         else if ((Lo >= 'A') && (Lo <= 'F'))
         {
            iSignLo = Lo - 'A' + 0x0A;
         }
         else
         {
            // Error !
            printf("Err: Invalid signature character in position %d in [%s]\n",ii + 1,pRecord->_pszSignature);
            continue;
         }

         int   iSignHi = 0;

         if ((Hi >= '0') && (Hi <= '9'))
         {
            iSignHi = Hi - '0';
         }
         else if ((Hi >= 'A') && (Hi <= 'F'))
         {
            iSignHi = Hi - 'A' + 0x0A;
         }
         else
         {
            // Error !
            printf("Err: Invalid signature character in position %d in [%s]\n",ii,pRecord->_pszSignature);
            continue;
         }

         pSignature[iSignSize++] = (BYTE)(iSignLo | (iSignHi << 4));
      }
      
      if (!_pACS->AddBytes(pSignature,(WORD)iSignSize,pRecord))
      {
         #ifdef _DEBUG
         printf("Err: Can't add [ %s / %s::%s ] signature. Possible duplicate.\n\n",pRecord->_pszName,pRecord->_pszVersion,pRecord->_pszSignature);
         #endif

         continue;
      }
   }

   // Finalize
   if (!_pACS->Finalize())
   {
      // Error !
      delete _pACS;
      _pACS = NULL;
      return 0;
   }

   Walker      Visitor;
   
   Visitor.Init(ForEach,pszMask,false);

   Visitor.Run(".");

   delete _pACS;
   _pACS = NULL;

   return 0;
}

/* ******************************************************************** **
** @@                   End of File
** ******************************************************************** */
