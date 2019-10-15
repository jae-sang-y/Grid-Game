#pragma once

#include <dxgi1_4.h>
#include <wincodec.h>
#include <d3d12.h>

#ifndef _IMAGE_LOADER

#define _IMAGE_LOADER
int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow);

DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
#endif