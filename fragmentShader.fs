#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

struct Material {
    vec3 diffuse;
    vec3 specular;
    float shininess;
    sampler2D texture_diffuse1;
    int useTexture;
    float alpha;
    vec3 emissive;
    int ignoreLighting;
}; 

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    int isExterior;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NR_POINT_LIGHTS 14

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform vec3 viewPos;
uniform bool turnAmbient;
uniform bool turnDiffuse;
uniform bool turnSpecular;
uniform bool turnAll;

// Function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffTex, vec3 specTex);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffTex, vec3 specTex);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffTex, vec3 specTex);

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 diffTex = material.diffuse;
    if(material.useTexture != 0) {
        diffTex = vec3(texture(material.texture_diffuse1, TexCoord));
    }
    vec3 specTex = material.specular;

    vec3 result = vec3(0.0);
    
    // Physical Volume Detection: Determine if fragment is inside or outside the warehouse
    // Warehouse bounds are exactly X:[-40,40], Y:[0,20], Z:[-40,40]
    // Interior walls start at 39.9. We use 39.95 as the cutoff.
    bool isInside = (abs(FragPos.x) < 39.95 && FragPos.y < 19.95 && abs(FragPos.z) < 39.95);
    
    // Step 1: Directional lighting (Sun/Moon)
    // Exterior lighting (Sun) hit everything if turnAll is on, or just exterior components
    if(turnAll || !isInside || material.ignoreLighting != 0) {
        result += CalcDirLight(dirLight, norm, viewDir, diffTex, specTex);
    }

    // Step 2: Point lights loop (Independent Control)
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        if (material.ignoreLighting == 2) continue; // Skip Sun/Moon/Clouds
        
        bool lightIsExt = (pointLights[i].isExterior == 1);
        
        // Isolation Logic:
        // Inside fragments only receive light from interior bulbs (isExterior 0)
        // Outside fragments only receive light from entrance bulbs (isExterior 1)
        if (lightIsExt != isInside) {
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, diffTex, specTex);
        }
    }
    
    // Step 3: Spot light (Interior only)
    if (isInside) {
        result += CalcSpotLight(spotLight, norm, FragPos, viewDir, diffTex, specTex);
    }
    
    FragColor = vec4(result + material.emissive, material.alpha);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffTex, vec3 specTex) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * diffTex;
    vec3 diffuse = light.diffuse * diff * diffTex;
    vec3 specular = light.specular * spec * specTex;
    
    vec3 res = vec3(0);
    if(turnAmbient || material.ignoreLighting != 0) res += ambient;
    if(turnDiffuse || material.ignoreLighting != 0) res += diffuse;
    if(turnSpecular || material.ignoreLighting != 0) res += specular;
    return res;
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffTex, vec3 specTex) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * diffTex;
    vec3 diffuse = light.diffuse * diff * diffTex;
    vec3 specular = light.specular * spec * specTex;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    vec3 res = vec3(0);
    if(turnAmbient || light.isExterior == 1) res += ambient;
    if(turnDiffuse || light.isExterior == 1) res += diffuse;
    if(turnSpecular || light.isExterior == 1) res += specular;
    return res;
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffTex, vec3 specTex) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * diffTex;
    vec3 diffuse = light.diffuse * diff * diffTex;
    vec3 specular = light.specular * spec * specTex;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    vec3 res = vec3(0);
    if(turnAmbient) res += ambient;
    if(turnDiffuse) res += diffuse;
    if(turnSpecular) res += specular;
    return res;
}
