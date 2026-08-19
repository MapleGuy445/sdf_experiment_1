FEATURES
{
	#include "common/features.hlsl"
}

MODES
{
    Forward();
    Depth();
}

COMMON
{
	#include "common/shared.hlsl"
}

struct VertexInput
{
	#include "common/vertexinput.hlsl"
};

struct PixelInput
{
	#include "common/pixelinput.hlsl"
};

VS
{
	#include "common/vertex.hlsl"

	PixelInput MainVs( VertexInput i )
	{
		PixelInput o = ProcessVertex( i );
		// Add your vertex manipulation functions here
		return FinalizeVertex( o );
	}
}

PS
{
	#include "common/pixel.hlsl"

    struct SdfPixelOutput
    {
        float4 color : SV_Target0;
        float depth : SV_Depth;
    };

    float radius < Attribute("radius"); > ;
    float3 sphereOrigin < Attribute("sphereOrigin"); > ;

    float sdSphere(float3 p, float3 cen, float rad)
    {
        return length(p - cen) - rad;
    }

    SdfPixelOutput MainPs(PixelInput i)
    {
        SdfPixelOutput o;
        Material m = Material::Init(i);

        float3 worldPos = Depth::GetWorldPosition(i.vPositionSs.xy);
        float3 ro = g_vCameraPositionWs;
        float3 rd = normalize(worldPos - ro);

        float tmin = 1.0;
        float tmax = 4000.0;
        float t = tmin;
        bool hit = false;

        for (int j = 0; j < 200 && t < tmax; j++)
        {
            float d = sdSphere(ro + rd * t, sphereOrigin, radius);
            if (d < 0.001 * t) { hit = true; break; }
            t += d;
        }
        if (!hit) discard;

        float3 hitPosWs = ro + rd * t;

        float4 hitPosPs = Position3WsToPs(hitPosWs);
        float hitDepth = hitPosPs.z / hitPosPs.w;

        m.Albedo = float3(1, 0, 0);
        o.color = ShadingModelStandard::Shade(m);

        o.depth = hitDepth;

        return o;
    }
}
