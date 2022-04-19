#include "DXUT.h"
#include "SDKmisc.h"
#include "resource.h"

#include "View.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

#pragma comment(lib, "legacy_stdio_definitions.lib")

struct CBufferWVP {
	XMFLOAT4X4 World = { 1, 0, 0, 0 ,0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	XMFLOAT4X4 View = { 1, 0, 0, 0 ,0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	XMFLOAT4X4 Proj = { 1, 0, 0, 0 ,0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
};

struct CBufferDiffuse {
	XMFLOAT4 Diffuse = { 1, 0, 0, 1 };
};

struct hr {
	void operator <<(HRESULT hr) {
		if (FAILED(hr)) {
			std::string txt{ DXGetErrorDescriptionA(hr) };
			OutputDebugStringA(txt.c_str());
			throw hr;
		}
	}
} hr;

struct DXView : View {
	ComPtr<ID3DX10Font> font = nullptr;
	ComPtr<ID3D10InputLayout> IL = nullptr;
	ComPtr< ID3DX10Sprite> sprite = nullptr;
	ComPtr<ID3D10Buffer> VB = nullptr;
	ComPtr<ID3D10Buffer> IB = nullptr;
	ComPtr<ID3D10VertexShader> VS = nullptr;
	ComPtr<ID3D10PixelShader> PS = nullptr;

	ComPtr<ID3D10Buffer> WVP = nullptr;
	ComPtr<ID3D10Buffer> Diffuse = nullptr;
	ComPtr<ID3D10RasterizerState> RS = nullptr;
	ComPtr<ID3D10DepthStencilState> DSS = nullptr;
	ComPtr<ID3D10BlendState> BS = nullptr;
	ComPtr<ID3D10SamplerState> SS = nullptr;

	ComPtr<ID3D10Buffer> Rect = nullptr;
	ID3D10Device* D = nullptr;
	UINT RectOffset = 0;
	UINT RectStride = sizeof(XMFLOAT2);
	float to_real_Pw = 1.f;
	float to_real_Ph = 1.f;

	std::map<std::wstring, ComPtr<ID3D10ShaderResourceView>> textures = {};

	ID3D10ShaderResourceView* GetImage(std::wstring path) {
		if (textures.find(path) == textures.end())
			hr << D3DX10CreateShaderResourceViewFromFileW(D, path.c_str(), nullptr, nullptr, textures[path].GetAddressOf(), nullptr);
		return textures.at(path).Get();
	}

	void DrawImage(const std::wstring path, const XMFLOAT2 pos, const XMFLOAT2 size) override;
	void DrawRect(const XMFLOAT2 pos, const XMFLOAT2 size) override;
	void DrawStr(const XMFLOAT2 pos, const std::string text) override;
	void ClearDepth() override;
	XMFLOAT2 GetMousePos() override;
} *X = nullptr;
View* V = nullptr;

LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext)
{
	main_msg(hWnd, uMsg, wParam, lParam);
	return 0;
}

HRESULT CALLBACK OnD3D10CreateDevice(ID3D10Device* D, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	X = new DXView;
	X->D = D;
	V = X;
	if (V != X) throw;
	hr << D3DX10CreateSprite(D, 512, X->sprite.GetAddressOf());

	DWORD dwShaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif
	ComPtr<ID3D10Blob> PSBuf = nullptr;
	ComPtr<ID3D10Blob> VSBuf = nullptr;
	ComPtr<ID3D10Blob> Error = nullptr;

	HRESULT HR = D3DX10CompileFromFileW(L"res/fx.hlsl", NULL, NULL, "VS", "vs_4_0", dwShaderFlags, NULL, NULL, VSBuf.GetAddressOf(), Error.GetAddressOf(), NULL);
	if (FAILED(HR)) {
		std::string error{ (const char*)Error->GetBufferPointer() };
		hr << HR;
	}
	hr << D->CreateVertexShader((DWORD*)VSBuf->GetBufferPointer(), VSBuf->GetBufferSize(), X->VS.GetAddressOf());

	HR = D3DX10CompileFromFileW(L"res/fx.hlsl", NULL, NULL, "PS", "ps_4_0", dwShaderFlags, NULL, NULL, PSBuf.GetAddressOf(), Error.GetAddressOf(), NULL);
	if (FAILED(HR)) {
		std::string error{ (const char*)Error->GetBufferPointer() };
		hr << HR;
	}
	hr << D->CreatePixelShader((DWORD*)PSBuf->GetBufferPointer(), PSBuf->GetBufferSize(), X->PS.GetAddressOf());

	const D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,  D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr << D->CreateInputLayout(layout, 1, VSBuf->GetBufferPointer(), VSBuf->GetBufferSize(), X->IL.GetAddressOf());

	D3D10_BLEND_DESC BlendState = {};
	BlendState.BlendEnable[0] = TRUE;
	BlendState.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
	BlendState.BlendOp = D3D10_BLEND_OP_ADD;
	BlendState.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	BlendState.SrcBlend = D3D10_BLEND_SRC_ALPHA;
	BlendState.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
	BlendState.SrcBlendAlpha = D3D10_BLEND_ZERO;
	BlendState.DestBlendAlpha = D3D10_BLEND_ZERO;
	hr << D->CreateBlendState(&BlendState, X->BS.GetAddressOf());

	D3D10_RASTERIZER_DESC RSDesc = {};
	RSDesc.FillMode = D3D10_FILL_SOLID;
	RSDesc.CullMode = D3D10_CULL_NONE;
	RSDesc.FrontCounterClockwise = TRUE;
	RSDesc.DepthBias = 0;
	RSDesc.DepthBiasClamp = 0;
	RSDesc.SlopeScaledDepthBias = 0;
	RSDesc.ScissorEnable = FALSE;
	RSDesc.MultisampleEnable = TRUE;
	RSDesc.AntialiasedLineEnable = FALSE;
	hr << D->CreateRasterizerState(&RSDesc, X->RS.GetAddressOf());

	D3D10_DEPTH_STENCIL_DESC DSDesc = {};
	DSDesc.DepthEnable = TRUE;
	DSDesc.DepthFunc = D3D10_COMPARISON_LESS_EQUAL;
	DSDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
	hr << D->CreateDepthStencilState(&DSDesc, X->DSS.GetAddressOf());

	{
		std::vector<XMFLOAT2> vertices = {
			XMFLOAT2(0.f, 0.f),
			XMFLOAT2(1.f, 0.f),
			XMFLOAT2(0.f, 1.f),
			XMFLOAT2(1.f, 0.f),
			XMFLOAT2(1.f, 1.f),
			XMFLOAT2(0.f, 1.f),
		};

		D3D10_BUFFER_DESC BufferDesc = {};
		BufferDesc.BindFlags = D3D10_BIND_FLAG::D3D10_BIND_VERTEX_BUFFER;
		BufferDesc.ByteWidth = sizeof(XMFLOAT2) * vertices.size();
		BufferDesc.Usage = D3D10_USAGE_DEFAULT;
		D3D10_SUBRESOURCE_DATA data = {};
		data.pSysMem = vertices.data();

		hr << D->CreateBuffer(&BufferDesc, &data, X->Rect.GetAddressOf());
	}
	{
		D3D10_BUFFER_DESC BufferDesc = {};
		BufferDesc.BindFlags = D3D10_BIND_FLAG::D3D10_BIND_CONSTANT_BUFFER;
		BufferDesc.ByteWidth = sizeof(CBufferWVP);
		BufferDesc.Usage = D3D10_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		hr << D->CreateBuffer(&BufferDesc, nullptr, X->WVP.GetAddressOf());
	}
	{
		D3D10_BUFFER_DESC BufferDesc = {};
		BufferDesc.BindFlags = D3D10_BIND_FLAG::D3D10_BIND_CONSTANT_BUFFER;
		BufferDesc.ByteWidth = sizeof(CBufferDiffuse);
		BufferDesc.Usage = D3D10_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		hr << D->CreateBuffer(&BufferDesc, nullptr, X->Diffuse.GetAddressOf());
	}
	{
		D3D10_SAMPLER_DESC desc{};
		desc.AddressU = D3D10_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D10_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D10_TEXTURE_ADDRESS_CLAMP;
		desc.Filter = D3D10_FILTER_ANISOTROPIC;
		hr << D->CreateSamplerState(&desc, X->SS.GetAddressOf());
	}
	{
		hr << D3DX10CreateFontA(D, 50, 0, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas", X->font.GetAddressOf());
	}
	return S_OK;
}

HRESULT CALLBACK OnD3D10ResizedSwapChain(ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	X->to_real_Pw = (float)virtual_W / pBackBufferSurfaceDesc->Width;
	X->to_real_Ph = (float)virtual_H / pBackBufferSurfaceDesc->Height;
	return S_OK;
}

void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	X->DT = fElapsedTime;
	X->T = fTime;
	main_step();
}

void DXView::DrawStr(XMFLOAT2 pos, std::string text) {
	RECT rect{};
	rect.left = pos.x;
	rect.top = pos.y;
	X->font->DrawTextA(nullptr, text.data(), -1, &rect, DT_NOCLIP, D3DXCOLOR(X->color.x, X->color.y, X->color.z, X->color.w));
	X->color = X->default_color;
}

void DXView::DrawImage(std::wstring path, XMFLOAT2 pos, XMFLOAT2 size) {
	X->D->VSSetShader(X->VS.Get());
	X->D->PSSetShader(X->PS.Get());
	X->D->GSSetShader(nullptr);
	X->D->OMSetDepthStencilState(X->DSS.Get(), 0);
	X->D->RSSetState(X->RS.Get());
	X->D->OMSetBlendState(X->BS.Get(), nullptr, 0xffff);
	X->D->PSSetSamplers(0, 1, X->SS.GetAddressOf());

	X->D->IASetInputLayout(X->IL.Get());
	X->D->IASetVertexBuffers(0, 1, X->Rect.GetAddressOf(), &X->RectStride, &X->RectOffset);
	X->D->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CBufferWVP* WVP = nullptr;
	hr << X->WVP->Map(D3D10_MAP_WRITE_DISCARD, NULL, (void**)&WVP);
	XMStoreFloat4x4(&WVP->World, XMMatrixScaling(size.x, size.y, 1) * XMMatrixTranslation(pos.x, pos.y, 0.f));
	XMStoreFloat4x4(&WVP->View, XMMatrixIdentity());
	XMStoreFloat4x4(&WVP->Proj, XMMatrixScaling(2.f / virtual_W, -2.f / virtual_H, 1.f) * XMMatrixTranslation(-1, 1, 0));
	X->WVP->Unmap();

	CBufferDiffuse* Diffuse = nullptr;
	hr << X->Diffuse->Map(D3D10_MAP_WRITE_DISCARD, NULL, (void**)&Diffuse);
	Diffuse->Diffuse = color;
	X->Diffuse->Unmap();

	X->D->VSSetConstantBuffers(0, 1, X->WVP.GetAddressOf());
	X->D->PSSetConstantBuffers(0, 1, X->Diffuse.GetAddressOf());

	ID3D10ShaderResourceView* resources[] = { X->GetImage(path) };
	X->D->PSSetShaderResources(0, 1, resources);

	X->D->Draw(6, 0);
	X->color = X->default_color;
}

void DXView::DrawRect(XMFLOAT2 pos, XMFLOAT2 size)
{
	DrawImage(L"res/solid.bmp", pos, size);
}

void DXView::ClearDepth() {
	D->ClearDepthStencilView(DXUTGetD3D10DepthStencilView(), D3D10_CLEAR_DEPTH, 1.f, 0);
}

XMFLOAT2 DXView::GetMousePos() {
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(DXUTGetHWND(), &p);
	return XMFLOAT2(
		roundf(p.x * to_real_Pw),
		roundf(p.y * to_real_Ph)
	);
}

void CALLBACK OnD3D10FrameRender(ID3D10Device* D, double fTime, float fElapsedTime, void* pUserContext)
{
	static float bg[4] = { 0.25f, 0.25f, 0.25f, 1.f };
	D->ClearRenderTargetView(DXUTGetD3D10RenderTargetView(), bg);
	X->ClearDepth();
	X->DT = fElapsedTime;
	X->T = fTime;
	main_draw();
}

void CALLBACK OnD3D10DestroyDevice(void* pUserContext)
{
	delete X;
	V = nullptr;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackFrameMove(OnFrameMove);

	DXUTSetCallbackD3D10DeviceCreated(OnD3D10CreateDevice);
	DXUTSetCallbackD3D10SwapChainResized(OnD3D10ResizedSwapChain);
	DXUTSetCallbackD3D10DeviceDestroyed(OnD3D10DestroyDevice);
	DXUTSetCallbackD3D10FrameRender(OnD3D10FrameRender);

	DXUTInit(true, true, NULL); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings(true, true); // Show the cursor and clip it when in full screen
	DXUTCreateWindow(L"Grid Game");
	DXUTCreateDevice(true, virtual_W, virtual_H);
	main_start();
	DXUTMainLoop(); // Enter into the DXUT render loop
	main_end();

	return DXUTGetExitCode();
}