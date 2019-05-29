// adapted from: http://www.tartiflop.com/disp2norm/srcview/index.html
// note: I have omitted the second and third order processing, because this
//       would require branching, which would stall the GPU needlessly.
//       You will notice incorrect normals at the edges of the normal map.

#version 150

uniform sampler2D uTex;

in vec2 vTexCoord0;

out vec4 oColor;

void main(void)
{
	float r = texture( uTex, vTexCoord0.xy ).r;
	float g = texture( uTex, vTexCoord0.xy ).g;
	float b = texture( uTex, vTexCoord0.xy ).b;
	oColor = vec4( r, g,b, 1.0 );
}