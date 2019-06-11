#version 400
#pragma optionNV(unroll none)
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec4		vertPosition[];
in vec3		vertNormal[];
in vec4		vertUp[];
in vec2		vertTexCoord0[];
in float	vertAmp[];

out vec4		vPosition;
out vec3		vNormal;
out vec4		vUp;
out vec2		vTexCoord0;
out float		vAmp;

uniform mat4	ciModelViewProjection;
uniform float	uTime;

vec3 GetNormal()
{
//   vec3 a = vec3(vertPosition[0]) - vec3(vertPosition[1]);
//   vec3 b = vec3(vertPosition[2]) - vec3(vertPosition[1]);
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 0.0;
    //vec3 direction = normal * ((sin(uTime * 0.5) + 1.1) / 2.0) * magnitude; 
	vec3 direction = normal * magnitude; 
    return position + vec4(direction, 0.0);
} 

void main()
{
	vec3 normal = GetNormal();

	for( int i = 0; i <= 3; i++ ) {
		vPosition = vertPosition[i];
		vNormal = vertNormal[i];
		//vNormal = normal;
		vUp = vertUp[i];
		vTexCoord0 = vertTexCoord0[i];
		vAmp = vertAmp[i];
		gl_Position = explode(gl_in[i].gl_Position, normal);
		//gl_Position = ciModelViewProjection * vertPosition[i];
		EmitVertex();
	}
	EndPrimitive();
}