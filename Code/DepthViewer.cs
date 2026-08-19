using Sandbox;
using Sandbox.Rendering;

public sealed class DepthViewer : BasePostProcess<DepthViewer>
{
    public override void Render()
	{
		var shader = Material.FromShader( "depth_viewer.shader" );
        var blit = BlitMode.WithBackbuffer( shader, Stage.AfterPostProcess, 200, false );
		Blit( blit, "depth_viewer" );
	}
}