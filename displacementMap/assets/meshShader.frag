#version 150

uniform sampler2D	uTexNormal;
uniform mat3 ciNormalMatrix;
uniform float	uTime;

in vec2		vertTexCoord0;
in vec4		vertPosition;
in vec3		vertNormal;
in vec4		vertColor;
in vec4		vertColorBack;
in vec4     vertUp;
in vec3		viewDir;

out vec4	oColor;

void main()
{	
//	const vec4  kAmbient = vec4(0.1, 0.1, 0.1, 1);
//	const vec4	kSpecular = vec4(0.7, 0.7, 0.4, 1);
//	const float kShininess = 50.0;
//	
//	vec3 N = normalize( vertNormal );
//	N = vec3(1);
//	vec3 L = normalize( -vertPosition.xyz );   
//	vec3 E = normalize( -vertPosition.xyz ); 
//	vec3 H = normalize( L + E ); 
//	vec4 diffuse = vec4(1.0);
//	// ambient term
//	vec4 ambient = kAmbient;
//	if (gl_FrontFacing){
//		diffuse = vertColor;
//	}else{
//		diffuse = vertColorBack;
//		//diffuse = vertColor;
//		//N *= -0.7;
//	}
//	// diffuse term
//
//	diffuse *= max( dot( N, L ), 0.0 );    
//
//	// specular term
//	vec4 specular = kSpecular; 
//	specular *= pow( max( dot( N, H ), 0.0 ), kShininess );
//
//	// reflection term (fake ground reflection)
//	float f = max( 0.0, dot( -N, normalize( vertUp.xyz ) ) );
//	vec4 reflection = vec4(f, f, f, 1.0) * kSpecular;
//
//	// final color 
//	oColor = vec4((ambient + reflection + diffuse).xyz, 1.0);
	// retrieve normal from texture
	vec3 Nmap = texture( uTexNormal, vertTexCoord0.xy ).rgb;

	// modify it with the original surface normal
	const vec3 Ndirection = vec3(0.0, 1.0, 0.0);	// see: normal_map.frag (y-direction)
	vec3 Nfinal = ciNormalMatrix * normalize( vertNormal + Nmap - Ndirection );

	// perform some falloff magic
	float falloff = sin( max( dot( Nfinal, vec3(0.15, 0.15, 1.0) ), 0.0) * 2.25);	
	float alpha = 0.01 + 0.3 * pow( falloff, 30.0 );
	float r =  Nfinal.x * 5;
	float g = Nfinal.z;
	float b = Nfinal.y;

	// output color
	oColor = vec4( 1,1,1 , alpha );
}