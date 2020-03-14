/******************************************************************************/
/*                                                                            */
/* MMDMVCommon.fx                                                             */
/* MMDMVEffect���ʒ�`/�G�t�F�N�g�ݒ�t�@�C��                                 */
/* �쐬: �������J����                                                         */
/*                                                                            */
/******************************************************************************/

#ifndef _MMDMV_COMMON
#define _MMDMV_COMMON

// �����I�{�̑������Ńn�[�h�R�[�e�B���O���Ă��邽�ߕύX�֎~�I����
// [�J���҃���] MMEHack_Effect.h �̒�`�ƍ��킹�邱��
#define MMEHACK_EFFECT_RENDERTARGET	"MMDOVRCamera.fx"

#define MMEHACK_EFFECT_OVRRENDERL	"MMDOVRRenderL.fx"
#define MMEHACK_EFFECT_OVRRENDERR	"MMDOVRRenderR.fx"

#define MMEHACK_EFFECT_TEXTURENAME		RTMirrorView
#define MMEHACK_EFFECT_EYETEXTURENAMEL	RTEyeViewL
#define MMEHACK_EFFECT_EYETEXTURENAMER	RTEyeViewR

// �����I�{�̑������Ńn�[�h�R�[�e�B���O���Ă��邽�ߕύX�֎~�I����

// ���w�萔�E�֐���`
#define M_PI			(3.141592654)
#define DEG2RAD(deg)	(M_PI * asfloat(deg) / 180.0)


/* ������Ȃ��l�͂������火�ȊO�G��ȁI */

// �J�����I�u�W�F�N�g�̖��O pmd/pmx/x �ǂ�ł�OK
#define OBJECT_CAMERA			"MMDMVCamera.x"
#define OBJECT_CAMERA_ITEM		""

// �œ_�I�u�W�F�N�g�̖��O pmd/pmx/x �ǂ�ł�OK
#define OBJECT_FOCUS			"FocusPosition.x"

// �œ_�I�u�W�F�N�g�����݂��Ȃ��ꍇ�̃f�t�H���g�l
#define FOCUS_DEFAULT_POS	float3(0, 10, 0)

// �A���`�G�C���A�X(AA) ON:true OFF:false
// ON�ɂ���ƕ`���Y��BOFF�ɂ���ƌy���Ȃ�B
#define MIRROR_RT_ENABLE_AA		(true)


/* ������Ȃ��l�͂������灪�ȊO�G��ȁI */

#endif // _MMDMV_COMMON
