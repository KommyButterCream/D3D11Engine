cbuffer ViewCB : register(b0)
{
	float2 scale;
	float2 offset;
}

struct VS_IN
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD0;
	uint texIndex : TEXCOORD1;
};

struct VS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	uint texIndex : TEXCOORD1;
};

VS_OUT main(VS_IN input)
{
	VS_OUT o;

	float2 p = input.pos.xy;

    // 화면 변환만 적용
    p *= scale;
    p += offset;

    o.pos = float4(p, 0, 1);
    o.uv = input.uv;
    o.texIndex = input.texIndex;
	
	return o;
}