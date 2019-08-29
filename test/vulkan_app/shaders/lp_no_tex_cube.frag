#version 450

layout(location = 0) in GS_DATA
{
    vec3 vs_position;
    vec3 vs_normal;
} fs_in;


layout(location = 0) out vec4 out_final;
layout(location = 1) out vec4 out_albedo;
layout(location = 2) out vec4 out_position;
layout(location = 3) out vec4 out_normal;

layout(push_constant) uniform Push_Constants
{
    mat4 model_view;
    mat4 projection;
    vec4 color;

    float roughness;
    float metalness;
} push_k;

void set_roughness(float v)
{
    out_normal.a = v;
}

void set_metalness(float v)
{
    out_position.a = v;
}

void main(void)
{
    out_final = vec4(0.0, 0.0, 0.0, 1.0);
    out_albedo = vec4(push_k.color.rgb, 1.0);

    out_position = vec4(fs_in.vs_position, 1.0);
    out_normal = vec4(fs_in.vs_normal, 1.0);

    set_roughness(push_k.roughness);
    set_metalness(push_k.metalness);
}
