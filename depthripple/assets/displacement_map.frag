#version 150

#include "util.frag"

uniform sampler2D	uTex0;
uniform float		uTime;
uniform float		uAmplitude;

in vec2 vTexCoord0;

out vec4 oColor;

float wave( float period )
{
	return sin( period * 6.283185 );
}

// calculate displacement based on uv coordinate
float displace( vec2 uv )
{
	// large up and down movement
	float d = wave( (uv.x * 0.1) - uTime * 0.01 );
	// add a large wave from left to right
	d -= 1.2 * wave( (uv.x * 0.9) - uTime * 0.04 );
	// add diagonal waves from back to front
	d -= 0.25 * wave( ((uv.x + uv.y) * 2.2) - uTime * 0.05 );
	// add additional waves for increased complexity
	d += 0.25 * wave( (uv.y * 1.2) - uTime * 0.01 );
	d -= 0.15 * wave( ((uv.y + uv.x) * 2.8) - uTime * 0.09 );
	d += 0.15 * wave( ((uv.y - uv.x) * 1.9) - uTime * 0.08 );

	return d;
}

void main()
{
	vec4 ripple = texture2D(uTex0, vec2(vTexCoord0.x, vTexCoord0.y));
	// only sin waves
	float d = uAmplitude * displace( vTexCoord0.xy ) + ripple.r;
	// 4D simplex noise
	d = uAmplitude * snoise(vec4(vTexCoord0.x * 5, 1.0, uTime * 0.5, vTexCoord0.y * 5)) + ripple.r;
	// unknown pleasures
	//n = uAmplitude * clamp((1 - abs(vTexCoord0.y - 0.5) * 3), 0, 1) * snoise(vec4(vTexCoord0.x * 10, 1.0, uTime * 0.5, vTexCoord0.y * 5)) + ripple.r;
	// double 4D simplex noise - do not know the name of the pattern
	//d = uAmplitude * snoise(vec4(d * 0.5,0.0,0.0, 1.0)) + ripple.r;
	// double 4D simplex noise - do not know the name of the pattern
	d = d;
	oColor = vec4( d, d, d, 1.0 );
}