#version 150

uniform float		uTime;
uniform float		uMouse;
uniform float		uAmplitude;
uniform float[35]	uDepth;
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

//void main()
//{
//	float displacement = texture( uTexDisplacement, vTexCoord0.xy ).r;
//	float dumping1 = texture( uTexDisplacement, vTexCoord0.xy ).g;
//
//
//	float p = vTexCoord0.x -uMouse;
//	if(uRipple == 1){
//		dumping1 += uAmplitude * 0.3;
//	}
//	dumping1 = clamp(dumping1, 0, 20);
//
//	float y = 0;// = cubicPulse(p, 0.08, vTexCoord0.x) * 300;
//
//	float a = ( 100  * p);
//	y = exp(-pow(a, 2)) * 5;
//
//	float target = clamp(pow(y, 2) * (cos(sin(uTime * 2)) *sin(dumping1 - pow(abs(p), 0.4))) * uAmplitude, -10.0f, 10.0f);
//	dumping1 *= 0.99;
//
//	displacement += (target - displacement) / 8;
//
//	oColor = vec4( displacement, dumping1, 0.0, 1.0 );
//}
//
void main()
{
	float displacement = texture( uTexDisplacement, vTexCoord0.xy ).r;
	
	
	
	
	
	float p = vTexCoord0.x - uMouse;
	float target =  texture( uTexDisplacement, vTexCoord0.xy ).g;

	int i = 0;
	for (int y = 40; y < 480; y += 100) {
		for (int x = 20; x < 640; x += 100) {
			if(uDepth[i] == 1.0){
				target += exp(-pow((vTexCoord0.x - x / 640.0) * 20, 2)) * 10;
			}
			i++;
		}
	}


//	if(uRipple == 1){
//		target += exp(-pow(p * 20, 2)) * 10;
//		//dumping += uAmplitude * 0.3;
//	}

	//dumping = clamp(dumping, 0, 20);
	
	vec2 e = vec2( 50.0 / 256.0 , 0);
	float d1 = texture( uTexDisplacement, vTexCoord0.xy - e.xy ).r;
	float d2 = texture( uTexDisplacement, vTexCoord0.xy + e.xy ).r;

	target += (d2 + d1) * 0.1 - (displacement - 0.5) * 0.4;

	float a = ( 100  * p);
	float y = exp(-pow(a, 2)) * 5;
	//target += clamp(pow(y, 2) * (cos(sin(uTime * 2)) *sin(dumping - pow(abs(p), 0.4))) * uAmplitude, -10.0f, 10.0f);
	//dumping *= 0.99;
	//displacement *= 0.8;
//	displacement = 0.0;
//	target = 0.0;
	//target = clamp(target, -50.0, 50.0);
	displacement += (target - displacement) / 50;
	target *= 0.998;
	oColor = vec4( displacement, target, 1.0, 1.0 );
}
