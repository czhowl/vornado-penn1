#version 150

uniform mat4	ciModelView;
uniform mat4	ciModelViewProjection;
uniform mat3	ciNormalMatrix;
uniform sampler2D uTexDisplacement;

in vec4			ciPosition;
in vec3			ciNormal;
in vec2			ciTexCoord0;

out vec4		vertPosition;
out vec3		vertNormal;
out vec4        vertUp;
out vec2		vTexCoord0;

void main()
{
	float displacement = texture( uTexDisplacement, ciTexCoord0.xy ).r;

	vec4 displacedPosition = ciPosition;
	displacedPosition.xyz += ciNormal * displacement;

	vertPosition = displacedPosition;       
	vertNormal = ciNormalMatrix * ciNormal;
	vertUp = ciModelView * vec4(0, 1, 0, 0);

	vTexCoord0 = ciTexCoord0;
	
	// vertex shader must always pass projection space position
	gl_Position = ciModelViewProjection * displacedPosition;
}