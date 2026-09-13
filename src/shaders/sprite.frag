#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in flat uint fragTextureId;

layout(set = 0, binding = 0) uniform sampler2D textures[];

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(textures[nonuniformEXT(fragTextureId)], fragTexCoord);

    if (outColor.a < 0.1) {
        discard;
    }
}
