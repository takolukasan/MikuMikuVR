#ifndef _RENDERPROFILER_H
#define _RENDERPROFILER_H


// #define RENDERPROFILER_ENABLE


#define CHECKPOINT_MAX			(16)
#define CHECKPOINT_LABEL_LEN	(256)

class RenderProfiler {
public:
	RenderProfiler(ID3D11Device *pD3D11, ID3D11DeviceContext *pD3D11Context);
	~RenderProfiler();

	void BeginFrame();
	void EnterCheckPoint(LPCTSTR lpCheckPointName);
	void LeaveCheckPoint();
	void EndFrame();

	void DrawProfiledGraph();

private:
#ifdef RENDERPROFILER_ENABLE

	BOOL bUsePerformanceCounter;

	LARGE_INTEGER Freq;
	
	INT nCPCount;

	LARGE_INTEGER BeginCount;
	LARGE_INTEGER EndCount;

	LARGE_INTEGER CPBeginCount[CHECKPOINT_MAX];
	LARGE_INTEGER CPLeaveCount[CHECKPOINT_MAX];
	TCHAR tcCPLabel[CHECKPOINT_MAX][CHECKPOINT_LABEL_LEN];

	LARGE_INTEGER FrameCount;
	LARGE_INTEGER CPMidCount[CHECKPOINT_MAX];

	double FrameTime;
	double CPTime[CHECKPOINT_MAX];

#endif
};


extern RenderProfiler *g_pProfiler;


#endif // _RENDERPROFILER_H
