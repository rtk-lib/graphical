#version 450

layout(location = 0) in vec2 inPosition;

layout(location = 1) in vec2 inInstancePos;
layout(location = 2) in vec2 inInstanceSize;
layout(location = 3) in float inInstanceRotation;
layout(location = 4) in uint inInstanceTextureId;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) flat out uint fragTextureId;

layout(push_constant) uniform PushConstants {
    mat4 projectionView;
} pc;

void main() {
    vec2 localPos = inPosition * inInstanceSize;

    float rad = radians(inInstanceRotation);
    float c = cos(rad);
    float s = sin(rad);
    mat2 rot = mat2(c, s, -s, c);
    vec2 rotatedPos = rot * localPos;

    vec2 worldPos = rotatedPos + inInstancePos;
    gl_Position = pc.projectionView * vec4(worldPos, 0.0, 1.0);

    fragTexCoord = inPosition;

    fragTextureId = inInstanceTextureId;
}