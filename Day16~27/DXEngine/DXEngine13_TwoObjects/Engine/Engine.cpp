#include "Engine.h"

Engine::Engine(HINSTANCE hinstance) : DXApp(hinstance)
{

}

Engine::~Engine()
{
	Memory::SafeDelete(mesh);
	Memory::SafeDelete(mesh2);

	material->Release();
	Memory::SafeDelete(material);

	material2->Release();
	Memory::SafeDelete(material2);

	Memory::SafeRelease(constantBuffer);

	//vertexShader->Release();
	//Memory::SafeDelete(vertexShader);

	//pixelShader->Release();
	//Memory::SafeDelete(pixelShader);
}

int Engine::Run()
{
	return window->Run(this);
}

bool Engine::Init()
{
	if (DXApp::Init() == false)
		return false;

	// 장면 초기화.
	if (InitializeScene() == false)
		return false;

	// 행렬 초기화.
	if (InitializeTransformation() == false)
		return false;

	return true;
}

void Engine::Update()
{
	//// 월드 행렬 바인딩.
	//mesh->Update(deviceContext);

	// 뷰/투영 행렬 바인딩.
	deviceContext->VSSetConstantBuffers(1, 1, &constantBuffer);
}

void Engine::Render()
{
	float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// 렌더 타겟을 설정한 색상으로 칠하기.
	deviceContext->ClearRenderTargetView(renderTargetView, color);

	// 뎁스/스텐실 뷰 지우기.
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	// 월드 행렬 바인딩.
	mesh->Update(deviceContext);

	// 셰이더 바인딩.
	//vertexShader->BindShader(deviceContext);
	//pixelShader->BindShader(deviceContext);
	material->BindShaders(deviceContext);

	// 텍스처/샘플러 스테이트 바인딩.
	//pixelShader->BindTexture(deviceContext);
	//pixelShader->BindSamplerState(deviceContext);
	material->BindTexture(deviceContext);
	material->BindSamplerState(deviceContext);

	// 메시 버퍼 그리기.
	mesh->RenderBuffers(deviceContext);


	// 월드 행렬 바인딩.
	mesh2->Update(deviceContext);

	// 셰이더 바인딩.
	//vertexShader->BindShader(deviceContext);
	//pixelShader->BindShader(deviceContext);
	material2->BindShaders(deviceContext);

	// 텍스처/샘플러 스테이트 바인딩.
	//pixelShader->BindTexture(deviceContext);
	//pixelShader->BindSamplerState(deviceContext);
	material2->BindTexture(deviceContext);
	material2->BindSamplerState(deviceContext);

	// 메시 버퍼 그리기.
	mesh2->RenderBuffers(deviceContext);



	// 백버퍼 <-> 프론트 버퍼 교환.
	swapChain->Present(1, 0);

}

bool Engine::InitializeScene()
{
	// 머티리얼 객체 생성.
	material = new Material(TEXT("Shader/Specular"));
	material2 = new Material(TEXT("Shader/Specular"));

	// 머티리얼 컴파일.
	if (material->CompileShaders(device) == false)
		return false;
	if (material2->CompileShaders(device) == false)
		return false;

	// 각 셰이더 객체 생성,
	if (material->CreateShaders(device) == false)
		return false;
	if (material2->CreateShaders(device) == false)
		return false;

	// 텍스처 관련 처리.
	// 텍스처 추가.
	material->AddTexture(TEXT("Resources/Textures/T_Chr_FPS_D.png"));
	material2->AddTexture(TEXT("Resources/Textures/T_Chr_FPS_D.png"));

	// 텍스처 로드
	if (material->LoadTexture(device) == false)
		return false;
	if (material2->LoadTexture(device) == false)
		return false;

	// 샘플러 스테이트 생성.
	if (material->CreateSamplerState(device) == false)
		return false;
	if (material2->CreateSamplerState(device) == false)
		return false;

	//// 메쉬 생성.
	////mesh = new Mesh(0.0f, 0.0f, 0.0f);
	mesh = new Mesh("Resources/Models/HeroTPP.FBX");
	mesh->SetPosition(-70.0f, -90.0f, 0.0f);
	mesh->SetRotation(-90.0f, 180.0f, 0.0f);
	mesh2 = new Mesh("Resources/Models/HeroTPP.FBX");
	mesh2->SetPosition(70.0f, -90.0f, 0.0f);
	mesh2->SetRotation(-90.0f, 180.0f, 0.0f);

	// 초기화.
	if (mesh->InitializeBuffers(device, material) == false)
		return false;
	if (mesh2->InitializeBuffers(device, material2) == false)
		return false;

	return true;
}

bool Engine::InitializeTransformation()
{
	// 카메라 정보 설정.
	cameraPosition = XMVectorSet(0.0f, 0.0f, -200.0f, 0.0f);
	cameraView = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	camerUpVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// 뷰 행렬.
	XMMATRIX view = XMMatrixLookAtLH(cameraPosition, cameraView, camerUpVector);

	// 투영 행렬.
	// 시야각 / 종횡비 설정.
	float fovY = XMConvertToRadians(70.0f);
	float aspectRatio = static_cast<float>(window->GetScreenWidth()) / static_cast<float>(window->GetScreenHeight());

	XMMATRIX projection = XMMatrixPerspectiveFovLH(fovY, aspectRatio, 1.0f, 10000.0f);

	// 버퍼에 담을 구조체 변수 설정.
	PerSceneBuffer matrixData;
	matrixData.viewProjection = XMMatrixTranspose(view*projection);
	matrixData.worldLightPosition = XMFLOAT3(500.0f, 500.0f, -500.0f);
	float x = XMVectorGetX(cameraPosition);
	float y = XMVectorGetY(cameraPosition);
	float z = XMVectorGetZ(cameraPosition);
	matrixData.worldCameraPosition = XMFLOAT3(x, y, z);

	// 버퍼 생성.
	// 버퍼 서술자.
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(D3D11_BUFFER_DESC));

	cbDesc.ByteWidth = sizeof(PerSceneBuffer);
	cbDesc.CPUAccessFlags = 0;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.MiscFlags = 0;
	cbDesc.Usage = D3D11_USAGE_DEFAULT;

	// 인덱스 배열 정보 넣어줄 구조체.
	D3D11_SUBRESOURCE_DATA cbData;
	ZeroMemory(&cbData, sizeof(D3D11_SUBRESOURCE_DATA));
	cbData.pSysMem = &matrixData;

	// 상수 버퍼 생성.
	HRESULT result = device->CreateBuffer(&cbDesc, &cbData, &constantBuffer);
	if (IsError(result, TEXT("씬 상수 버퍼 생성 실패")))
	{
		return false;
	}

	return true;
}