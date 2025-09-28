#version 330 core
out vec4 FragColor;

uniform vec2 positions[30];
uniform int valid;

const float offset = 0.25f;
const float a = 1.25f;

void main() {
    vec3 col = vec3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < valid; i++){
        float radius = (offset * (a * i));
        if (distance(gl_FragCoord.xy, positions[i].xy) <= radius) {
            col = vec3(1.0f, 1.0f, 1.0f); 
            break; 
        }  
    }
    FragColor = vec4(col, 1.0f);
}
