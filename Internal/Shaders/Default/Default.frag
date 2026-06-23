#version 460 core

out vec4 o_FragColor;

in vec2 v_TexCoord;

void main() {
    vec2 uv = clamp(v_TexCoord, 0.0, 1.0);
    o_FragColor = vec4(uv, 0.0, 1.0);
}