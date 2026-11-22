// 단일 채널 타일/텍스처 + LUT 픽셀 셰이더.
//
// ImageGrayPS.hlsl 과 짝이다. 상수 버퍼로 분기하지 않고 셰이더를 둘로 나눈
// 이유는, LUT 를 안 쓰는 경로가 예전과 완전히 같은 코드로 남기 때문이다.
// 켜고 끄는 것은 SetCommonShaderStates 가 어느 셰이더를 바인딩하느냐로 정해진다.
//
// LUT 텍스처에는 자동 대비(퍼센타일 스트레치)와 컬러맵이 함께 구워져 있다.
// 그래서 여기서는 페치 한 번이면 끝난다.
//
//   16bit 이미지가 이 경로를 꼭 타야 하는 이유:
//   백버퍼가 B8G8R8A8_UNORM 이라 65536 계조가 256 으로 뭉개진다. LUT 없이는
//   관심 구간이 몇 계조 안에 갇혀서 사실상 상위 8비트만 보게 된다.

Texture2DArray texArray : register(t0);

// N x 1 RGBA8. N 은 8bit 이면 256, 16bit 이면 65536.
Texture2D lutTex : register(t1);

SamplerState samp0 : register(s0);

// LUT 전용 샘플러(선형 + CLAMP).
//
// 선형으로 두는 이유: 텍셀 중심을 맞추려면 셰이더가 LUT 크기를 알아야 하고
// 그러면 상수 버퍼가 다시 필요해진다. 프리셋은 모두 연속적인 색 램프라
// 이웃 항목 사이를 섞어도 눈에 띄는 차이가 없고, 오히려 더 매끄럽다.
SamplerState lutSamp : register(s1);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    uint texIndex : TEXCOORD1;
};

float4 main(PS_IN input) : SV_TARGET
{
    // 단일 채널 포맷은 Sample() 이 (r, 0, 0, 1) 을 반환한다.
    // R8 은 raw/255, R16 은 raw/65535 로 정규화된 값이다.
    float gray = texArray.Sample(samp0, float3(input.uv, input.texIndex)).r;

    float3 color = lutTex.Sample(lutSamp, float2(gray, 0.5f)).rgb;

    // 알파는 항상 1.0 으로 고정한다.
    // 백버퍼는 D2D 타깃(PREMULTIPLIED)과 공유되므로 알파가 1 이 아니면
    // 그 위에 그려지는 오버레이 블렌딩이 틀어진다.
    return float4(color, 1.0f);
}
