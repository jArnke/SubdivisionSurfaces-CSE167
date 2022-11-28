#version 330 core

in vec4 position;
in vec3 normal;

uniform mat4 modelview;

// Output the frag color
out vec4 fragColor;


void main (void){
    vec3 N = normalize(normal);
    fragColor = vec4(0.5f*N + 0.5f , 1.0f);
}
