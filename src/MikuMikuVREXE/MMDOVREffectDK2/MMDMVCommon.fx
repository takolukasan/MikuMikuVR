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

#define MMEHACK_EFFECT_EYETEXTURENAMEL	RTEyeViewL
#define MMEHACK_EFFECT_EYETEXTURENAMER	RTEyeViewR

// ↑↑！本体側内部でハードコーティングしているため変更禁止！↑↑

// 数学定数・関数定義
#define M_PI			(3.141592654)
#define DEG2RAD(deg)	(M_PI * asfloat(deg) / 180.0)


// 共通定数
#define MMEHACK_VIEWTYPE_DEFAULT	(0)		// ビュー行列・プロジェクション行列をを書き換えない
#define MMEHACK_VIEWTYPE_VIEWPROJ	(1)		// ビュー行列・プロジェクションを書き換える
#define MMEHACK_VIEWTYPE_PROJONLY	(2)		// プロジェクション行列のみを書き換える(ビュー行列はMMDのカメラを使う)

#define MMEHACK_VIEWEYE_LEFT		(0)
#define MMEHACK_VIEWEYE_RIGHT		(1)

// HMDのバージョン識別用
// OVR_CAPI.h にて定義されている ovrHmdType を参照のこと
#define MMEHACK_OVRHMDTYPE_DK2		(6)		// ovrHmd_DK2       = 6
#define MMEHACK_OVRHMDTYPE_CV1		(14)	// ovrHmd_CV1       = 14

#define MMEHACK_PROJ_ZNEAR_DEFAULT	(0.01)
#define MMEHACK_PROJ_ZFAR_DEFAULT	(100000.0)


/* 分からない人はここから↓以外触るな！ */

// カメラオブジェクトの名前 pmd/pmx/x どれでもOK
#define OBJECT_CAMERA			"MMDOVRCamera.x"
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
