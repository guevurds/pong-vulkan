#version 460

layout(location = 0) in vec3 inPos;   // posição do vértice
layout(location = 1) in vec2 inTex;   // coordenada de textura

// ALTERAÇÃO MÍNIMA: adiciona “std140” no uniform block
layout (std140, binding = 0) uniform UniformBuffer {
  mat4 WVP;
} ubo;

void main() {

  // gl_Position = vec4(pos, 1.0);
  gl_Position = ubo.WVP * vec4(inPos, 1.0);
}