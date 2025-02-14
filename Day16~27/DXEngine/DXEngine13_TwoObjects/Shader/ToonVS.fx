// 상수 버퍼.
cbuffer perObjectBuffer : register(b0)
{
	matrix world;
};

cbuffer perSceneBuffer : register(b1)
{
	matrix viewProjection;
	float3 worldLightPosition;
	float3 worldCameraPosition;
};

// 정점 셰이더 입력.
struct vs_input
{
	float4 position : POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
};

// 정점 셰이더 출력.
struct vs_output
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 diffuse : TEXCOORD1;

	float3 viewDir : TEXCOORD2;
	float3 reflection : TEXCOORD3;
};

// Entry Point(메인 함수).
vs_output main(vs_input input)
{
	vs_output output;

	// 공간 변환.
	// 월드-뷰-투영 변환.
	output.position = mul(input.position, world);

	// 임시로 라이트 위치 설정.
	//float3 worldLightPosition = float3(-500.0f, 500.0f, -500.0f);

	// 라이트 방향 구하기.
	float3 lightDir = normalize(output.position.xyz - worldLightPosition);

	// 월드 좌표계기준 노멀.
	float3 worldNormal = normalize(mul(input.normal, (float3x3)world));

	// 뷰 벡터.
	float3 viewDir = normalize(output.position.xyz - worldCameraPosition);
	output.viewDir = viewDir;

	// 반사벡터 구하기.
	output.reflection = reflect(lightDir, worldNormal);				//반사 벡터 구해주는 함수

	// 내적 연산(코사인 구하기).
	// LdotN.
	output.diffuse = dot(-lightDir, worldNormal);
	output.position = mul(output.position, viewProjection);

	//output.color = input.color;
	output.texCoord = input.texCoord;

	return output;
}