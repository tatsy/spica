#version 150

uniform vec3 cameraPosition;

smooth in vec3 vertexCameraspace;
smooth in vec3 normalCameraspace;
smooth in vec3 lightCameraspace;
smooth in vec4 vertexColor;

out vec4 color;

vec4 ambient = vec4(0.1, 0.1, 0.1, 1.0);

void main(void) {
    vec3 V = normalize(cameraPosition - vertexCameraspace);
    vec3 N = normalize(normalCameraspace);
    vec3 L = normalize(lightCameraspace - vertexCameraspace);
    vec3 H = normalize(L + V);

    float n_dot_l = dot(N, L);
    vec4 diffuse = vec4(max(0.0, n_dot_l));

    float n_dot_h = dot(N, H);
    vec4 specular = vec4(pow(max(0.0, n_dot_h), 64.0));

    color = ambient + vertexColor * (diffuse + specular);
}
