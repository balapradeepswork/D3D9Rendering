#include "stdafx.h"
#include "Renderer9.h"


Renderer9::Renderer9()
{
	g_pD3D = NULL;
	g_pd3dDevice = NULL;
	g_pSurface = NULL;
	ghWnd = NULL;
	gviewPort = { 0,0,0,0 };
	gPitch = 0;
	d3dpp = { 0 };
	sourceFormat = D3DFMT_X8R8G8B8;
}

Renderer9::Renderer9(SOURCETYPE sourceType)
{
	switch (sourceType)
	{
	case CAPTURE:
		this->sourceFormat = D3DFMT_A8R8G8B8;
		break;
	case RGB24:
		/*this->sourceFormat = D3DFMT_R8G8B8;
		break;*/
	case RGB32:
		this->sourceFormat = D3DFMT_X8R8G8B8;
		break;
	case YUV420:
		this->sourceFormat = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2');
		break;
	case NV12:
		this->sourceFormat = (D3DFORMAT)MAKEFOURCC('N', 'V', '1', '2');
		break;
	}
}


Renderer9::~Renderer9()
{
}

HRESULT	Renderer9::InitD3D(UINT width, UINT height, HWND hWnd)
{
	D3DDISPLAYMODE	ddm;

	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
	{
		std::cout << "Unable to Create Direct3D\n";
		return E_FAIL;
	}

	if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm)))
	{
		std::cout << "Unable to Get Adapter Display Mode\n";
		return E_FAIL;
	}

	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	d3dpp.Windowed = true;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.BackBufferFormat = ddm.Format;
	d3dpp.BackBufferHeight = (sourceFormat == D3DFMT_A8R8G8B8) ? ddm.Height : height;
	d3dpp.BackBufferWidth = (sourceFormat == D3DFMT_A8R8G8B8) ? ddm.Width : width;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
	{
		std::cout << "Unable to Create Device\n";
		return E_FAIL;
	}

	ghWnd = hWnd;
	gviewPort = { 0, 0, (LONG)d3dpp.BackBufferWidth, (LONG)d3dpp.BackBufferHeight };

	if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, sourceFormat, D3DPOOL_DEFAULT, &g_pSurface, NULL)))
	{
		std::cout << "Unable to Create Surface\n";
		return E_FAIL;
	}

	if(sourceFormat == D3DFMT_A8R8G8B8)
		if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, sourceFormat, D3DPOOL_SYSTEMMEM, &g_pCaptureSurface, NULL)))
		{
			std::cout << "Unable to Create Surface\n";
			return E_FAIL;
		}

	D3DLOCKED_RECT	lockedRect;

	if (FAILED(g_pSurface->LockRect(&lockedRect, NULL, 0)))					// compute the required buffer size
	{
		std::cout << "Unable to Lock Surface\n";
		return E_FAIL;
	}
	gPitch = lockedRect.Pitch;
	if (FAILED(g_pSurface->UnlockRect()))
	{
		std::cout << "Unable to Unlock Surface\n";
		return E_FAIL;
	}

	return S_OK;
}

HRESULT Renderer9::GrabImage()
{
	if (FAILED(g_pd3dDevice->GetFrontBufferData(0, g_pCaptureSurface)))
	{
		printf_s("Unable to get Buffer Surface Data\n");
		return E_FAIL;
	}

	if (FAILED(g_pd3dDevice->UpdateSurface(g_pCaptureSurface, NULL, g_pSurface, NULL)))
	{
		printf_s("Unable to update Surface \n");
		return E_FAIL;
	}

	return S_OK;
}

UINT Renderer9::GetPitch()
{
	return gPitch;
}

UINT Renderer9::GetHeight()
{
	return gviewPort.bottom;
}

UINT Renderer9::GetWidth()
{
	return gviewPort.right;
}

HRESULT Renderer9::Render()
{
	if (sourceFormat == D3DFMT_A8R8G8B8 && FAILED(GrabImage()))
		return E_FAIL;
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	IDirect3DSurface9 * pBackBuffer = NULL;
	if (FAILED(g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
	{
		std::cout << "Unable to get BackBuffer.\n";
		return E_FAIL;
	}

	if (FAILED(g_pd3dDevice->StretchRect(g_pSurface, NULL, pBackBuffer, NULL, D3DTEXF_LINEAR)))
	{
		std::cout << "Unable to get BackBuffer.\n";
		return E_FAIL;
	}

	pBackBuffer->Release();

	HRESULT	hr;
	if (g_pd3dDevice)
	{
		hr = g_pd3dDevice->TestCooperativeLevel();//Check Device Status - if Alt+tab or some such thing have caused any trouble
		if (hr != D3D_OK)
		{
			if (hr == D3DERR_DEVICELOST)	return hr;	//Device is lost - Do not render now 
			if (hr == D3DERR_DEVICENOTRESET)		//Device is ready to be acquired
			{
				if (FAILED(Reset()))
				{
					DestroyWindow(ghWnd);		//If Unable to Reset Device - Close the Application
					return hr;
				}
			}
		}
		g_pd3dDevice->BeginScene();
		g_pd3dDevice->EndScene();
		g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
	}

	return S_OK;
}

HRESULT Renderer9::Reset()
{
	D3DDISPLAYMODE	ddm;

	if (g_pSurface)															//Release the Surface - we need to get the latest surface
	{
		g_pSurface->Release();
		g_pSurface = NULL;
	}

	if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm)))	//User might have changed the mode - Get it afresh
	{
		std::cout << "Unable to Get Adapter Display Mode";
		return E_FAIL;
	}

	HRESULT hr = g_pd3dDevice->Reset(&d3dpp);
	if (FAILED(hr))
	{
		std::cout << "Unable to Reset Device";
		return E_FAIL;
	}

	if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, sourceFormat, D3DPOOL_DEFAULT, &g_pSurface, NULL)))
	{
		std::cout << "Unable to Recreate Surface";
		return E_FAIL;
	}

	if (sourceFormat == D3DFMT_A8R8G8B8)
		if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, sourceFormat, D3DPOOL_SYSTEMMEM, &g_pCaptureSurface, NULL)))
		{
			std::cout << "Unable to Create Surface\n";
			return E_FAIL;
		}

	return S_OK;
}

void Renderer9::Cleanup()
{
	if (g_pSurface)
	{
		g_pSurface->Release();
		g_pSurface = NULL;
	}
	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = NULL;
	}
	if (g_pD3D)
	{
		g_pD3D->Release();
		g_pD3D = NULL;
	}
}