#version 450

layout(location = 0) in vec2 inPosition;

layout(location = 1) in vec2 inInstancePos;
layout(location = 2) in vec2 inInstanceScale;
layout(location = 3) in vec2 inInstanceSize;
layout(location = 4) in vec2 inInstanceOrigin;
layout(location = 5) in uvec4 inInstanceTextureRect;
layout(location = 6) in vec4 inInstanceColor;
layout(location = 7) in float inInstanceRotation;
layout(location = 8) in uint inInstanceTextureId;
layout(location = 9) in uint inInstanceFlags;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 fragColor;
layout(location = 2) flat out uint fragTextureId;
layout(location = 3) flat out uvec4 fragTextureRect;

layout(push_constant) uniform PushConstants {
    mat4 projectionView;
} pc;

const uint SPRITE_FLIP_X = 1u << 0;
const uint SPRITE_FLIP_Y = 1u << 1;

void main()
{
    vec2 localPosition =
        inPosition * inInstanceSize -
        inInstanceOrigin;

    localPosition *= inInstanceScale;

    float angle = radians(inInstanceRotation);
    float c = cos(angle);
    float s = sin(angle);

    mat2 rotationMatrix = mat2(
         c, s,
        -s, c
    );

    vec2 worldPosition =
        rotationMatrix * localPosition +
        inInstancePos;

    gl_Position =
        pc.projectionView *
        vec4(worldPosition, 0.0, 1.0);

    vec2 localUv = inPosition;

    if ((inInstanceFlags & SPRITE_FLIP_X) != 0u)
        localUv.x = 1.0 - localUv.x;

    if ((inInstanceFlags & SPRITE_FLIP_Y) != 0u)
        localUv.y = 1.0 - localUv.y;

    fragTexCoord = localUv;
    fragColor = inInstanceColor;
    fragTextureId = inInstanceTextureId;
    fragTextureRect = inInstanceTextureRect;
}