#version 150
#extension GL_NV_shadow_samplers_cube : enable

uniform sampler2D	uTexNormal;
uniform samplerCube	uSkyBox;

uniform mat3 ciNormalMatrix;

uniform vec3 uLightDir;
uniform vec3 uEyePos;
uniform float uAmplitude;

in vec4		vertPosition;
in vec3		vertNormal;
in vec4     vertUp;
in vec2		vTexCoord0;
in float		vAmp;

out vec4	fragColor;

void main()
{	
	// retrieve normal from texture
	vec3 Nmap = texture( uTexNormal, vTexCoord0.xy ).rgb;
	// modify it with the original surfasce normal
	const vec3 Ndirection = vec3(0.0, 1.0, 0.0);	// see: normal_map.frag (y-direction)
	vec3 Nfinal = ciNormalMatrix * normalize( vertNormal + Nmap - Ndirection );

	float falloff = sin( max( dot( Nfinal, vec3(0.0, -1.0, 0.0) ), 0.0) * 2.25);	
	float alpha = 0.5 * pow( falloff, 30.0 );

	vec3 color = vec3(vAmp / (uAmplitude + 2.0) / 4 + 0.2);

	// graphic
	if(vAmp< uAmplitude / 2 + 0.1 && vAmp > uAmplitude / 2 - 0.1) color = vec3(0.0f);
	else if(vAmp > uAmplitude - 0.1) color = vec3(1.0f);
	else if(vAmp < -uAmplitude + 0.1) color = vec3(0.0f);

	color = vec3(mod(vAmp, 0.1f) * 5.0);

	vec3 reflectedRay = reflect(normalize(vertPosition.xyz - uEyePos), Nfinal);
//	color = textureCube(uSkyBox, reflectedRay).rgb;
	vec3 sunColor = vec3(1.0);
	//color += vec3(pow(max(0.0, dot(uLightDir, reflectedRay)), 50)) * sunColor;
	// final color 
	fragColor = vec4(color, 1.0);
	// plastic 
	//fragColor = vec4(alpha);
}