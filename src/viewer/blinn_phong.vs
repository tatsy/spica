#version 150

in vec3 vertices;
in vec3 normals;
in vec3 colors;

uniform mat4 mMat;
uniform mat4 vMat;
uniform mat4 pMat;
uniform vec3 light;

out vec4 color_fs;
out vec4 normal_fs;
out vec3 VtoL;

void main(void) {
    gl_Position = pMat * vMat * mMat * vec4(vertices, 1.0);

    color_fs = vec4(colors, 1.0);
    normal_fs = vMat * mMat * vec4(normals, 0.0);
    vec4 V = vMat * mMat * vec4(vertices, 1.0);
    VtoL = light - V.xyz;
}
