HEADER
{
    DevShader = true;
    Description = "fBm simplex noise compute shader";
}
MODES
{
}
CS
{
    #include "system.fxc"
    #include "procedural.hlsl"
    #include "common/shared.hlsl"

    RWTexture2D<float4> OutputTexture < Attribute("OutputTexture"); >;

    int   ResolutionX  < Attribute("ResolutionX"); >;
    int   ResolutionY  < Attribute("ResolutionY"); >;
    int   Octaves      < Attribute("Octaves"); >;
    float Persistence  < Attribute("Persistence"); >;
    float Lacunarity   < Attribute("Lacunarity"); >;
    float Scale        < Attribute("Scale"); >;
    float Seed         < Attribute("Seed"); >;
    float WarpStrength < Attribute("WarpStrength"); >;

    float FBM( float2 p )
    {
        float value     = 0.0;
        float amplitude = 1.0;
        float frequency = 1.0;
        float maxValue  = 0.0;

        float2 uv = p / float2( ResolutionX, ResolutionY );

        for ( int i = 0; i < Octaves; i++ )
        {
            float2 coord = float2(
                uv.x * frequency * Scale + Seed * 0.1,
                uv.y * frequency * Scale + Seed * 0.1
            );

            value    += Simplex2D( coord ) * amplitude;
            maxValue += amplitude;
            amplitude *= Persistence;
            frequency *= Lacunarity;
        }

        return value / maxValue;
    }

    float WarpedFBM( float2 p )
    {
        if ( WarpStrength <= 0.0 ) return FBM( p );

        float2 warp = float2(
            FBM( p + float2( 1.7, 9.2 ) ),
            FBM( p + float2( 8.3, 2.8 ) )
        );

        return FBM( p + WarpStrength * warp );
    }

    [numthreads( 8, 8, 1 )]
    void MainCs( uint3 id : SV_DispatchThreadID )
    {
        if ( (int)id.x >= ResolutionX || (int)id.y >= ResolutionY ) return;

        float val = WarpedFBM( float2( id.x, id.y ) );

        val = saturate( val * 0.5 + 0.5 );

        OutputTexture[id.xy] = float4( val, val, val, 1.0 );
    }
}