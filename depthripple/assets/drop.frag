#version 150

uniform sampler2D uTex0;

uniform vec2 uCenter;
uniform float uRadius;
uniform float uStrength;

in vec2 vTexCoord0;

out vec4 oColor;

const float PI = 3.141592653589793;

void main(void)
{
	// calculate first order centered finite difference (y-direction)
	vec4 drop = texture2D(uTex0, vTexCoord0);
	/* add the drop to the height */
    float d = max(0.0, 1.0 - length(vec2(uCenter.x, uCenter.y) - vec2(vTexCoord0.x, vTexCoord0.y)) / uRadius);
    d = 0.5 - cos(d * PI) * 0.5;
    drop.r += d * uStrength;

	oColor = drop;
}