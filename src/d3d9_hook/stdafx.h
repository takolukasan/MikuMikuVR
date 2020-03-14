// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
//

#pragma once

#include "targetver.h"

// Windows �w�b�_�[ �t�@�C��:
#include <windows.h>


#define MIRROR_RENDER

#define D3D9EX_ENABLE

#ifdef D3D9EX_ENABLE
#define OVR_ENABLE
#endif


// #define DEBUG_CONSOLE
#ifdef DEBUG_CONSOLE
void PrintConsole(const char *szString, const int *pInt = NULL);
void PrintConsole(const char *szString, const char *szString2);
#else
#define PrintConsole(a,b)
#endif

// TODO: �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă��������B
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

#include "LibOVR/Include/OVR_CAPI_D3D.h"
#include <Kernel/OVR_Nullptr.h>

#include "OculusRift/OculusRift.h"

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

#include "RenderProfiler.h"
