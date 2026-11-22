// 단일 채널(R8_UNORM / R16_UNORM) 타일/텍스처용 픽셀 셰이더.
//
// Gray 소스를 업로드 시점에 BGRA 4바이트로 확장하지 않고 1채널 그대로 두면
// VRAM 과 업로드 대역폭이 1/4 로 줄어든다. 3채널 복제는 여기서 공짜로 처리한다.

Texture2DArray texArray : register(t0);
SamplerState samp0 : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    uint texIndex : TEXCOORD1;
};

float4 main(PS_IN input) : SV_TARGET
{
    // 단일 채널 포맷은 Sample() 이 (r, 0, 0, 1) 을 반환한다.
    float gray = texArray.Sample(samp0, float3(input.uv, input.texIndex)).r;

    // 알파는 항상 1.0 으로 고정한다.
    // 백버퍼는 D2D 타깃(PREMULTIPLIED)과 공유되므로 알파가 1 이 아니면
    // 그 위에 그려지는 오버레이 블렌딩이 틀어진다.
    return float4(gray, gray, gray, 1.0f);
}
