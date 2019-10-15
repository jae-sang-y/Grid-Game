#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <D3Dcompiler.h>

#include "d3dx12.h"
#include "Image Loader.h"

#include <unordered_map>
#include <string>

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
		: ErrorCode(hr), FunctionName(functionName), Filename(filename), LineNumber(lineNumber) {
	}

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

using namespace DirectX;

struct Vertex {
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), texCoord(u, v) {}
	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
};

struct ConstantBufferPerObject {
	XMFLOAT4X4 wvpMat;
};

constexpr int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;

class Renderer {
	constexpr static int frameBufferCount = 3;

	LPCTSTR WindowName = L"BzTutsApp";

	LPCTSTR WindowTitle = L"Bz Window";

	int Width = 800;
	int Height = 600;

	bool FullScreen = false;

	bool Running = true;

	ID3D12Device* m_device;

	IDXGISwapChain3* m_swapChain;

	ID3D12CommandQueue* m_commandQueue;

	ID3D12DescriptorHeap* m_rtvDescriptorHeap;

	ID3D12Resource* m_renderTargets[frameBufferCount];

	ID3D12CommandAllocator* m_commandAllocator[frameBufferCount];

	ID3D12GraphicsCommandList* m_commandList;

	ID3D12Fence* m_fence[frameBufferCount];

	UINT64 m_fenceValue[frameBufferCount];

	int frameIndex;

	int rtvDescriptorSize;

	ID3D12PipelineState* pipelineStateObject;

	ID3D12RootSignature* rootSignature;

	D3D12_VIEWPORT viewport;

	D3D12_RECT scissorRect;

	ID3D12Resource* vertexBuffer;
	ID3D12Resource* indexBuffer;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	ID3D12Resource* depthStencilBuffer;
	ID3D12DescriptorHeap* dsDescriptorHeap;

	ConstantBufferPerObject cbPerObject;

	ID3D12Resource* constantBufferUploadHeaps[frameBufferCount];

	UINT8* cbvGPUAddress[frameBufferCount];

	XMFLOAT4X4 cameraProjMat;
	XMFLOAT4X4 cameraViewMat;

	XMFLOAT4 cameraPosition;
	XMFLOAT4 cameraTarget;
	XMFLOAT4 cameraUp;

	XMFLOAT4X4 cube1WorldMat;
	XMFLOAT4X4 cube1RotMat;
	XMFLOAT4 cube1Position;

	XMFLOAT4X4 cube2WorldMat;
	XMFLOAT4X4 cube2RotMat;
	XMFLOAT4 cube2PositionOffset;

	int numCubeIndices;

	ID3D12Resource* textureBuffer;
	ID3D12Resource* textureBuffer1;

	ID3D12DescriptorHeap* mainDescriptorHeap;
	ID3D12Resource* textureBufferUploadHeap;

	std::unordered_map<std::wstring, D3D12_SHADER_BYTECODE> m_shader;
	std::unordered_map < std::wstring, std::vector<D3D12_INPUT_ELEMENT_DESC> > m_inputlayout;

public:
	LRESULT WndCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void InitD3D();

	void Update();

	void UpdatePipeline();

	void Render();

	void Cleanup();

	void WaitForPreviousFrame();

	void InitializeWindow(HINSTANCE hInstance, int ShowWnd, bool fullscreen);

	void mainloop();

	void CreateDxgiFactory(IDXGIFactory4** ptr);
	void InitDevice(IDXGIFactory4* dxgiFactory);
	void CreateCommandQueue();
	void CreateSwapChain(IDXGIFactory4* dxgiFactory);
	void CreateCommandObject();
	void CreateRootSignature();
	void BuildShader();
	void BuildPSO();
	void BuildInputlayout();

	HWND hwnd = NULL;
	HANDLE m_fenceEvent;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);