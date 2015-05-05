#version 150

in vec4 color_fs;
in vec4 normal_fs;
in vec3 VtoL;

out vec4 color;

void main(void) {
    vec3 N = normalize(normal_fs.xyz);
    vec3 L = normalize(VtoL);
    float NdotL = dot(N, L);
    vec4 diffuse = vec4(max(0.0, NdotL));
    color = color_fs * diffuse;
}
