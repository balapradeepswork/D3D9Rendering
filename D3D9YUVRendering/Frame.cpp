#include "stdafx.h"
#include "Frame.h"

Frame::Frame()
{
}

Frame::Frame(SOURCETYPE type)
{
	this->type = type;
}


Frame::~Frame()
{
}

HRESULT Frame::FillSurface(IDirect3DSurface9 * g_pSurface)
{
	switch (type)
	{
	case CAPTURE:
		return S_OK;
	case RGB32:
		return Fill32bppImageToSurface(g_pSurface);
	case RGB24:
		return Fill24bppImageToSurface(g_pSurface);
	case YUV420:
		return FillYUV420ToSurface(g_pSurface);
	case NV12:
		return FillNV12ToSurface(g_pSurface);
	}
}

HRESULT Frame::ReadDataFromFile(char * file_path)
{
	switch (type)
	{
	case CAPTURE:
		return S_OK;
	case RGB32:
	case RGB24:
		return ReadRGBDataFromFile(file_path);
	case YUV420:
		return ReadYUV420DataFromFile(file_path);
	case NV12:
		return ReadNV12DataFromFile(file_path);
	}
}

void Frame::ReadData(UINT width, UINT height, UINT pitch, BYTE ** ppData)
{
	this->width = width;
	this->height = height;
	this->pitch = pitch;

	switch (type)
	{
	case RGB32:
	case RGB24:
		return ReadRGBData(ppData[0]);
	case YUV420:
		return ReadYUV420Data(ppData[0], ppData[1], ppData[2]);
	case NV12:
		return ReadNV12Data(ppData[0], ppData[1]);
	}
}

void Frame::CleanUp()
{
	switch (type)
	{
	case RGB32:
	case RGB24:
		delete B;
		break;
	case YUV420:
	case NV12:
		delete Y;
		break;
	}
}

HRESULT Frame::FillYUV420ToSurface(IDirect3DSurface9 * g_pSurface)
{
	D3DLOCKED_RECT d3d_rect;
	if (!g_pSurface)
		return E_FAIL;
	if (FAILED(g_pSurface->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT)))
	{
		std::cout << "Unable to Lock BackBuffer.\n";
		return E_FAIL;
	}

	byte * pDest = (BYTE *)d3d_rect.pBits;
	int stride = d3d_rect.Pitch;

	for (int h = 0; h < height; h++)
	{
		if (memcpy_s(pDest + h * stride, stride, Y + h * pitch, pitch))
		{
			std::cout << "Unable to fill BackBuffer with the Y data.\n";
			return E_FAIL;
		}
	}

	for (int h = 0; h < height / 2; h++)
	{
		if (memcpy_s(pDest + stride * height + h * stride / 2, stride / 2, V + h * pitch / 2, pitch / 2))
		{
			std::cout << "Unable to fill BackBuffer with the V data.\n";
			return E_FAIL;
		}
	}

	for (int h = 0; h < height / 2; h++)
	{
		if (memcpy_s(pDest + stride * height + stride * height / 4 + h * stride / 2, stride / 2, U + h * pitch / 2, pitch / 2))
		{
			std::cout << "Unable to fill BackBuffer with the U data.\n";
			return E_FAIL;
		}
	}

	if (FAILED(g_pSurface->UnlockRect()))
	{
		std::cout << "Unable to Create Surface\n";
		return E_FAIL;
	}

	return S_OK;
}

HRESULT Frame::FillNV12ToSurface(IDirect3DSurface9 * g_pSurface)
{
	D3DLOCKED_RECT d3d_rect;
	if (!g_pSurface)
		return E_FAIL;
	if (FAILED(g_pSurface->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT)))
	{
		std::cout << "Unable to Lock BackBuffer.\n";
		return E_FAIL;
	}

	byte * pDest = (BYTE *)d3d_rect.pBits;
	int stride = d3d_rect.Pitch;

	for (int h = 0; h < height; h++)
	{
		if (memcpy_s(pDest + h * stride, stride, Y + h * pitch, pitch))
		{
			std::cout << "Unable to fill BackBuffer with the Y data.\n";
			return E_FAIL;
		}
	}

	for (int h = 0; h < height / 2; h++)
	{
		if (memcpy_s(pDest + stride * height + h * stride, stride, UV + h * pitch, pitch))
		{
			std::cout << "Unable to fill BackBuffer with the V data.\n";
			return E_FAIL;
		}
	}

	if (FAILED(g_pSurface->UnlockRect()))
	{
		std::cout << "Unable to Create Surface\n";
		return E_FAIL;
	}

	return S_OK;
}

HRESULT Frame::Fill32bppImageToSurface(IDirect3DSurface9 * g_pSurface)
{
	D3DLOCKED_RECT d3d_rect;
	if (!g_pSurface)
		return E_FAIL;
	if (FAILED(g_pSurface->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT)))
	{
		std::cout << "Unable to Lock BackBuffer.\n";
		return E_FAIL;
	}

	byte * pDest = (BYTE *)d3d_rect.pBits;
	int stride = d3d_rect.Pitch;

	for (int i = 0; i < height; i++)
	{
		if (memcpy_s(pDest + i * stride, pitch, B + (height - 1 - i) * pitch, width * 4))
		{
			std::cout << "Unable to fill BackBuffer with the image data.\n";
			return E_FAIL;
		}
	}

	if (FAILED(g_pSurface->UnlockRect()))
	{
		std::cout << "Unable to Create Surface\n";
		return E_FAIL;
	}

	return S_OK;
}

