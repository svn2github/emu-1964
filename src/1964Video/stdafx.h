// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(_STDAFX_H_)
#define _STDAFX_H_

#define _CRT_SECURE_NO_DEPRECATE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define EXPORT				__declspec(dllexport)
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0400
#include <stdio.h>
#define WINVER 0x0400
#include <windows.h>
#include <windowsx.h>			// Button_* etc
#include <shlwapi.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <ShellAPI.h>
#include <winnt.h>			// For 32x32To64 multiplies
#endif
#include <math.h>			// For sqrt()
#include <iostream>
#include <fstream>
#include <istream>

#if _MSC_VER > 1200
#include "xmmintrin.h"
#endif


#include <process.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <d3d9types.h>
#include <D3dx9math.h>
#include <dxerr.h>
#define MYLPDIRECT3DTEXTURE			LPDIRECT3DTEXTURE9
#define MYD3DADAPTER_IDENTIFIER		D3DADAPTER_IDENTIFIER9
#define MYIDirect3DSurface			IDirect3DSurface9
#define MYLPDIRECT3DBASETEXTURE		LPDIRECT3DBASETEXTURE9
#define MYLPDIRECT3DSURFACE			LPDIRECT3DSURFACE9
#define MYD3DVIEWPORT				D3DVIEWPORT9
#define MYD3DCAPS					D3DCAPS9
#define MYLPDIRECT3DDEVICE			LPDIRECT3DDEVICE9
#define MYLPDIRECT3D				LPDIRECT3D9
#define MYIDirect3DBaseTexture		IDirect3DBaseTexture9
#define D3DRS_ZBIAS					D3DRS_DEPTHBIAS
#define SetVertexShaderConstant		SetVertexShaderConstantF
#include <vector>

#include "gl/gl.h"
#include "gl/glu.h"
#include "glext.h"
#include "glh_extension_ati.h"
#include "wglext.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }// Microdev check me.
#endif


#ifndef SAFE_CHECK
#define SAFE_CHECK(a)	if( (a) == NULL ) {ErrorMsg("Creater out of memory"); throw new std::exception();}
#endif

#include "typedefs.h"
#include "gfx.h"
#include "Video.h"
#include "Config.h"
#include "resource.h"
#include "./Debugger/Debugger.h"
#include "./Parser/RSP_S2DEX.h"
#include "./Parser/RSP_Parser.h"

#include "./Texture/TextureManager.h"
#include "./Texture/ConvertImage.h"
#include "./Texture/Texture.h"
#include "./Texture/DirectXTexture/DirectXTexture.h"

#include "./Combiner/CombinerDefs.h"
#include "./Combiner/DecodedMux.h"

#include "./Combiner/blender.h"
#include "./Combiner/DirectXCombiner/DirectXBlender.h"
#include "./Combiner/OpenGLCombiner/OGLBlender.h"

#include "./Combiner/combiner.h"
#include "./Combiner/GeneralCombiner.h"
#include "./Combiner/DirectXCombiner/DirectXCombiner.h"

#include "./Device/RenderTexture.h"
#include "./Device/FrameBuffer.h"

#include "./Device/GraphicsContext.h"
#include "./Device/DirectXDevice/DXGraphicsContext.h"
#include "./Device/DeviceBuilder.h"

#include "./Render/RenderBase.h"
#include "./Render/ExtendedRender.h"
#include "./Render/Render.h"
#include "./Render/DirectX/D3DRender.h"

#include "./Texture/OGLTexture/OGLTexture.h"

#include "./Combiner/OpenGLCombiner/OGLCombiner.h"

#include "./Debugger/OGLDebug.h"
#include "./Render/OpenGL/OGLRender.h"
#include "./Render/OpenGL/OGLExtRender.h"
#include "./Device/OGLDevice/OGLGraphicsContext.h"

#include "resource.h"

#include "./Utility/icolor.h"
#include "./Utility/CritSect.h"
#include "./Parser/Timing.h"

extern WindowSettingStruct windowSetting;

void __cdecl MsgInfo (char * Message, ...);
void __cdecl ErrorMsg (char * Message, ...);

#define MI_INTR_DP          0x00000020  
#define MI_INTR_SP          0x00000001  

extern uint32 g_dwRamSize;

extern uint32 * g_pRDRAMu32;
extern signed char* g_pRDRAMs8;
extern unsigned char *g_pRDRAMu8;

extern GFX_INFO g_GraphicsInfo;

extern char *project_name;

