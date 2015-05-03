varying vec4 normal;
varying vec3 VtoL;

void main(void) {
    vec3 N = normalize(normal.xyz);
    vec3 L = normalize(VtoL);
    float NdotL = dot(N, L);
    vec4 diffuse = vec4(max(0.0, NdotL));
    gl_FragColor = diffuse;
}
