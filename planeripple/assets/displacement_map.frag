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
//	d = uAmplitude * snoise(vec4(vTexCoord0.x * 10, 1.0, uTime * 0.5, vTexCoord0.y * 2)) + ripple.r;

	// unknown pleasures
//	d = uAmplitude * clamp((1 - abs(vTexCoord0.y - 0.5) * 3), 0, 1) * snoise(vec4(vTexCoord0.x * 10, 1.0, uTime * 0.5, vTexCoord0.y * 5)) + ripple.r;
	
	// double 4D simplex noise - do not know the name of the pattern
//	d = uAmplitude * snoise(vec4(d * 0.5,0.0,0.0, 1.0)) + ripple.r;
	
	// double 4D simplex noise - do not know the name of the pattern
//	d = uAmplitude * snoise(vec4(d, 1.0, uTime * 0.5, 5.0)) + ripple.r;
	
	// larger noise noise + smaller noise = fur effect, looks good with plastic mesh shader
//	d = (uAmplitude + 10) * (snoise(vec4(vTexCoord0.x * 5, 1.0, uTime * 0.2, vTexCoord0.y))) + ripple.r;
//	d += (0 - d / uAmplitude) * (snoise(vec4(vTexCoord0.x * 5000, uTime * 1,1.0, vTexCoord0.y * 1000)));
	
	// Ocean / Clouds, with metal mesh shader will have the aluminium folio effect
	d = 0;
	for(float i = 0.0; i < 10.0; i += 1.0){
		d += 1 / pow(2.0, i) * (uAmplitude + 10) * (snoise(vec4(vTexCoord0.x * 5 * pow(2.0, i), 1.0, uTime * 0.2, vTexCoord0.y * pow(2.0, i))));
	}
	d += ripple.r;
	
	// more dramatic
//	d = 0;
//	for(float i = 0.0; i < 10.0; i += 1.0){
//		d += 1 / pow(2.0, i) * (uAmplitude + 10) * (snoise(vec4(vTexCoord0.x * 5 * pow(2.0, i), 1.0, uTime * 0.2, vTexCoord0.y * pow(2.0, i))));
//	}
//	d = d*d + ripple.r;
	
	// webs
//	d = 0;
//	for(float i = 0.0; i < 6.0; i += 1.0){
//		d += abs(1 / pow(2.0, i) * (uAmplitude + 10) * (snoise(vec4(vTexCoord0.x * 5 * pow(2.0, i), 1.0, uTime * 0.2, vTexCoord0.y * pow(2.0, i)))));
//	}
//	d = abs(d);     // create creases
////    d = pow(d, 2);
//	d = 5 - d; // invert so creases are at top
//	d += ripple.r;

//	d = (snoise(vec4(vTexCoord0.x * 10, 1.0, uTime * 0.02, vTexCoord0.y * 2)));
//	//d = snoise(vec4(d + snoise(vec4(d))));
//	d = (uAmplitude) * snoise(vec4(d + snoise(vec4(d))));
//	d += ripple.r;

	oColor = vec4( d, d, d, 1.0 );
}