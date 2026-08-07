HEADER
{
    Description = "Example post processing shader";
}

MODES
{
    Default();
    VrForward();
}

FEATURES
{
}

COMMON
{
    #include "system.fxc"
    #include "common.fxc"
    #include "sbox_shared.fxc"

    struct VertexInput
    {
        float3 vPositionOs : POSITION < Semantic( PosXyz ); >;
        float2 vTexCoord : TEXCOORD0 < Semantic( LowPrecisionUv ); >;
    };

    struct PixelInput
    {
        float2 vTexCoord : TEXCOORD0;

        // VS only
        #if ( PROGRAM == VFX_PROGRAM_VS )
            float4 vPositionPs        : SV_Position;
        #endif

        // PS only
        #if ( ( PROGRAM == VFX_PROGRAM_PS ) )
            float4 vPositionSs        : SV_ScreenPosition;
        #endif
        //float4 vPositionPs        : SV_Position;
        //float4 vPositionSs        : SV_ScreenPosition;
    };
}

VS
{
    

    float CalculateProjectionDepthFromViewDepth( float flViewDepth )
    {
        float flZScale = g_vInvProjRow3.z;
        float flZTran = g_vInvProjRow3.w;
        return ( 1.0 / flViewDepth - flZTran ) / flZScale;
    }

    PixelInput MainVs( VertexInput i )
    {
           PixelInput o;
        o.vPositionPs = float4(i.vPositionOs.xyz, 1.0f);
        o.vPositionPs.z = RemapValClamped( CalculateProjectionDepthFromViewDepth( 500 ), 0.0, 1.0,  g_flViewportMinZ, g_flViewportMaxZ );
        o.vTexCoord = i.vTexCoord;
        return o;
    }
}

PS
{
    #include "postprocess/common.hlsl"
    #include "msaa_offsets.fxc"
    #include "common/msaa.hlsl"


    RenderState( DepthWriteEnable, true );
    RenderState( DepthEnable, true );
    RenderState( DepthFunc, ALWAYS );

    CreateTexture2D( g_tColorBuffer ) < Attribute( "ColorBuffer" );      SrgbRead( true ); Filter( MIN_MAG_LINEAR_MIP_POINT ); AddressU( MIRROR ); AddressV( MIRROR ); >;
    CreateTexture2D( g_tDepthBuffer ) < Attribute( "DepthTexture" );     SrgbRead( false ); Filter( MIN_MAG_MIP_POINT ); >;

    float FetchDepth( float2 vTexCoord )
    {
        float2 vProjCoord = ( vTexCoord * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
        float3 vCameraRayWs = mul( g_matProjectionToWorld, float4( vProjCoord, 1.0f, 1 ) ).xyz;

        float flProjectedDepth = Tex2D( g_tDepthBuffer, vTexCoord).r;
        flProjectedDepth = RemapValClamped( flProjectedDepth,
        g_flViewportMinZ, g_flViewportMaxZ, 0, 1);

        float flZScale = g_vInvProjRow3.z;
        float flZTran = g_vInvProjRow3.w;

        float flDepthRelativeToRayLength = 1.0 / ( ( flProjectedDepth * flZScale + flZTran ) );

        return flProjectedDepth;
    }

    struct PixelOutput
    {
        float4 vColor : SV_Target0;
        float flDepth : SV_DEPTH;
    };

    PixelOutput MainPs( PixelInput i ) : SV_Target0
    {
        PixelOutput o;
        float objectDepth = i.vPositionSs.z;


        float2 vScreenUv = CalculateViewportUvFromInvSize( i.vPositionSs.xy, 1.0f / g_vRenderTargetSize );

        float depth = Tex2DLevel( g_tDepthBuffer, vScreenUv.xy, 0.0 ).r;
        o.flDepth = depth;

        // float3 flProjectedDepth = Tex2D( g_tDepthBuffer, screenUv);
        depth = RemapValClamped( depth, g_flViewportMinZ, g_flViewportMaxZ, 0.0, 1.0);

        o.vColor.rgb = float3(depth, depth, depth);
        o.vColor.a = 1.0f;
        
        // Invert the color and write it to our output
        return o;
    }
}