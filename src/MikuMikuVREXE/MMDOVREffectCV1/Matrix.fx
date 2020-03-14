/******************************************************************************/
/*                                                                            */
/* Matrix.fx                                                                  */
/* �ėp�s�񐶐����Z for MMDMVEffect                                           */
/* �쐬: �������J����                                                         */
/*                                                                            */
/******************************************************************************/

#ifndef _MMDMV_MATRIX
#define _MMDMV_MATRIX

// XNA Math Library �� xnamathmatrix.inl �Ƃ����Q�l��HLSL�����Ɏ���
// �����Ƃ��͊��ƊۃR�s�[��������ꕔ���������Ă݂���B
//
//
// MatrixLookAtLH
// �J�����̈ʒu(Eye)�������(Up)���_����œ_(At)�������Ă��鎞��
// ������W�n�̃r���[�s����쐬���܂��B
// 
// �p�����[�^�[
// EyePosition �J�����̈ʒu
// FocusPosition �œ_�̈ʒu
// UpDirection �J�����̏���� (�ʏ�� <0.0f, 1.0f, 0.0f>)
//
// �߂�l
// 
// ���_�����[���h��Ԃ���r���[��Ԃɕϊ�����r���[�s���Ԃ��܂��B
// 
// D3DXMatrixLookAtLH()��XMMatrixLookAtLH()���Q�l�ɂ����B
//
//  zaxis = normal(At - Eye)
//  xaxis = normal(cross(Up, zaxis))
//  yaxis = cross(zaxis, xaxis)
//
//	 xaxis.x           yaxis.x           zaxis.x          0
//	 xaxis.y           yaxis.y           zaxis.y          0
//	 xaxis.z           yaxis.z           zaxis.z          0
//	-dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye)  1
//
// �{�Ƃ�MatrixLookToLH���R�[�����邪�A���g���ǂ��������̂ƁA�p�t�H�[�}���X���l�����ēW�J�B

inline float4x4 MatrixLookAtLH
(
	float3 EyePosition,		// Eye
	float3 FocusPosition,	// At
	float3 UpDirection		// Up
)
{
	float3 EyeDirection;
	float4x4 M;

	float3 R0, R1, R2;
	float3 R3;

	EyeDirection = FocusPosition - EyePosition;

	R2 = normalize(EyeDirection);
	R0 = normalize(cross(UpDirection, R2));
	R1 = cross(R2, R0);

	R3.x = -dot(R0, EyePosition);
	R3.y = -dot(R1, EyePosition);
	R3.z = -dot(R2, EyePosition);

	M = float4x4 (
		R0.x, R1.x, R2.x, 0,
		R0.y, R1.y, R2.y, 0,
		R0.z, R1.z, R2.z, 0,
		R3.x, R3.y, R3.z, 1
	);

	return M;
}

// MatrixTranslation
// �w�肳�ꂽ�I�t�Z�b�g���畽�s�ړ��s����쐬���܂��B
// 
// �p�����[�^�[
// 
// OffsetX x ���ɉ��������s�ړ�
// OffsetY y ���ɉ��������s�ړ�
// OffsetZ z ���ɉ��������s�ړ�
//
// �߂�l
// 
// ���s�ړ��s���Ԃ��܂��B

inline float4x4 MatrixTranslation
(
    FLOAT OffsetX, 
    FLOAT OffsetY, 
    FLOAT OffsetZ
)
{
	float4x4 M;

	M = float4x4 (
		1.0,		0.0,		0.0,		0.0,
		0.0,		1.0,		0.0,		0.0,
		0.0,		0.0,		1.0,		0.0,
		OffsetX,	OffsetY,	OffsetZ,	1.0
	);

	return M;
}

// MatrixRotationX
// x ������]���Ƃ��ĉ�]�����s����쐬���܂��B
//
// �p�����[�^�[
//
// Angle x ������]���Ƃ����]�̊p�x (���W�A���P��)�B�p�x�́A��]������Ƃ��A���_���玞�v���̕����Ŏw�肵�܂��B 
// 
// �߂�l
//
// ��]�s���Ԃ��܂��B

inline float4x4 MatrixRotationX
(
    FLOAT Angle
)
{
	float4x4 M;

    FLOAT fSinAngle = sin(Angle);
    FLOAT fCosAngle = cos(Angle);

	M = float4x4 (
		1.0,		0.0,		0.0,		0.0,
		0.0,		fCosAngle,	fSinAngle,	0.0,
		0.0,		-fSinAngle,	fCosAngle,	0.0,
		0.0,		0.0,		0.0,		1.0
	);

	return M;
}

// MatrixRotationY
// y ������]���Ƃ��ĉ�]�����s����쐬���܂��B
//
// �p�����[�^�[
//
// Angle y ������]���Ƃ����]�̊p�x (���W�A���P��)�B�p�x�́A��]������Ƃ��A���_���玞�v���̕����Ŏw�肵�܂��B 
// 
// �߂�l
//
// ��]�s���Ԃ��܂��B

inline float4x4 MatrixRotationY
(
    FLOAT Angle
)
{
	float4x4 M;

    FLOAT fSinAngle = sin(Angle);
    FLOAT fCosAngle = cos(Angle);

	M = float4x4 (
		fCosAngle,	0.0,		-fSinAngle,	0.0,
		0.0,		1.0,		0.0,		0.0,
		fSinAngle,	0.0,		fCosAngle,	0.0,
		0.0,		0.0,		0.0,		1.0
	);

	return M;
}

// MatrixRotationZ
// z ������]���Ƃ��ĉ�]�����s����쐬���܂��B
//
// �p�����[�^�[
//
// Angle z ������]���Ƃ����]�̊p�x (���W�A���P��)�B�p�x�́A��]������Ƃ��A���_���玞�v���̕����Ŏw�肵�܂��B 
// 
// �߂�l
//
// ��]�s���Ԃ��܂��B

inline float4x4 MatrixRotationZ
(
    FLOAT Angle
)
{
	float4x4 M;

    FLOAT fSinAngle = sin(Angle);
    FLOAT fCosAngle = cos(Angle);

	M = float4x4 (
		fCosAngle,	fSinAngle,	0.0,		0.0,
		-fSinAngle,	fCosAngle,	0.0,		0.0,
		0.0,		0.0,		1.0,		0.0,
		0.0,		0.0,		0.0,		1.0
	);

	return M;
}

#endif // _MMDMV_MATRIX
