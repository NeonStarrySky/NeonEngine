// vertex_shader.glsl
#version 460 core

// ∂•µ„ ‰»ÎŒª÷√
layout(location = 0) in vec3 p;
uniform vec3 cameraFront;//n1
uniform vec3 cameraPosition;//o
void main() {
    vec3 op = p - cameraPosition;
    //float ph = max(dot(op, cameraFront),0);
    float ph = dot(op, cameraFront);
    vec3 oh = op-ph*cameraFront;

    gl_Position = vec4(oh.x, oh.y , ph*ph*0.1 , ph);
}