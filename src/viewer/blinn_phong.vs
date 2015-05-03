uniform mat4 mMat;
uniform mat4 vMat;
uniform mat4 pMat;

varying vec4 normal;
varying vec3 VtoL;

void main(void) {
    gl_Position = pMat * vMat * mMat * gl_Vertex;

    normal = vMat * mMat * vec4(gl_Normal, 0.0);
    vec4 V = vMat * mMat * gl_Vertex;
    VtoL = gl_LightSource[0].position.xyz - V.xyz;
}
