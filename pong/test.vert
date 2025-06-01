#version 460

struct VertexData {
  float x, y, z;
  float u, v;
};

layout (binding = 0) readonly buffer Vertices {VertexData data[];} in_Vertices;

// ALTERAÇÃO MÍNIMA: adiciona “std140” no uniform block
layout (std140, binding = 1) uniform UniformBuffer {
  mat4 WVP;
} ubo;

void main() {
  VertexData vtx = in_Vertices.data[gl_VertexIndex];

  vec3 pos = vec3(vtx.x, vtx.y, vtx.z);

  // gl_Position = vec4(pos, 1.0);
  gl_Position = ubo.WVP * vec4(pos, 1.0);
}