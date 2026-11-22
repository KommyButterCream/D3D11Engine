// 4채널(B8G8R8A8_UNORM) 타일/텍스처용 픽셀 셰이더.
// 단일 채널(R8_UNORM / R16_UNORM) 소스는 ImageGrayPS.hlsl 을 사용한다.

Texture2DArray texArray : register(t0);
SamplerState samp0 : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    uint texIndex : TEXCOORD1; // VS 에서 넘어온 배열 인덱스
};

float4 main(PS_IN input) : SV_TARGET
{
    float4 color = texArray.Sample(samp0, float3(input.uv, input.texIndex));

    // 알파는 항상 1.0 으로 고정한다.
    // 백버퍼는 D2D 타깃 비트맵(D2D1_ALPHA_MODE_PREMULTIPLIED)과 같은 서피스를
    // 공유하므로, 4채널 소스의 알파가 그대로 기록되면 D2D 가 그 서피스를
    // premultiplied 로 해석해 오버레이가 겹친 영역의 색이 틀어진다.
    return float4(color.rgb, 1.0f);
}
