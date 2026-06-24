#version 460 core

// G-Buffer outputs
layout(location = 0) out vec4 gPosition;   // World position XYZ, W=1 (geometry present)
layout(location = 1) out vec4 gNormal;     // World normal XYZ
layout(location = 2) out vec4 gAlbedo;     // Albedo RGB, roughness A
layout(location = 3) out vec4 gMaterial;   // Metallic R, AO G, emission B

in vec3 v_WorldPosition;
in vec3 v_Normal;
in vec2 v_TexCoord;

uniform vec4 u_Albedo = vec4(0.8, 0.8, 0.8, 1.0);
uniform float u_Metallic = 0.0;
uniform float u_Roughness = 0.5;
uniform float u_AmbientOcclusion = 1.0;
uniform vec4 u_EmissionColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform float u_EmissionIntensity = 0.0;
uniform bool u_EmissionEnabled = false;

void main() {
	float emission = u_EmissionEnabled ? u_EmissionIntensity : 0.0;

    gPosition = vec4(v_WorldPosition, 1.0);
    gNormal   = vec4(normalize(v_Normal), 0.0);
    gAlbedo   = vec4(u_Albedo.rgb, u_Roughness);
    gMaterial = vec4(u_Metallic, u_AmbientOcclusion, emission, 0.0);
}
