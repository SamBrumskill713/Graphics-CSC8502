#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

in Vertex {
    vec3 worldPos;
    vec2 texCoord;
    mat3 TBN;
} IN;

layout(location = 0) out vec4 gAlbedo; // base colour
layout(location = 1) out vec4 gNormal; // encoded normals (xyz), specular in w

void main() {

    // Albedo
    vec4 baseCol = texture(diffuseTex, IN.texCoord);
    if(baseCol.a < 0.1) discard;       // alpha test for cutouts

    gAlbedo = baseCol;

    // Normal map (tangent → world)
    vec3 normalTex = texture(bumpTex, IN.texCoord).xyz * 2.0 - 1.0;
    vec3 worldNormal = normalize(IN.TBN * normalTex);

    gNormal = vec4(normalize(worldNormal) * 0.5 + 0.5, 1.0);
}