#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler2D textures[];

layout(location = 0) in vec2 fragLocalUv;
layout(location = 1) in vec4 fragColor;
layout(location = 2) flat in uint fragTextureId;
layout(location = 3) flat in uvec4 fragTextureRect;

layout(location = 0) out vec4 outColor;

void main() {
    uint textureIndex = nonuniformEXT(fragTextureId);

    vec2 textureDimensions =
        vec2(textureSize(textures[textureIndex], 0));

    vec2 rectanglePosition =
        vec2(fragTextureRect.xy);

    vec2 rectangleSize =
        vec2(fragTextureRect.zw);

    if (rectangleSize.x == 0.0 || rectangleSize.y == 0.0) {
        rectanglePosition = vec2(0.0);
        rectangleSize = textureDimensions;
    }

    vec2 pixelCoordinates =
        rectanglePosition +
        fragLocalUv * rectangleSize;

    vec2 normalizedUv =
        pixelCoordinates / textureDimensions;

    outColor =
        texture(textures[textureIndex], normalizedUv) *
        fragColor;
}