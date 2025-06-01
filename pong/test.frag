#version 460

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 out_Color;

layout(binding = 1) uniform sampler2D uTexture;

void main() {
    // Busca a cor da textura
    vec4 texColor = texture(uTexture, fragTexCoord);

    // Se alpha da textura for 0 (sem textura real), cai no branco
    if (texColor.a == 0.0) {
        out_Color = vec4(1.0, 1.0, 1.0, 1.0);  // branco padrão
    } else {
        out_Color = texColor;
    }
}