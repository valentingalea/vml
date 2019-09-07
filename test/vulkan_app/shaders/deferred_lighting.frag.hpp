layout(location = 0) in vec2 in_uvs;

layout(location = 0) out vec4 out_color = vec4();

const uint G_BUFFER_ALBEDO	= 0;
const uint G_BUFFER_POSITION	= 1;
const uint G_BUFFER_NORMAL	= 2;

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput g_buffer_albedo;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput g_buffer_position;
layout(input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput g_buffer_normal;

layout(push_constant) uniform push_k_t
{
    vec4 light_direction;
    mat4 view_matrix;
} push_k;

const float PI = 3.14159265359;

float normal_distribution_ggx(vec3 vs_normal, vec3 halfway, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float ndoth = max(dot(vs_normal, halfway), 0.0);
    float a2minus1 = a2 - 1;
    float integral_den = ndoth * ndoth * a2minus1 + 1;

    float num = a2;
    float den = PI * integral_den * integral_den;

    return num / den;
}

float geometry_schlick_ggx(float ndotv, float roughness)
{
    float r = (roughness + 1);
    float k = (r * r) / 8;

    float num = ndotv;
    float den = ndotv * (1 - k) + k;

    return num / den;
}

float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness)
{
    float ndotv = max(dot(n, v), 0.0);
    float ndotl = max(dot(n, l), 0.0);

    return geometry_schlick_ggx(ndotv, roughness) * geometry_schlick_ggx(ndotl, roughness);
}

vec3 fresnel_schlick(float cos_theta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}


// roughness of the material controlled in normal.a and the metalness in the position.a
vec4 lighting(void)
{
    vec4 albedo = subpassLoad(g_buffer_albedo);
    albedo.xyz = pow(albedo.xyz, vec3(2.2));
    vec4 gposition = subpassLoad(g_buffer_position);
    vec3 vs_position = gposition.xyz;
    vec4 gnormal = -subpassLoad(g_buffer_normal);
    vec3 vs_normal = gnormal.xyz;
    float roughness = gnormal.a;
    float metallic = gposition.a;

    vec3 radiance = vec3(24.47, 21.31, 20.79);
    vec3 ws_light = normalize(push_k.light_direction.xyz);
    ws_light.y *= 1.0;
    vec3 light_vector = vec3(push_k.view_matrix * vec4(ws_light, 0.0));
    vec3 to_camera = normalize(vs_position);
    vec3 halfway = normalize(to_camera + light_vector);

    float n_dot_l = max(dot(vs_normal, light_vector), 0.0);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.xyz, metallic);
    vec3 F = fresnel_schlick(max(dot(vs_normal, to_camera), 0.0), F0);
    float D = normal_distribution_ggx(vs_normal, halfway, roughness);
    float G = geometry_smith(vs_normal, to_camera, light_vector, roughness);

    vec3 num = F * D * G;
    float den = 4 * max(dot(vs_normal, to_camera), 0.0) * max(dot(vs_normal, light_vector), 0.1);
    vec3 spec = num / max(den, 0.001);

    vec3 ks = F * 0.5;
    vec3 kd = vec3(1.0) - ks;
    kd *= 1.0 - metallic;

    vec3 result = (kd * vec3(albedo) / PI + spec) * radiance * n_dot_l;

    vec3 ambient = vec3(0.03) * vec3(albedo);
    result += ambient;

    ambient = pow(ambient, vec3(1.0 / 2.2));
    
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0 / 2.2));

    result = clamp(result, ambient, vec3(1.0));

    return vec4(result, 1.0);
}

void main(void)
{
    vec3 albedo_color = subpassLoad(g_buffer_albedo).rgb;
    vec3 vs_position = subpassLoad(g_buffer_position).rgb;
    vec3 vs_normal = subpassLoad(g_buffer_normal).rgb;

    out_color = vec4(albedo_color, 1.0);

    if (vs_normal.x > -10.0 && vs_normal.y > -10.0 && vs_normal.z > -10.0)
    {
	out_color = lighting();
    }
}
