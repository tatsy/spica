#version 150

in vec3 vertices;
in vec3 normals;
in vec3 colors;

uniform mat4 mMat;
uniform mat4 vMat;
uniform mat4 pMat;
uniform mat4 normalMat;
uniform vec3 light;

smooth out vec3 vertexCameraspace;
smooth out vec3 normalCameraspace;
smooth out vec3 lightCameraspace;
smooth out vec4 vertexColor;

void main(void) {
    gl_Position = pMat * vMat * mMat * vec4(vertices, 1.0);

    vertexCameraspace = (vMat * mMat * vec4(vertices, 1.0)).xyz;
    normalCameraspace = (normalMat * vec4(normals, 0.0)).xyz;

    lightCameraspace = (vMat * mMat * vec4(light, 0.0)).xyz;

    vertexColor = vec4(colors, 1.0);
}
