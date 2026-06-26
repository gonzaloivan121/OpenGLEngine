#version 460 core

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;
layout(location = 3) out vec4 gMaterial;

in vec3 v_WorldPosition;
in vec3 v_Normal;
in vec2 v_TexCoord;

uniform vec4 u_Albedo = vec4(1.0);
uniform float u_Metallic = 0.0;
uniform float u_Roughness = 0.5;
uniform float u_AmbientOcclusion = 1.0;
uniform vec4 u_EmissionColor = vec4(1.0);
uniform float u_EmissionIntensity = 0.0;
uniform bool u_EmissionEnabled = false;

float Quantize(float value, float steps) {
    return floor(value * steps) / max(steps - 1.0, 1.0);
}

vec3 Quantize(vec3 color, float steps) {
    return vec3(
        Quantize(color.r, steps),
        Quantize(color.g, steps),
        Quantize(color.b, steps)
    );
}

void main() {
    vec3 emission = u_EmissionEnabled ? u_EmissionIntensity * u_EmissionColor.rgb : vec3(0.0);

    vec3 normal = normalize(v_Normal);

    // A simple cell look for deferred: color bands and flatter spec setup.
    vec3 celAlbedo = Quantize(u_Albedo.rgb, 4.0);
    float celRoughness = Quantize(clamp(u_Roughness, 0.0, 1.0), 3.0);

    gPosition = vec4(v_WorldPosition, 1.0);
    gNormal = vec4(normal, 0.0);
    gAlbedo = vec4(celAlbedo, celRoughness);
    gMaterial = vec4(u_Metallic, u_AmbientOcclusion, emission.r, 1.0);
}
