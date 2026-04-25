// Texture2D 대신 Texture2DArray 사용
Texture2DArray texArray : register(t0); 
SamplerState samp0 : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    uint texIndex : TEXCOORD1; // VS에서 넘어온 인덱스
};

float4 main(PS_IN input) : SV_TARGET
{
    // 32비트 타일 풀에서 샘플링 (이미 BGRA 데이터가 들어있음)
    float4 color = texArray.Sample(samp0, float3(input.uv, input.texIndex));
    
    // 만약 DXGI 화면 캡처(BGRA)와 이미지(RGBA)의 채널이 섞인다면 
    // 여기서 .rgba 또는 .bgra 로 스위즐링을 조정할 수 있습니다.

    return color;
}