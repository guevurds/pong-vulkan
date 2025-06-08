#version 460

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in int texIndex;

layout(location = 0) out vec4 out_Color;


layout(binding = 1) uniform sampler2D uTexture[MAX_TEXTURES]; //definida em tempo de execução pelo script shader.cpp


void main() {
    // Busca a cor da textura
    vec4 texColor = texture(uTexture[texIndex], fragTexCoord);

    // Se alpha da textura for 0 (sem textura real), cai no branco
    out_Color = texColor;
} 