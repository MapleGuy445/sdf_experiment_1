using Sandbox;
using Sandbox.Utility;

public class NoiseSampler
{
    public int ResolutionX  { get; set; } = 128;
    public int ResolutionY  { get; set; } = 128;
    public int Octaves      { get; set; } = 6;
    public float Persistence  { get; set; } = 0.5f;
    public float Lacunarity   { get; set; } = 2.0f;
    public float Scale        { get; set; } = 3.0f;
    public float Seed         { get; set; } = 0f;
    public float WarpStrength { get; set; } = 0f;

    private ComputeShader computeShader = new ComputeShader( "noise_shader_compute" );

    // Returns a render target texture filled with noise.
    // Caller is responsible for disposing it when done.
    public Texture Dispatch( float scale, float seed, int octaves, float persistence, float lacunarity, float warp )
    {
        var texture = Texture.CreateRenderTarget()
            .WithSize( ResolutionX, ResolutionY )
            .WithFormat( ImageFormat.RGBA8888 )
            .Create();

        computeShader.Attributes.Set( "OutputTexture", texture );
        computeShader.Attributes.Set( "Scale",         scale );
        computeShader.Attributes.Set( "Seed",          seed );
        computeShader.Attributes.Set( "Octaves",       octaves );
        computeShader.Attributes.Set( "Persistence",   persistence );
        computeShader.Attributes.Set( "Lacunarity",    lacunarity );
        computeShader.Attributes.Set( "WarpStrength",  warp );
        computeShader.Attributes.Set( "ResolutionX",   ResolutionX );
        computeShader.Attributes.Set( "ResolutionY",   ResolutionY );

        int groupsX = ( ResolutionX + 7 ) / 8;
        int groupsY = ( ResolutionY + 7 ) / 8;
        computeShader.Dispatch( groupsX, groupsY, 1 );

        return texture;
    }

    // Convenience: dispatch with current settings as elevation map
    public Texture DispatchElevation() =>
        Dispatch( Scale, Seed, Octaves, Persistence, Lacunarity, WarpStrength );

    // Convenience: dispatch temperature map (very large scale, decorrelated seed)
    public Texture DispatchTemperature() =>
        Dispatch( Scale * 0.15f, Seed + 1000f, 2, 0.5f, 2.0f, 0.0f );

    // Convenience: dispatch humidity map (medium scale, decorrelated seed)
    public Texture DispatchHumidity() =>
        Dispatch( Scale * 0.25f, Seed + 2000f, 3, 0.5f, 2.0f, 0.3f );

    // CPU sampling — useful for reading individual values without the GPU round trip,
    // e.g. when you need a height value at a specific world position for physics.
    public float Sample( float x, float y )
    {
        float value = 0f, amplitude = 1f, frequency = 1f, maxValue = 0f;
        float uvX = x / ResolutionX;
        float uvY = y / ResolutionY;

        for ( int i = 0; i < Octaves; i++ )
        {
            value    += Noise.Simplex( uvX * frequency * Scale + Seed * 0.1f,
                                       uvY * frequency * Scale + Seed * 0.1f ) * amplitude;
            maxValue += amplitude;
            amplitude *= Persistence;
            frequency *= Lacunarity;
        }

        return value / maxValue;
    }
}