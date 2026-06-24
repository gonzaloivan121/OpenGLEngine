#version 460 core

out vec4 o_FragColor;

in vec2 v_TexCoord;

// G-Buffer samplers
uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_gMaterial;

// Camera
uniform vec3 u_CameraPosition;
uniform vec2 u_Resolution;

// Lights -----------------------------------------------------------

#define MAX_LIGHTS 16

// Directional lights
uniform int   u_DirLightCount;
uniform vec3  u_DirLightDirection[MAX_LIGHTS];
uniform vec3  u_DirLightColor[MAX_LIGHTS];
uniform float u_DirLightIntensity[MAX_LIGHTS];

// Point lights
uniform int   u_PointLightCount;
uniform vec3  u_PointLightPosition[MAX_LIGHTS];
uniform vec3  u_PointLightColor[MAX_LIGHTS];
uniform float u_PointLightIntensity[MAX_LIGHTS];
uniform float u_PointLightRange[MAX_LIGHTS];
uniform float u_PointLightLinear[MAX_LIGHTS];
uniform float u_PointLightQuadratic[MAX_LIGHTS];

// Spot lights
uniform int   u_SpotLightCount;
uniform vec3  u_SpotLightPosition[MAX_LIGHTS];
uniform vec3  u_SpotLightDirection[MAX_LIGHTS];
uniform vec3  u_SpotLightColor[MAX_LIGHTS];
uniform float u_SpotLightIntensity[MAX_LIGHTS];
uniform float u_SpotLightInnerCone[MAX_LIGHTS];
uniform float u_SpotLightOuterCone[MAX_LIGHTS];

// ------------------------------------------------------------------

vec3 CalcDirectionalLight(int i, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(-u_DirLightDirection[i]);
    float diff = max(dot(normal, lightDir), 0.0);

    // Blinn-Phong specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float shininess = mix(128.0, 2.0, roughness);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    float specStrength = mix(0.04, 1.0, metallic);

    vec3 diffuse  = diff * albedo * u_DirLightColor[i] * u_DirLightIntensity[i];
    vec3 specular = spec * specStrength * u_DirLightColor[i] * u_DirLightIntensity[i];
    return diffuse + specular;
}

vec3 CalcPointLight(int i, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(u_PointLightPosition[i] - fragPos);
    float dist = length(u_PointLightPosition[i] - fragPos);

    if (dist > u_PointLightRange[i]) return vec3(0.0);

    float attenuation = 1.0 / (1.0 + u_PointLightLinear[i] * dist + u_PointLightQuadratic[i] * dist * dist);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float shininess = mix(128.0, 2.0, roughness);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    float specStrength = mix(0.04, 1.0, metallic);

    vec3 diffuse  = diff * albedo * u_PointLightColor[i] * u_PointLightIntensity[i] * attenuation;
    vec3 specular = spec * specStrength * u_PointLightColor[i] * u_PointLightIntensity[i] * attenuation;
    return diffuse + specular;
}

vec3 CalcSpotLight(int i, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(u_SpotLightPosition[i] - fragPos);
    float theta = dot(lightDir, normalize(-u_SpotLightDirection[i]));
    float epsilon = u_SpotLightInnerCone[i] - u_SpotLightOuterCone[i];
    float intensity = clamp((theta - u_SpotLightOuterCone[i]) / epsilon, 0.0, 1.0);

    if (intensity <= 0.0) return vec3(0.0);

    float dist = length(u_SpotLightPosition[i] - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float shininess = mix(128.0, 2.0, roughness);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    float specStrength = mix(0.04, 1.0, metallic);

    vec3 diffuse  = diff * albedo * u_SpotLightColor[i] * u_SpotLightIntensity[i] * intensity;
    vec3 specular = spec * specStrength * u_SpotLightColor[i] * u_SpotLightIntensity[i] * intensity;
    return diffuse + specular;
}

void main() {
    vec4 positionData = texture(u_gPosition, v_TexCoord);

    // If W == 0 no geometry was rendered here — discard so the background pass shows through
    if (positionData.w == 0.0) {
        discard;
    }

    vec3 fragPos   = positionData.xyz;
    vec3 normal    = normalize(texture(u_gNormal, v_TexCoord).xyz);
    vec4 albedoData = texture(u_gAlbedo, v_TexCoord);
    vec3 albedo    = albedoData.rgb;
    float roughness = albedoData.a;
    vec4 materialData = texture(u_gMaterial, v_TexCoord);
    float metallic  = materialData.r;
    float ao        = materialData.g;
    float emission  = materialData.b;

    vec3 viewDir = normalize(u_CameraPosition - fragPos);

    // Ambient
    vec3 ambient = 0.03 * albedo * ao;

    vec3 lighting = ambient;

    // Directional lights
    for (int i = 0; i < u_DirLightCount; ++i) {
        lighting += CalcDirectionalLight(i, normal, viewDir, albedo, roughness, metallic);
    }

    // Point lights
    for (int i = 0; i < u_PointLightCount; ++i) {
        lighting += CalcPointLight(i, fragPos, normal, viewDir, albedo, roughness, metallic);
    }

    // Spot lights
    for (int i = 0; i < u_SpotLightCount; ++i) {
        lighting += CalcSpotLight(i, fragPos, normal, viewDir, albedo, roughness, metallic);
    }

    // Emission
    lighting += albedo * emission;

    o_FragColor = vec4(lighting, 1.0);
}
