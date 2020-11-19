//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;

	float4 DiffuseMtrl;
	float4 DiffuseLight;
	float3 LightVecW;
	float gTime;

	float4 AmbientMtrl;
	float4 AmbientLight;

	float4 SpecularMtrl;
	float4 SpecularLight;
	float3 EyePosW;
	float SpecularPower;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 normalW : NORMAL;
	float3 PosW : POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader -- Implements Gouraud Shading using Diffuse lighting only
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( float4 Pos : POSITION, float3 NormalL : NORMAL )
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	
	/*output.Pos = mul( Pos, World );
	output.Pos = mul( output.Pos, View );
	output.Pos = mul( output.Pos, Projection );

	// Convert from local space to world space
	// W component of vector is 0 as vectors cannot be translated
	float3 normalW = mul(float4(NormalL, 0.0f), World).xyz;
    normalW = normalize(normalW);

	// Transform vertex position to world space
	float4 posW = mul(Pos, World);
	output.PosW = posW.xyz;

	// Transform to homogeneous clip space
	float4 pos = mul(Pos, posW);
	output.Pos = pos;*/
	

	output.Pos = mul(Pos, World);
	output.PosW = output.Pos;
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	//output.normalW = float4(1, 1, 1, 1);

	// Convert from local space to world space
	output.normalW = mul(float4(NormalL, 0.0f), World).xyz;
	output.normalW = normalize(output.normalW);

	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	float3 toEye = normalize(EyePosW - input.PosW.xyz);

	// Interpolated normals can become unnormal - so normalize
	input.normalW = normalize(input.normalW);

	float diffuseAmount = max(dot(LightVecW, input.normalW), 0.0f);
	float3 diffuse = diffuseAmount * (DiffuseMtrl * DiffuseLight).rgb;

	float3 ambient = AmbientMtrl * AmbientLight;

	// Compute the reflection vector
	float3 r = reflect(-LightVecW, input.normalW);

	// Determine how much (if any) specular light makes it into the eye
	float specularAmount = pow(max(dot(r, toEye), 0.0f), SpecularPower);

	// Compute specular terms separately
	float3 specular = specularAmount * (SpecularMtrl * SpecularLight).rgb;

	float4 Color;
	Color.rgb = (diffuse)+(ambient)+(specular);
	Color.a = DiffuseMtrl.a;

	return Color;
}
