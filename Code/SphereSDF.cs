using Sandbox;
using Sandbox.Rendering;

public sealed class SphereSDF : BasePostProcess<SphereSDF>
{
    [Property]
    public float Radius{ get; set; } = 1.0f;

    public override void Render()
	{

        Attributes.Set( "radius", Radius );

		var shader = Material.FromShader( "sphere_sdf.shader" );
        var blit = BlitMode.WithBackbuffer( shader, Stage.AfterPostProcess, 200, false );
		Blit( blit, "radius" );
	}
}