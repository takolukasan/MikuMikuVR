// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

// Windows ヘッダー ファイル:
#include <windows.h>

#define MIRROR_RENDER

#define D3D9EX_ENABLE

#ifdef D3D9EX_ENABLE
#define OVR_ENABLE
#endif

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
#include <new>
#include <vector>


#define STRSAFE_NO_CB_FUNCTIONS
#include <strsafe.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9math.inl>

#ifdef OVR_ENABLE
#include <dxgi.h>
#include <d3d11.h>
#include <d3dx11.h>
#endif

#include <xnamath.h>

#include "resource.h"

#include "MirrorWindow.h"
#include "HookFunctions.h"



#ifdef OVR_ENABLE

#include "LibOVR/Include/OVR.h"
#define OVR_D3D_VERSION 11
#include "LibOVR/Src/OVR_CAPI_D3D.h"
#undef OVR_D3D_VERSION

#include "OVRManager.h"

#endif


#include "d3d9_IDirect3D9.h"
#include "d3d9_IDirect3DDevice9.h"
#include "d3d9_IDirect3DVertexBuffer9.h"
#include "d3dx9_ID3DXEffect.h"

#include "d3d9_hook.h"
#include "MMEHack.h"
#include "MMEHack_Effect.h"

#include "Direct3D9_VtblUtil.h"
