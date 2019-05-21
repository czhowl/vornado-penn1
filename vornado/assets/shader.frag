#version 150

uniform vec2	uResolution;
uniform vec2	uMouse;
uniform float	uTime;
uniform float	uNoise;

out vec4		oColor;

//#define samples 1
//
//#define numballs 3

//----------------------------------------------------------------

//vec4 blobs[numballs];
//
//float sdMetaBalls( vec3 pos )
//{
//	float m = 0.0;
//	float p = 0.0;
//	float dmin = 1e20;
//		
//	float h = 1.0; // track Lipschitz constant
//	
//	for( int i=0; i<numballs; i++ )
//	{
//		// bounding sphere for ball
//        float db = length( blobs[i].xyz - pos );
//        if( db < blobs[i].w )
//    	{
//    		float x = db/blobs[i].w;
//    		p += 1.0 - x*x*x*(x*(x*6.0-15.0)+10.0);
//	    	m += 1.0;
//    		h = max( h, 0.5333*blobs[i].w );
//	    }
//	    else // bouncing sphere distance
//	    {
//    		dmin = min( dmin, db - blobs[i].w );
//    	}
//	}
//    float d = dmin + 0.1;
//	
//	if( m>0.5 )
//	{
//		float th = 0.2;
//		d = h*(th-p);
//	}
//	
//	return d;
//}
//
//vec3 norMetaBalls( vec3 pos )
//{
//	vec3 nor = vec3( 0.0, 0.0001, 0.0 );
//		
//	for( int i=0; i<numballs; i++ )
//	{
//        float db = length( blobs[i].xyz - pos );
//		float x = clamp( db/blobs[i].w, 0.0, 1.0 );
//		float p = x*x*(30.0*x*x - 60.0*x + 30.0);
//		nor += normalize( pos - blobs[i].xyz ) * p / blobs[i].w;
//	}
//	
//	return normalize( nor );
//}
//
//float map( in vec3 p )
//{
//	return sdMetaBalls( p );
//}
//
//const float precis = 0.01;
//
//vec2 intersect( in vec3 ro, in vec3 rd )
//{
//	float maxd = 10.0;
//    float h = precis*2.0;
//    float t = 0.0;
//    float m = 1.0;
//    for( int i=0; i<75; i++ )
//    {
//        if( h<precis||t>maxd ) continue;//break;
//        t += h;
//	    h = map( ro+rd*t );
//    }
//
//    if( t>maxd ) m=-1.0;
//    return vec2( t, m );
//}
//
//vec3 calcNormal( in vec3 pos )
//{
//	return norMetaBalls( pos );
//}
//
//float plot(vec2 st, float pct){
//  return  smoothstep( pct-0.02, pct, st.y) -
//          smoothstep( pct, pct+0.02, st.y);
//}

float cubicPulse( float c, float w, float x )
{
    x = abs(x - c);
    if( x>w ) return 0.0;
    x /= w;
    return pow(1.0 - x*x*(3.0-2.0*x), 5);
}

float pcurve( float x, float a, float b )
{
    float k = pow(a+b,a+b) / (pow(a,a)*pow(b,b));
    return k * pow( x, a ) * pow( 1.0-x, b );
}

float parabola( float x, float k )
{
    return pow( 4.0*x*(1.0-x), k );
}

#define NUM_OCTAVES 5
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

void main() {
	vec2 st = gl_FragCoord.xy/uResolution;

//	float msamples = sqrt(float(samples));
//	vec3 tot = vec3(0.0);
//	for( int a=0; a<samples; a++ ){
//		vec2  poff = vec2( mod(float(a),msamples), floor(float(a)/msamples) )/msamples;
//		float toff = 0.0;
//	}
	// output pixel color
	float r, g, b;
	//float t = sin(uTime+3) * 8;
	float t = uTime;
	r = sin(exp(cos(t * 0.2 + st.y))* 2) + 1.2;
	//g = sin(cos(t * 0.3 + st.y*0.1)) + 0.8;
	g = noise(vec2(st.y * 0.8 + t * 0.2, st.x * 10)) * 0.3;
	b = sin(tan(sin(t*0.1 + st.y*0.1 + 0.2))* 2) + 1.2 + sin(cos(t * 0.3));

	float n = noise(vec2(t * 0.8, st.x * 30));
	float n2 = noise(vec2(st.x * 30, t * 0.6 + 1));
	float ntot = (n * pow(1 - st.y, 1) + n2 * pow(st.y, 1)) * 0.2;
	float y = cubicPulse(uMouse.x / uResolution.x, 0.05, st.x) * pow(1 - st.y, 1.1) + ntot;
	//float y = parabola(st.x - uMouse.x / uResolution.x + 0.5, 50) + ntot;
	

    vec3 color = vec3(r, g, b);

	color *= clamp(y, 0.05, 1);
	//color.x *=  clamp(y, 0.05, 1);

    //float pct = plot(st,y);
    //color = (1.0-pct)*color+pct*vec3(0.0,1.0,0.0);
//	// gamma
	color = pow( clamp(color,0.0,1.0), vec3(0.45) );
//
//	// vigneting
    color *= 0.5 + 0.5*pow( 16.0*st.x*st.y*(1.0-st.x)*(1.0-st.y), 0.15 );


	oColor = vec4(color,1.0);
}