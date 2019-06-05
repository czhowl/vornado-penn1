#version 150
#extension GL_NV_shadow_samplers_cube : enable

uniform sampler2D	uTexNormal;
uniform samplerCube	uSkyBox;
uniform sampler2D	uWaterBottom;

uniform mat3 ciNormalMatrix;

uniform vec3 uLightDir;
uniform vec3 uEyePos;

in vec4		vertPosition;
in vec3		vertNormal;
in vec4     vertUp;
in vec2		vTexCoord0;

out vec4	fragColor;

float poolHeight = 10.0;
float sunSize = 500.0;
const float IOR_AIR = 1.0;
const float IOR_WATER = 1.333;

vec2 intersectCube(vec3 origin, vec3 ray, vec3 cubeMin, vec3 cubeMax) {
    vec3 tMin = (cubeMin - origin) / ray;
    vec3 tMax = (cubeMax - origin) / ray;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}

vec3 getWallColor(vec3 point) {
    float scale = 0.5;
    
    vec3 wallColor;
    vec3 normal;
    wallColor = texture2D(uWaterBottom, point.xz * 0.1 + 0.5).rgb;
	//wallColor = texture2D(uWaterBottom, vTexCoord0.xy).rgb;
    //wallColor = vec3(0.5);
    normal = vec3(0.0, 1.0, 0.0);
    
    scale /= length(point); /* pool ambient occlusion */
    
    /* caustics */
    vec3 refractedLight = -refract(-uLightDir, vec3(0.0, 1.0, 0.0), IOR_AIR / IOR_WATER);
    float diffuse = max(0.0, dot(refractedLight, normal));
    
    //vec4 caustic = texture2D(causticTex, 0.75 * (point.xz - point.y * refractedLight.xz / refractedLight.y) * 0.25 + 0.5);
    //scale += diffuse * caustic.r * 2.0 * caustic.g;
    return wallColor * scale;
    /*return vec3(1.0) * scale;*/
}


vec3 getSurfaceRayColor(vec3 origin, vec3 ray, vec3 waterColor, vec3 suncolor, float sunsize) {
    vec3 color;
    vec2 t = intersectCube(origin, ray, vec3(-200.0, -poolHeight, -200.0), vec3(200.0, 1.0, 200.0));
    vec3 hit = origin + ray * t.y;
    if (hit.y < 0.9) {
		color = getWallColor(hit);
		//color = vec3(0.1,0.5,0.9);
		//color = texture2D(uWaterBottom, vTexCoord0.xy).rgb;
    } else {
		color = textureCube(uSkyBox, ray).rgb;
		color += vec3(pow(max(0.0, dot(uLightDir, ray)), sunsize)) * suncolor;
	}
    if (ray.y < 0.0) color *= waterColor;
    return color;
}

void main()
{	
	// retrieve normal from texture
	vec3 Nmap = texture( uTexNormal, vTexCoord0.xy ).rgb;
	// modify it with the original surface normal
	const vec3 Ndirection = vec3(0.0, 1.0, 0.0);	// see: normal_map.frag (y-direction)
	vec3 Nfinal = ciNormalMatrix * normalize( vertNormal + Nmap - Ndirection );
	
	vec3 waterColor = vec3(0.1, 0.2, 0.5);
	
	vec3 incomingRay = normalize(vertPosition.xyz - uEyePos);

	vec3 reflectedRay = reflect(incomingRay, Nfinal);
	vec3 refractedRay = refract(incomingRay, Nfinal, IOR_AIR / IOR_WATER);
	//float fresnel = mix(0.25, 1.0, pow(1.0 - dot(Nfinal, -incomingRay), 2.0));

	//waterColor = textureCube(uSkyBox, reflectedRay).rgb;
	vec3 sunColor = vec3(0.8, 0.6, 0.9) * 1.2;
	//vec3 reflectedColor = vec3(pow(max(0.0, dot(uLightDir, reflectedRay)), sunSize)) * sunColor;
	vec3 reflectedColor = getSurfaceRayColor(vertPosition.xyz, reflectedRay, waterColor, sunColor, sunSize);
	vec3 refractedColor = getSurfaceRayColor(vertPosition.xyz, refractedRay, waterColor, sunColor, sunSize);
	// final color 
	fragColor = vec4(mix(refractedColor, reflectedColor, 0.5), 1.0);
	//fragColor = vec4(refractedColor, 1.0);
}