#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D globalTextures[];

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

void main() {
    outPosition = vec4(fragPos, 1.0);
    outNormal = vec4(normalize(fragNormal), 1.0);
    
    outAlbedo = texture(globalTextures[0], fragTexCoord); 
}