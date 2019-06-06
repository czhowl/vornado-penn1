#version 400

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
    float d = max(0.0, 1.0 - length(vec2(uCenter.x * 2.5, uCenter.y / 2.0) - vec2(vTexCoord0.x * 2.5, vTexCoord0.y / 2.0)) / uRadius);
    d = 0.5 - cos(d * PI) * 0.5;
    drop.r += d * uStrength;

	oColor = drop;
}