#version 150

uniform float		uTime;
uniform float		uMouse;
uniform int			uRipple;
uniform sampler2D	uTexDisplacement;

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

void main()
{
	float displacement = texture( uTexDisplacement, vTexCoord0.xy ).r;
	float target = texture( uTexDisplacement, vTexCoord0.xy ).g;

	float p = uMouse;

	float y = cubicPulse(p, 0.08, vTexCoord0.x) * 300;

	float a = M_PI*( 50  * (vTexCoord0.x - p));
	y = sin(a) / a * 10;
	
	if(uRipple == 1){
		target = clamp(pow(y, 2), -10.0f, 10.0f);
	}
//	float s = 1.0;
//	if(displacement > target){
//		s = -1.0;
//	}
	
	displacement += (target - displacement) / 10;


	target *= 0.98;
	//target -= target*target * 0.1;
	oColor = vec4( displacement, target, displacement, 1.0 );
}