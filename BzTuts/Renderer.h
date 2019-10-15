#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <D3Dcompiler.h>

#include "d3dx12.h"
#include "Image Loader.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

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

	HWND hwnd = NULL;

	LPCTSTR WindowName = L"BzTutsApp";

	LPCTSTR WindowTitle = L"Bz Window";

	int Width = 800;
	int Height = 600;

	bool FullScreen = false;

	bool Running = true;
	

	ID3D12Device* device;

	IDXGISwapChain3* swapChain;

	ID3D12CommandQueue* commandQueue;

	ID3D12DescriptorHeap* rtvDescriptorHeap;

	ID3D12Resource* renderTargets[frameBufferCount];

	ID3D12CommandAllocator* commandAllocator[frameBufferCount];

	ID3D12GraphicsCommandList* commandList;

	ID3D12Fence* fence[frameBufferCount];
	
	UINT64 fenceValue[frameBufferCount];

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
public:
	LRESULT WndCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool InitD3D();

	void Update();

	void UpdatePipeline();

	void Render();

	void Cleanup();

	void WaitForPreviousFrame();

	bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, bool fullscreen);

	void mainloop();

	HANDLE fenceEvent;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);