#version 460

layout(location = 0) in vec3 inPos;   // posição do vértice
layout(location = 1) in vec2 inTex;   // coordenada de textura

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) flat out int texIndex;

// ALTERAÇÃO MÍNIMA: adiciona “std140” no uniform block
layout (std140, binding = 0) uniform UniformBuffer {
  mat4 WVP;
  int textureIndex;
} ubo;

void main() {
  // gl_Position = vec4(pos, 1.0);
  gl_Position = ubo.WVP * vec4(inPos, 1.0);

  fragTexCoord = inTex;
  texIndex = ubo.textureIndex;
}