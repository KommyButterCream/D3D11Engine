cbuffer ColorBuffer : register(b1)
{
	float4 wireColor;
}

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    uint texIndex : TEXCOORD1;
};

float4 main(PS_IN input) : SV_TARGET
{
	return wireColor;
}