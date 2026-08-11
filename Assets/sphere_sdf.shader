MODES
{
    Forward();
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
    #include "procedural.hlsl"

    Texture2D colorBuffer < Attribute("ColorBuffer"); SrgbRead(true); > ;

    float radius < Attribute("radius"); > ;

    float sdSphere(float3 p, float3 cen, float rad)
    {
        return length(p - cen) - rad;
    }

	float4 MainPs( PixelInput i ) : SV_Target0
    {
        float2 uv = i.vPositionSs.xy / g_vRenderTargetSize.xy;
        float4 color = colorBuffer.SampleLevel(g_sBilinearMirror, uv, 0);
        float3 worldPos = Depth::GetWorldPosition(i.vPositionSs.xy);
        //Show depth buffer to screen
        //return float4(float3(Depth::GetNormalized(i.vPositionSs.xy)), 1.0);
        float3 ro = g_vCameraPositionWs;
        float3 rd = normalize(worldPos - ro);;

        float tmin = 1.0; // min distance we start at
        float tmax = 2000.0; // max distance we can reach
        float d = 0.0;  // calculated signed distance from SDF
        float t = tmin; // distance travel
        bool hit = false;
        for (int i = 0; i < 200 && t < tmax; i++)
        {
            float d1 = sdSphere(ro + rd * t, float3(0.0, 0.0, 0.0), radius);
            float d2 = sdSphere(ro + rd * t, float3(30.0, 0.0, 0.0), radius);
            d = max(d1, -d2);
            if (d < 0.001) { hit = true; break; }
            t += d;
        }

        if (hit && t < Depth::GetLinear(i.vPositionSs.xy)) {
            color = float4(1, 1, 1, 1);
		}

        return color;
	}
}