HRESULT Frame::Fill24bppImageToSurface(IDirect3DSurface9 * g_pSurface)
{
	D3DLOCKED_RECT d3d_rect;
	if (!g_pSurface)
		return E_FAIL;
	if (FAILED(g_pSurface->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT)))
	{
		std::cout << "Unable to Lock BackBuffer.\n";
		return E_FAIL;
	}

	byte * pDest = (BYTE *)d3d_rect.pBits;
	int stride = d3d_rect.Pitch;

	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			if (memcpy_s(pDest + h * stride + w * 4, 3, B + (height - 1 - h) * pitch + w * 3, 3))
			{
				std::cout << "Unable to fill BackBuffer with the image data.\n";
				return E_FAIL;
			}
		}

	}

	if (FAILED(g_pSurface->UnlockRect()))
	{
		std::cout << "Unable to Create Surface\n";
		return E_FAIL;
	}

	return S_OK;
}

HRESULT Frame::ReadRGBDataFromFile(char * file_path)
{
	std::ifstream rgb_file(file_path, std::ios::binary);
	if (rgb_file.is_open())
	{
		BITMAPFILEHEADER *bITMAPFILEHEADER = new BITMAPFILEHEADER();
		BITMAPINFOHEADER *bITMAPINFOHEADER = new BITMAPINFOHEADER();

		rgb_file.read(reinterpret_cast<char *>(bITMAPFILEHEADER), sizeof(BITMAPFILEHEADER));
		rgb_file.read(reinterpret_cast<char *>(bITMAPINFOHEADER), sizeof(BITMAPINFOHEADER));

		width = bITMAPINFOHEADER->biWidth;
		height = bITMAPINFOHEADER->biHeight;
		size_t bitmapSize = bITMAPFILEHEADER->bfSize - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER);
		pitch = bitmapSize / bITMAPINFOHEADER->biHeight;

		if (!AllocBuffers())
			return E_FAIL;

		rgb_file.read(reinterpret_cast<char *>(B), pitch*height);

		rgb_file.close();

		return S_OK;
	}
	else
	{
		std::cout << "Unable to open the nv12 file";
		return E_FAIL;
	}
}

HRESULT Frame::ReadYUV420DataFromFile(char * file_path)
{
	
	std::ifstream yuv420_file(file_path, std::ios::binary);
	if (yuv420_file.is_open())
	{
		yuv420_file.read(reinterpret_cast<char *>(&width), sizeof(width));
		yuv420_file.read(reinterpret_cast<char *>(&height), sizeof(height));
		yuv420_file.read(reinterpret_cast<char *>(&pitch), sizeof(pitch));

		if (!AllocBuffers())
			return E_FAIL;
		
		yuv420_file.read(reinterpret_cast<char *>(Y), pitch*height);
		yuv420_file.read(reinterpret_cast<char *>(U), pitch*height / 4);
		yuv420_file.read(reinterpret_cast<char *>(V), pitch*height / 4);
		
		yuv420_file.close();

		return S_OK;
	}
	else
	{
		std::cout << "Unable to open the nv12 file";
		return E_FAIL;
	}
}

HRESULT Frame::ReadNV12DataFromFile(char * file_path)
{
	
	std::ifstream nv12_file(file_path, std::ios::binary);
	if (nv12_file.is_open())
	{
		nv12_file.read(reinterpret_cast<char *>(&width), sizeof(width));
		nv12_file.read(reinterpret_cast<char *>(&height), sizeof(height));
		nv12_file.read(reinterpret_cast<char *>(&pitch), sizeof(pitch));

		if (!AllocBuffers())
			return E_FAIL;
		
		nv12_file.read(reinterpret_cast<char *>(Y), pitch*height);
		nv12_file.read(reinterpret_cast<char *>(UV), pitch*height / 2);
	
		nv12_file.close();

		return S_OK;
	}
	else
	{
		std::cout << "Unable to open the nv12 file";
		return E_FAIL;
	}
}

void Frame::ReadRGBData(BYTE * B)
{
	this->B = B;
}

void Frame::ReadYUV420Data(BYTE * Y, BYTE * U, BYTE * V)
{
	this->Y = Y;
	this->U = U;
	this->V = V;
}

void Frame::ReadNV12Data(BYTE * Y, BYTE * UV)
{
	this->Y = Y;
	this->UV = UV;
	this->U = UV;
	this->V = U + 1;
}

bool Frame::AllocBuffers()
{
	switch (type)
	{
	case RGB32:
	case RGB24:
		B = new BYTE[pitch*height];
		if (!B)
		{
			std::cout << "Unable to allocate RGB buffers";
			return false;
		}
		G = B + 1;
		R = G + 1;
		return true;
	case YUV420:
		Y = new BYTE[pitch*height * 3 / 2];
		if (!Y)
		{
			std::cout << "Unable to allocate YUV420 buffers";
			return false;
		}
		U = Y + pitch * height;
		V = U + pitch / 2 * height / 2;
		return true;
	case NV12:
		Y = new BYTE[pitch*height * 3 / 2];
		if (!Y)
		{
			std::cout << "Unable to allocate NV12 buffers";
			return false;
		}
		UV = Y + pitch * height;
		U = UV;
		V = UV + 1;
		return true;
	}
}
