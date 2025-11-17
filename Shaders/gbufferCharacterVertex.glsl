#version 330 core

// --- Matrices ---
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 modelMatrixInverseTranspose;

// --- Skinning ---
uniform mat4 jointTransforms[128];

// --- Material ---
in vec3 position;
in vec3 normal;
in vec2 texCoord;
in vec3 tangent;
in vec3 binormal;

in ivec4 joints;
in vec4 weights;

// Output to fragment shader
out Vertex {
    vec3 worldPos;
    vec2 texCoord;
    mat3 TBN;
} OUT;

void main() {

    // ----- Skinning -----
    mat4 skinMatrix =
          jointTransforms[joints.x] * weights.x +
          jointTransforms[joints.y] * weights.y +
          jointTransforms[joints.z] * weights.z +
          jointTransforms[joints.w] * weights.w;

    vec4 skinnedPos    = skinMatrix * vec4(position, 1.0);
    vec3 skinnedNormal = normalize((skinMatrix * vec4(normal, 0.0)).xyz);
    vec3 skinnedTang   = normalize((skinMatrix * vec4(tangent, 0.0)).xyz);
    vec3 skinnedBitan  = normalize((skinMatrix * vec4(binormal, 0.0)).xyz);

    vec4 worldPos4 = modelMatrix * skinnedPos;
    OUT.worldPos = worldPos4.xyz;

    // Build TBN
    OUT.TBN = mat3(
        normalize(modelMatrix * vec4(skinnedTang, 0.0)).xyz,
        normalize(modelMatrix * vec4(skinnedBitan, 0.0)).xyz,
        normalize(modelMatrix * vec4(skinnedNormal, 0.0)).xyz
    );

    OUT.texCoord = texCoord;

    gl_Position = projMatrix * viewMatrix * worldPos4;
}