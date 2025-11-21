#version 330 core

uniform sampler2D depthTex;
uniform sampler2D normTex;
uniform sampler2D shadowTex;

uniform mat4 shadowMatrix;
uniform vec2 pixelSize;
uniform vec3 cameraPos;

uniform float lightRadius;
uniform vec3 lightPos;
uniform vec4 lightColour;
uniform mat4 inverseProjView;

out vec4 diffuseOutput;
out vec4 specularOutput;

void main(void){
	vec2 texCoord = vec2 (gl_FragCoord.xy * pixelSize);
	float depth = texture (depthTex, texCoord.xy).r ;
	vec3 ndcPos = vec3 (texCoord, depth) * 2.0 - 1.0;
	vec4 invClipPos = inverseProjView * vec4 ( ndcPos, 1.0);
	vec3 worldPos = invClipPos.xyz / invClipPos.w ;

	float dist = length(lightPos - worldPos);
	float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);

	if(atten == 0.0){
		discard;
	}

	vec3 normal = normalize(texture(normTex, texCoord.xy).xyz * 2.0 - 1.0);
	vec3 incident = normalize(lightPos - worldPos);
	vec3 viewDir = normalize(cameraPos - worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	float lambert = clamp(dot(incident, normal), 0.0, 1.0);
	float rFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
	float specFactor = pow(rFactor, 60.0);
	vec3 attenuated = lightColour.xyz * atten;

	vec4 lightClip = shadowMatrix * vec4(worldPos, 1.0);
	
	lightClip /= lightClip.w;
	
	vec2 shadowUV = lightClip.xy * 0.5 + 0.5;
	
	float lightSpaceDepth = lightClip.z * 0.5 + 0.5;
	
	float bias = 0.003;
	
	float shadow = 1.0;
	if(shadowUV.x >= 0.0 && shadowUV.x <= 1.0 && shadowUV.y >= 0.0 && shadowUV.y <= 1.0) {
		float shadowMapDepth = texture(shadowTex, shadowUV).r;
		
		shadow = (shadowMapDepth + bias < lightSpaceDepth) ? 0.0 : 1.0;
	}
	diffuseOutput = vec4(attenuated * lambert* shadow, 1.0);
	specularOutput = vec4(attenuated * specFactor * 0.33 *shadow, 1.0);
}