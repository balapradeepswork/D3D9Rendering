#ifndef _FRAME_H_
#define _FRAME_H_

#include <d3d9.h>
#include <iostream>
#include <fstream>
#include <atlbase.h>

enum SOURCETYPE
{
	CAPTURE,
	RGB32,
	RGB24,
	YUV420,
	NV12
};

class Frame
{
private:
	//vars
	UINT pitch;
	BYTE* B;
	BYTE* G;
	BYTE* R;
	BYTE* Y;
	BYTE* U;
	BYTE* V;
	BYTE* UV;

	//methods
	HRESULT FillYUV420ToSurface(IDirect3DSurface9*	g_pSurface);
	HRESULT FillNV12ToSurface(IDirect3DSurface9*	g_pSurface);
	HRESULT Fill32bppImageToSurface(IDirect3DSurface9*	g_pSurface);
	HRESULT Fill24bppImageToSurface(IDirect3DSurface9*	g_pSurface);
	HRESULT ReadRGBDataFromFile(char *file_path);
	HRESULT ReadYUV420DataFromFile(char *file_path);
	HRESULT ReadNV12DataFromFile(char *file_path);
	void ReadRGBData(BYTE* B);
	void ReadYUV420Data(BYTE* Y, BYTE* U, BYTE* V);
	void ReadNV12Data(BYTE* Y, BYTE* UV);
	bool AllocBuffers();

public:
	//vars
	UINT width;
	UINT height;
	SOURCETYPE type;

	//methods
	Frame();
	Frame(SOURCETYPE type);
	~Frame();
	HRESULT FillSurface(IDirect3DSurface9*	g_pSurface);
	HRESULT ReadDataFromFile(char *file_path);
	void ReadData(UINT width, UINT height, UINT pitch, BYTE** ppData);
	void CleanUp();
};

#endif