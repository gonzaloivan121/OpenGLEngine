#version 460 core

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;
layout(location = 3) out vec4 gMaterial;

in vec3 v_WorldPosition;
in vec3 v_Normal;
in vec2 v_TexCoord;

uniform vec3 u_Albedo = vec3(1.0);
uniform float u_Metallic = 0.0;
uniform float u_Roughness = 0.5;
uniform float u_AmbientOcclusion = 1.0;
uniform vec3 u_EmissionColor = vec3(1.0);
uniform float u_EmissionIntensity = 0.0;
uniform bool u_EmissionEnabled = false;

void main() {
    vec3 emission = u_EmissionEnabled ? u_EmissionIntensity * u_EmissionColor : vec3(0.0);

    gPosition = vec4(v_WorldPosition, 1.0);
    gNormal = vec4(normalize(v_Normal), 0.0);
    gAlbedo = vec4(u_Albedo, u_Roughness);
    gMaterial = vec4(u_Metallic, u_AmbientOcclusion, emission.r, 0.0);
}