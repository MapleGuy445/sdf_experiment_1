using Sandbox;

[Title( "SDF Sphere Controller" )]
[Category( "SDF" )]
[Icon( "blur_circular" )]
public sealed class SdfSphereController : Component, Component.ExecuteInEditor
{
	[Property] public float Radius { get; set; } = 20f;

	[Property] public GameObject SphereOrigin { get; set; }

	[Property] public Renderer TargetRenderer { get; set; }

	protected override void OnStart()
	{
		if ( TargetRenderer is null )
		{
			TargetRenderer = GetComponent<Renderer>();
		}
	}

	protected override void OnUpdate()
	{
		if ( TargetRenderer is null )
			return;

		Vector3 originWs = SphereOrigin is not null
			? SphereOrigin.Transform.Position
			: Transform.Position;

		TargetRenderer.Attributes.Set( "radius", Radius );
		TargetRenderer.Attributes.Set( "sphereOrigin", originWs );
	}
}