/******************************************************************************/
/*                                                                            */
/* Matrix.fx                                                                  */
/* 汎用行列生成演算 for MMDMVEffect                                           */
/* 作成: たこルカさん                                                         */
/*                                                                            */
/******************************************************************************/

#ifndef _MMDMV_MATRIX
#define _MMDMV_MATRIX

// XNA Math Library の xnamathmatrix.inl とかを参考にHLSL向けに実装
// 説明とかは割と丸コピーだったり一部書き直してみたり。
//
//
// MatrixLookAtLH
// カメラの位置(Eye)が上方向(Up)視点から焦点(At)を向いている時の
// 左手座標系のビュー行列を作成します。
// 
// パラメーター
// EyePosition カメラの位置
// FocusPosition 焦点の位置
// UpDirection カメラの上方向 (通常は <0.0f, 1.0f, 0.0f>)
//
// 戻り値
// 
// 頂点をワールド空間からビュー空間に変換するビュー行列を返します。
// 
// D3DXMatrixLookAtLH()とXMMatrixLookAtLH()を参考にした。
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
// 本家はMatrixLookToLHをコールするが、中身が良く分からんのと、パフォーマンスを考慮して展開。

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
// 指定されたオフセットから平行移動行列を作成します。
// 
// パラメーター
// 
// OffsetX x 軸に沿った平行移動
// OffsetY y 軸に沿った平行移動
// OffsetZ z 軸に沿った平行移動
//
// 戻り値
// 
// 平行移動行列を返します。

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
// x 軸を回転軸として回転した行列を作成します。
//
// パラメーター
//
// Angle x 軸を回転軸とする回転の角度 (ラジアン単位)。角度は、回転軸を基準とし、原点から時計回りの方向で指定します。 
// 
// 戻り値
//
// 回転行列を返します。

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
// y 軸を回転軸として回転した行列を作成します。
//
// パラメーター
//
// Angle y 軸を回転軸とする回転の角度 (ラジアン単位)。角度は、回転軸を基準とし、原点から時計回りの方向で指定します。 
// 
// 戻り値
//
// 回転行列を返します。

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
// z 軸を回転軸として回転した行列を作成します。
//
// パラメーター
//
// Angle z 軸を回転軸とする回転の角度 (ラジアン単位)。角度は、回転軸を基準とし、原点から時計回りの方向で指定します。 
// 
// 戻り値
//
// 回転行列を返します。

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
