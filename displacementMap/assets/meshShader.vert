#version 150

uniform mat4	ciModelView;
uniform mat4	ciModelViewProjection;
uniform mat3	ciNormalMatrix;
uniform sampler2D uTexDisplacement;
uniform float	uOffset;

in vec4			ciPosition;
in vec3			ciNormal;
in vec4			ciColor;
in vec2			ciTexCoord0;
in vec4			backColor;

out vec4		vertPosition;
out vec3		vertNormal;
out vec4		vertColor;
out vec4		vertColorBack;
out vec4        vertUp;
out vec3		viewDir;
out vec2		vertTexCoord0;

void main()
{
//	vertPosition = ciModelView * ciPosition;       
//	vertNormal = ciNormalMatrix * ciNormal;
//	vertColorBack = backColor;
//	vertColor = ciColor;
//	vertUp = ciModelView * vec4(0, 1, 0, 0);
//	viewDir = normalize((ciModelView * vec4(1)).xyz);
//	
//	// vertex shader must always pass projection space position
//	gl_Position = ciModelViewProjection * ciPosition;

// lookup displacement in map
	float displacement = texture( uTexDisplacement, ciTexCoord0.xy ).r;

	// now take the vertex and displace it along its normal
	vec4 displacedPosition = ciPosition;
	displacedPosition.xyz += ciNormal * displacement + vec3(0.0, uOffset, 0.0);

	// pass the surface normal and texture coordinate on to the fragment shader
	vertNormal = ciNormal;
	vertTexCoord0 = ciTexCoord0;

	// pass vertex on to the fragment shader
	gl_Position = ciModelViewProjection * displacedPosition;
}