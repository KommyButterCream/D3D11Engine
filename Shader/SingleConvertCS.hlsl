RWTexture2D<float4> DestTexture : register(u0);
ByteAddressBuffer SrcRawData : register(t0);

cbuffer Params : register(b0)
{
	uint width;
	uint height;
	uint stride;
	uint channel;
};

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	if (DTid.x >= width || DTid.y >= height)
		return;

	uint byteOffset = (DTid.y * stride) + (DTid.x* channel);
	float4 color = float4(0, 0, 0, 1);

	if (channel == 1)
	{
		// 8-bit Gray
		uint val = SrcRawData.Load(byteOffset & ~3);
		uint shift = (byteOffset % 4) * 8;
		float gray = ((val >> shift) & 0xFF) / 255.0f;
		color = float4(gray, gray, gray, 1.0f);
	}
	else if (channel == 3)
	{
		// 24-bit BGR
		uint base = byteOffset & ~3;
		uint shift = (byteOffset % 4) * 8;

		// 4바이트씩 두 개를 읽어 8바이트 구간 확보
        uint low = SrcRawData.Load(base);
        uint high = SrcRawData.Load(base + 4);
        
        uint rgb = 0;
        if (shift <= 8) 
        {
            // shift가 0인 경우: [B G R A] 에서 [B G R]만 필요 -> low에서 해결
            // shift가 8인 경우: [A B G R] 에서 [B G R] 추출 -> low에서 해결
            rgb = (low >> shift) & 0xFFFFFF;
        }
        else 
        {
            // shift가 16 또는 24인 경우: low와 high에 걸쳐 있음
            // low의 윗부분과 high의 아랫부분을 조합
            uint lowPart = (low >> shift); // low에서 남은 비트들
            uint highPart = (high << (32 - shift)); // high에서 가져올 비트들
            rgb = (lowPart | highPart) & 0xFFFFFF;
        }

		color.b = (rgb & 0xFF) / 255.0f;
		color.g = ((rgb >> 8) & 0xFF) / 255.0f;
		color.r = ((rgb >> 16) & 0xFF) / 255.0f;
	}

	DestTexture[DTid.xy] = color;
}