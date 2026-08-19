PS
{
    #include "common/pixel.hlsl"
    #include "postprocess/common.hlsl"
    #include "postprocess/functions.hlsl"
    #include "procedural.hlsl" // assume this exposes a Perlin3D(float3) or similar

    Texture2D colorBuffer < Attribute("ColorBuffer"); SrgbRead(true); > ;

    float3 volumeCenter < Attribute("volumeCenter"); > ;
    float volumeRadius < Attribute("volumeRadius"); > ;
    float stepSize < Attribute("stepSize"); > ; // fixed world-space step
    int maxSteps < Attribute("maxSteps"); > ; // hard safety cap
    float densityScale < Attribute("densityScale"); > ;
    float noiseFreq < Attribute("noiseFreq"); > ;

    // returns false if no intersection
    bool RaySphere(float3 ro, float3 rd, float3 cen, float rad, out float tNear, out float tFar)
    {
        float3 oc = ro - cen;
        float b = dot(oc, rd);
        float c = dot(oc, oc) - rad * rad;
        float disc = b * b - c;
        if (disc < 0.0) { tNear = 0; tFar = 0; return false; }
        float s = sqrt(disc);
        tNear = -b - s;
        tFar = -b + s;
        return true;
    }

    float4 MainPs( PixelInput i ) : SV_Target0
    {
        float2 uv = i.vPositionSs.xy / g_vRenderTargetSize.xy;
        float4 sceneColor = colorBuffer.SampleLevel(g_sBilinearMirror, uv, 0);
        float3 worldPos = Depth::GetWorldPosition(i.vPositionSs.xy);

        float3 ro = g_vCameraPositionWs;
        float3 rd = normalize(worldPos - ro);

        float tNear, tFar;
        if (!RaySphere(ro, rd, volumeCenter, volumeRadius, tNear, tFar))
            return sceneColor; // condition 1: pixel doesn't touch the bbox at all

        float sceneDepth = Depth::GetLinear(i.vPositionSs.xy);

        tNear = max(tNear, 0.0); // camera may be inside the volume
        tFar = min(tFar, sceneDepth); // occluded by scene geometry beyond/at that depth

        if (tFar <= tNear)
            return sceneColor; // volume is behind the camera or fully occluded

        // dither the start point to break up fixed-step banding
        float jitter = frac(sin(dot(uv, float2(12.9898,78.233))) * 43758.5453);
        float t = tNear + jitter * stepSize;

        float3 accumColor = 0;
        float accumAlpha = 0;

        for (int s = 0; s < maxSteps; s++)
        {
        if (t >= tFar) break; // condition 3: exited the bbox
        if (accumAlpha >= 0.995) break; // condition 2: reached max opacity

        float3 pos = ro + rd * t;
        float n = Perlin3D(pos * noiseFreq); // [-1,1] or [0,1] depending on your im
        float density = saturate(n) * densityScale;

        // Beer-Lambert so opacity is step-size independent
        float sampleAlpha = 1.0 - exp(-density * stepSize);
        float3 sampleColor = float3(1,1,1); // or shade by noise/gradient

        accumColor += (1.0 - accumAlpha) * sampleAlpha * sampleColor;
        accumAlpha += (1.0 - accumAlpha) * sampleAlpha;

        t += stepSize;
        }

        return float4(lerp(sceneColor.rgb, accumColor, accumAlpha), 1.0);
    }
}