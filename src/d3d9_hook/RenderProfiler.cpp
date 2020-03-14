#include "stdafx.h"


RenderProfiler *g_pProfiler;


RenderProfiler::RenderProfiler(ID3D11Device *pD3D11, ID3D11DeviceContext *pD3D11Context)
{
#ifdef RENDERPROFILER_ENABLE

	if( QueryPerformanceFrequency(&this->Freq) ) {
		this->bUsePerformanceCounter = TRUE;
	}
	else {
		/* ‚µ‚Á‚Ï‚¢ */
		this->bUsePerformanceCounter = FALSE;
		this->Freq.QuadPart = 1000;	/* 1000 count / 1sec @ timeGetTime() */
	}

	this->nCPCount = 0;

	this->BeginCount.QuadPart = 0;
	this->EndCount.QuadPart = 0;
	this->FrameCount.QuadPart = 0;

	memset(this->CPBeginCount, 0, sizeof(this->CPBeginCount));
	memset(this->CPLeaveCount, 0, sizeof(this->CPLeaveCount));
	memset(this->CPMidCount, 0, sizeof(this->CPMidCount));
	memset(this->tcCPLabel, 0, sizeof(this->tcCPLabel));

	this->FrameTime = 0.0;
	memset(this->CPTime, 0, sizeof(this->CPTime));

#endif
}

RenderProfiler::~RenderProfiler()
{
#ifdef RENDERPROFILER_ENABLE

#endif
}

void RenderProfiler::BeginFrame()
{
#ifdef RENDERPROFILER_ENABLE

	if( this->bUsePerformanceCounter ) {
		QueryPerformanceFrequency(&this->Freq);
		QueryPerformanceCounter(&this->BeginCount);
	}
	else {
		this->Freq.QuadPart = 1000;	/* 1000 count / 1sec @ timeGetTime() */
		this->BeginCount.QuadPart = (LONGLONG)timeGetTime();
	}

	memset(this->CPBeginCount, 0, sizeof(this->CPBeginCount));
	memset(this->CPLeaveCount, 0, sizeof(this->CPLeaveCount));
	memset(this->CPMidCount, 0, sizeof(this->CPMidCount));
	memset(this->tcCPLabel, 0, sizeof(this->tcCPLabel));
	memset(this->CPTime, 0, sizeof(this->CPTime));

	this->nCPCount = 0;

#endif
}

void RenderProfiler::EnterCheckPoint(LPCTSTR lpCheckPointName)
{
#ifdef RENDERPROFILER_ENABLE

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	this->CPBeginCount[this->nCPCount] = counter;

	if( lpCheckPointName ) {
		StringCchCopy(this->tcCPLabel[this->nCPCount], CHECKPOINT_LABEL_LEN, lpCheckPointName);
	}
	else {
		StringCchCopy(this->tcCPLabel[this->nCPCount], CHECKPOINT_LABEL_LEN, TEXT("\0"));
	}
	this->tcCPLabel[this->nCPCount][CHECKPOINT_LABEL_LEN - 1] = 0x0000;

#endif
}

void RenderProfiler::LeaveCheckPoint()
{
#ifdef RENDERPROFILER_ENABLE

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	this->CPLeaveCount[this->nCPCount] = counter;

	if( this->nCPCount < CHECKPOINT_MAX )
		this->nCPCount++;

#endif
}

void RenderProfiler::EndFrame()
{
#ifdef RENDERPROFILER_ENABLE

	if( this->bUsePerformanceCounter ) {
		QueryPerformanceCounter(&this->EndCount);
	}
	else {
		this->EndCount.QuadPart = (LONGLONG)timeGetTime();
	}

	this->FrameCount.QuadPart = this->EndCount.QuadPart - this->BeginCount.QuadPart;
	this->FrameTime = (double)this->FrameCount.QuadPart / (double)this->Freq.QuadPart;

	int i;

	for( i = 0; i < this->nCPCount; i++ ) {
		this->CPMidCount[i].QuadPart = this->CPLeaveCount[i].QuadPart - this->CPBeginCount[i].QuadPart;
		this->CPTime[i] = (double)this->CPMidCount[i].QuadPart / (double)this->Freq.QuadPart;
	}

	this->nCPCount = 0;

#endif
}

