#include "DXApp.h"

DXApp::DXApp(HINSTANCE hinstance)
{
	// 윈도우 객체 생성.
	window = new AppWindow(hinstance);
}


DXApp::~DXApp()
{
	//delete window;
	Memory::SafeDelete(window);

	// DX 리소스 해제.
	Memory::SafeRelease(device);
	Memory::SafeRelease(deviceContext);
	Memory::SafeRelease(swapChain);
	Memory::SafeRelease(renderTargetView);
}

bool DXApp::Init()
{
	// 창 초기화.
	if (window->InitializeWindow() == false)
		return false;

	// DX 초기화.
	if (InitializeDirect3D() == false)
		return false;

	return true;
}

bool DXApp::InitializeDirect3D()
{
	// 스왑체인 생성 - Back/Front 버퍼 교환용.
	// 서술자 (Descriptor).
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = 1;		//Front버퍼는 이미 존재하므로 Back버퍼를 하나 추가
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = window->GetWindowHandle();
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		//어떤식으로 칠할지 색 옵션 설정
	swapDesc.BufferDesc.Width = window->GetScreenWidth();
	swapDesc.BufferDesc.Height = window->GetScreenHeight();

	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	// 스왑체인 & 장치 & 장치문맥 생성.
	HRESULT result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &swapDesc, &swapChain, &device, NULL, &deviceContext);

	// 오류 확인.
	if (FAILED(result))
	{
		MessageBox(NULL, TEXT("장치 생성 실패"), TEXT("오류"), MB_OK);
		return false;
	}

	// 백버퍼(텍스처) 생성.
	ID3D11Texture2D* backBufferTexture;
	result = swapChain->GetBuffer(NULL, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);

	if (FAILED(result))
	{
		MessageBox(NULL, TEXT("백버퍼 생성 실패"), TEXT("오류"), MB_OK);
		return false;
	}

	// 렌더 타겟 뷰 생성.
	result = device->CreateRenderTargetView(backBufferTexture, NULL, &renderTargetView);

	if (FAILED(result))
	{
		MessageBox(NULL, TEXT("렌더 타겟 생성 실패"), TEXT("오류"), MB_OK);
		return false;
	}

	// 렌더 타겟 연결.
	deviceContext->OMSetRenderTargets(1, &renderTargetView, NULL);

	// 버퍼 생성을 위해 사용했던 자원 해제.
	Memory::SafeRelease(backBufferTexture);

	// 뷰포트 설정.
	SetViewport();

	return true;
}

void DXApp::SetViewport()
{
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(window->GetScreenWidth());
	viewport.Height = static_cast<float>(window->GetScreenHeight());

	// 뷰포트 설정.
	deviceContext->RSSetViewports(1, &viewport);
}
