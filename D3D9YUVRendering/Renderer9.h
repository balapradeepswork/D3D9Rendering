#ifndef _RENDERER9_H_
#define _RENDERER9_H_

#include <d3d9.h>
#include <iostream>
#include "Frame.h"

class Renderer9
{
private:
	//vars
	IDirect3D9*				g_pD3D;
	IDirect3DDevice9*		g_pd3dDevice;
	HWND					ghWnd;
	RECT					gviewPort;
	UINT					gPitch;
	D3DPRESENT_PARAMETERS	d3dpp;
	D3DFORMAT				sourceFormat;

public:
	//vars
	IDirect3DSurface9 * g_pSurface;
	IDirect3DSurface9 * g_pCaptureSurface;

	//methods
	Renderer9();
	Renderer9(SOURCETYPE sourceType);
	~Renderer9();
	HRESULT	InitD3D(UINT width, UINT height, HWND hWnd = NULL);
	HRESULT GrabImage();
	UINT GetPitch();
	UINT GetHeight();
	UINT GetWidth();
	HRESULT Render();
	HRESULT Reset();
	void Cleanup();
};

#endif // !_RENDERER9_H_