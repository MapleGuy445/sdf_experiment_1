MODES
{
    Forward();
    Depth();
}

COMMON
{
    #include "postprocess/shared.hlsl"
}

struct VertexInput
{
    float3 vPositionOs : POSITION < Semantic(PosXyz); > ;
};

struct PixelInput
{
	#include "common/pixelinput.hlsl"
};

VS
{
    PixelInput MainVs(VertexInput i)
    {
        PixelInput o;

        o.vPositionPs = float4(i.vPositionOs.xy, 0.0f, 1.0f);

        return o;
    }
}

PS
{
    #include "common/pixel.hlsl"
    #include "postprocess/common.hlsl"
    #include "postprocess/functions.hlsl"

    float4 MainPs( PixelInput i ) : SV_Target0
    {
		return float4(float3(Depth::Get(i.vPositionSs.xy)), 1.0);
	}
}