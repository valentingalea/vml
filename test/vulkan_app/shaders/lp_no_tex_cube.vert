#version 450

layout(push_constant) uniform Push_Constants
{
    mat4 model_view;
    mat4 projection;
    vec4 color;

    float roughness;
    float metalness;
} push_k;

layout(location = 0) in vec3 vertex_position;

layout(location = 0) out VS_DATA
{
    // view space
    vec3 vs_position;
} vs_out;

void main(void)
{
    vec4 vs_position = push_k.model_view * vec4(vertex_position, 1.0);

    gl_Position = push_k.projection * vs_position;

    vs_out.vs_position = vs_position.xyz;
}
 
