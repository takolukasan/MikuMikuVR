/******************************************************************************/
/*                                                                            */
/* MMDMVCommon.fx                                                             */
/* MMDMVEffect共通定義/エフェクト設定ファイル                                 */
/* 作成: たこルカさん                                                         */
/*                                                                            */
/******************************************************************************/

#ifndef _MMDMV_COMMON
#define _MMDMV_COMMON

// ↓↓！本体側内部でハードコーティングしているため変更禁止！↓↓
// [開発者メモ] MMEHack_Effect.h の定義と合わせること
#define MMEHACK_EFFECT_RENDERTARGET	"MMDOVRCamera.fx"

#define MMEHACK_EFFECT_OVRRENDERL	"MMDOVRRenderL.fx"
#define MMEHACK_EFFECT_OVRRENDERR	"MMDOVRRenderR.fx"

#define MMEHACK_EFFECT_TEXTURENAME		RTMirrorView
#define MMEHACK_EFFECT_EYETEXTURENAMEL	RTEyeViewL
#define MMEHACK_EFFECT_EYETEXTURENAMER	RTEyeViewR

// ↑↑！本体側内部でハードコーティングしているため変更禁止！↑↑

// 数学定数・関数定義
#define M_PI			(3.141592654)
#define DEG2RAD(deg)	(M_PI * asfloat(deg) / 180.0)


/* 分からない人はここから↓以外触るな！ */

// カメラオブジェクトの名前 pmd/pmx/x どれでもOK
#define OBJECT_CAMERA			"MMDMVCamera.x"
#define OBJECT_CAMERA_ITEM		""

// 焦点オブジェクトの名前 pmd/pmx/x どれでもOK
#define OBJECT_FOCUS			"FocusPosition.x"

// 焦点オブジェクトが存在しない場合のデフォルト値
#define FOCUS_DEFAULT_POS	float3(0, 10, 0)

// アンチエイリアス(AA) ON:true OFF:false
// ONにすると描画綺麗。OFFにすると軽くなる。
#define MIRROR_RT_ENABLE_AA		(true)


/* 分からない人はここから↑以外触るな！ */

#endif // _MMDMV_COMMON
