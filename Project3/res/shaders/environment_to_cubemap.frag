#version 330 core

in vec3 localPosition;

uniform bool equirectangularMapAvailable;
uniform sampler2D equirectangularMap;
uniform vec4 backgroundColor;

out vec4 outColor;

const vec2 invAtan = vec2(0.1591, 0.3183); // (1/2pi, 1/pi)
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
	vec3 d = normalize(localPosition); // make sure to normalize localPosition
    vec2 uv = SampleSphericalMap(d);
    vec3 color = vec3(0.0);
	if (equirectangularMapAvailable)
	{
                color = min(vec3(10.0), texture(equirectangularMap, uv).rgb);
	}
	else
	{
	    // Angle-based viewing ray elevation in world space
		vec3 rayWorldspace = d;

		vec3 horizonWorldspace = rayWorldspace * vec3(1.0, 0.0, 1.0);
		float elevation = (1.0 - dot(rayWorldspace, horizonWorldspace)) * sign(rayWorldspace.y);

		// Colors
		vec3 bgColor = pow(backgroundColor.rgb, vec3(2.2)); // Linearize
		float bgIntensity = 0.1 * bgColor.r + 0.7 * bgColor.g + 0.2 * bgColor.b;
		float skyFactor = smoothstep(0.0, 1.0, elevation);
		float groundFactor = smoothstep(0.0, -0.0005, elevation);
		float horizonFactor = clamp(1.0 - max(skyFactor, groundFactor), 0.0, 1.0);
		vec3 horizonColor = horizonFactor * bgColor;
		vec3 groundColor  = groundFactor * vec3(bgIntensity) * 0.2;
		vec3 skyColor     = skyFactor * bgColor * 0.7;
		color = groundColor + skyColor + horizonColor;
	}
	
    outColor = vec4(color, 1.0);
}
