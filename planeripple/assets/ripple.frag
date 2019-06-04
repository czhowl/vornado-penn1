#version 150

uniform sampler2D uTex0;
uniform vec2 uDelta;

in vec2 vTexCoord0;

out vec4 oColor;

const float PI = 3.141592653589793;

//void main(void)
//{
//	/* get vertex info */
//    vec4 drop = texture2D(uTex0, vTexCoord0.xy);
//    
//    /* calculate average neighbor height */
//    vec2 dx = vec2(uDelta.x, 0.0);
//    vec2 dy = vec2(0.0, uDelta.y);
//    float average = (
//    texture2D(uTex0, vTexCoord0.xy - dx).r +
//    texture2D(uTex0, vTexCoord0.xy - dy).r +
//    texture2D(uTex0, vTexCoord0.xy + dx).r +
//    texture2D(uTex0, vTexCoord0.xy + dy).r
//    ) * 0.25;
//      
//    /* change the velocity to move toward the average */
//    drop.g += (average - drop.r) * 2.0;
//      
//    /* attenuate the velocity a little so waves do not last forever */
//    drop.g *= 0.99;
//      
//    /* move the vertex along the velocity */
//    drop.r += drop.g;
//
//	drop.r *= 0.99;
//
//	oColor = drop;
//
//	//if(vTexCoord0.y < 0.5 && vTexCoord0.x < 0.5 && vTexCoord0.y > 0.45 && vTexCoord0.x > 0.45) oColor = vec4(0.1);
//}

void main(void)
{
	/* get vertex info */
    vec4 drop = texture2D(uTex0, vTexCoord0.xy);
    
    /* calculate average neighbor height */
    vec2 dx = vec2(uDelta.x, 0.0);
    vec2 dy = vec2(0.0, uDelta.y);
    float average = (
    texture2D(uTex0, vTexCoord0.xy - dx).r +
    texture2D(uTex0, vTexCoord0.xy - dy).r +
    texture2D(uTex0, vTexCoord0.xy + dx).r +
    texture2D(uTex0, vTexCoord0.xy + dy).r
    ) * 0.25;
      
    /* change the velocity to move toward the average */
    drop.g += (average - drop.r) * 2.0;
      
    /* attenuate the velocity a little so waves do not last forever */
    drop.g *= 0.995;
      
    /* move the vertex along the velocity */
    drop.r += drop.g;

	drop.r *= 0.995;

	oColor = drop;

	//if(vTexCoord0.y < 0.5 && vTexCoord0.x < 0.5 && vTexCoord0.y > 0.45 && vTexCoord0.x > 0.45) oColor = vec4(0.1);
}