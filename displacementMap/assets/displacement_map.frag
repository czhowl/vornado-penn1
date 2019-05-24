#version 150

uniform float		uTime;
uniform sampler2D	uTexRipple;
uniform float		uAmplitude;


uniform sampler2D	uTexDisp;
uniform float		uMouse;
uniform float		uRippleAmplitude;
uniform float		uDumping;

in vec2 vTexCoord0;

out vec4 oColor;

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p){
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}

float cubicPulse( float c, float w, float x )
{
    x = abs(x - c);
    if( x>w ) return 0.0;
    x /= w;
    return pow(1.0 - x*x*(3.0-2.0*x), 5);
}
#define M_PI 3.14159265358979323846
float sinc( float x, float c, float k )
{
    float a = M_PI*(k*x-c);
    return sin(a)/a;
}

float wave( float period )
{
	return sin( period * 6.283185 );
}

// calculate displacement based on uv coordinate
float displace( vec2 uv)
{
	// large up and down movement
	float d = wave( (uv.x * 2.1) - uTime * 0.01 );
	// add a large wave from left to right
	d -= 1.2 * wave( (uv.x * 1.9) - uTime * 0.04 );
	// add diagonal waves from back to front
	d -= 0.25 * wave( ((uv.x + uv.y) * 2.2) - uTime * 0.05 );
	// add additional waves for increased complexity
	d += 0.25 * wave( (uv.y * 3.2) - uTime * 0.01 );
	d -= 0.15 * wave( ((uv.y + uv.x) * 2.8) - uTime * 0.09 );
	d += 0.15 * wave( ((uv.y - uv.x) * 1.9) - uTime * 0.4 );
	//d += noise(vec2(uTime * 0.01, uv.x * 10)) * 10;
	d += noise(vec2(uTime * 0.01 - 100, uv.y * 10));

	return d;
}

void main()
{
	float ripple = texture( uTexRipple, vTexCoord0.xy ).r;
	float d = uAmplitude * displace( vTexCoord0.xy);
	d += ripple;


	float pos = texture( uTexDisp, vTexCoord0.xy ).r;
	float p = vTexCoord0.x -uMouse;
	float a =  M_PI*( 100  * p) * uRippleAmplitude;
	float y = sin(a) / a * 5;
	float target = pow(y, 2) * (cos(sin(uTime * 2)) *sin(uDumping - pow(abs(p), 0.4))) * uRippleAmplitude;
	pos += (target - pos) / 8;
	d += pos;

	oColor = vec4( d, pos, d, 1.0 );
}