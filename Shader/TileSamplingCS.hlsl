ByteAddressBuffer RawInput : register(t0);
RWTexture2DArray<float4> OutputArray : register(u0);

cbuffer Config : register(b0)
{
	uint g_lodScale;
	uint g_channels;
	uint g_destIndex;
	uint g_cacheWidth;
	uint g_srcOffsetX;   // 캐시 내 타일 시작점 X
    uint g_srcOffsetY;   // 캐시 내 타일 시작점 Y
}

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// DTid 는 타일 내부의 픽셀 좌표(0~511)
	uint tileSize;
	uint height;
	uint elements;
	OutputArray.GetDimensions(tileSize, height, elements);

	if (DTid.x >= tileSize || DTid.y >= tileSize) return;

	// 원본 데이터 내에서의 샘플링 좌표 (LOD 고려)
	uint srcX = DTid.x * g_lodScale;
	uint srcY = DTid.y * g_lodScale;

	// 타일 내 픽셀 좌표를 캐시 버퍼 내 좌표로 변환
    uint cacheX = g_srcOffsetX + (DTid.x * g_lodScale);
    uint cacheY = g_srcOffsetY + (DTid.y * g_lodScale);

    // Raw Buffer 바이트 오프셋 계산
    uint byteOffset = (cacheY * g_cacheWidth + cacheX) * g_channels;

	float4 color = float4(0, 0, 0, 1);

	if (g_channels == 1)
	{
		// 8-bit Gray
		uint val = RawInput.Load(byteOffset & ~3);
		uint shift = (byteOffset % 4) * 8;
		float gray = ((val >> shift) & 0xFF) / 255.0f;
		color = float4(gray, gray, gray, 1.0f);
	}	
	else if (g_channels == 3)
	{
		// 24-bit BGR
		uint base = byteOffset & ~3;
		uint shift = (byteOffset % 4) * 8;

		// 4바이트씩 두 개를 읽어 8바이트 구간 확보
        uint low = RawInput.Load(base);
        uint high = RawInput.Load(base + 4);
        
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
	else if (g_channels == 4)
	{
		// 32-bit BGRA
		uint val = RawInput.Load(byteOffset);
		color.b = (val & 0xFF) / 255.0f;
		color.g = ((val >> 8) & 0xFF) / 255.0f;
		color.r = ((val >> 16) & 0xFF) / 255.0f;
		color.a = ((val >> 24) & 0xFF) / 255.0f;
	}

	OutputArray[uint3(DTid.xy, g_destIndex)] = color;
}