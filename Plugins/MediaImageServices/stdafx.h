//
// stdafx.h
//
// Source file for standard system include files,
// or project files that are used frequently but changed infrequently
//

#pragma once

#define STRICT
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE

#define _WIN32_DCOM
//#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT
#define _ATL_FREE_THREADED
#define _ATL_ALL_WARNINGS

//#pragma warning( push, 0 )

#include "Resource.h"
//#include <atlstr.h>
#include <atlbase.h>
#include <atlcom.h>
#include <dshow.h>
#include <amvideo.h>

// qedit.h workaround for missing dxtrans.h
//#pragma include_alias( "dxtrans.h", "qedit.h" )
//#define __IDxtCompositor_INTERFACE_DEFINED__
//#define __IDxtAlphaSetter_INTERFACE_DEFINED__
//#define __IDxtJpeg_INTERFACE_DEFINED__
//#define __IDxtKey_INTERFACE_DEFINED__
//#include <qedit.h>

#include "qedit.h"

//using namespace ATL;

//#pragma warning( pop )
