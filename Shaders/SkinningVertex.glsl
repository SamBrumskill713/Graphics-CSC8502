#version 400

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;
in vec2 texCoord;
in vec4 jointWeights;
in vec4 jointColour;
in ivec4 jointIndices;


uniform mat4 joints[128];

out Vertex{
	vec2 texCoord;
	//vec4 jointColour;
} OUT;

void main(void){
	//vec3 visColour = vec3(0.0);
	vec4 localPos = vec4(position, 1.0f);
	vec4 skelPos = vec4(0, 0, 0, 0);

	for(int i = 0; i < 4; ++i){
		int jointIndex = jointIndices[i];
		float jointWeight = jointWeights[i];

		skelPos += joints[jointIndex] * localPos * jointWeight;
		//if(jointWeight == 1){
			//visColour = vec3(1.0, 0.0, 0.0);
		//}
		//else{
			//visColour = vec3(0.0, 0.0, 1.0);
		//}
	}
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4(skelPos.xyz, 1.0);
	OUT.texCoord = texCoord;
	//OUT.jointColour = vec4(visColour, 1.0);
}