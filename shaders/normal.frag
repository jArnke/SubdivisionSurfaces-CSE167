#version 330 core

in vec4 position;
in vec3 normal;

uniform mat4 modelview;


//material params
const vec4 ambient = vec4(0.1f, 0.2f, 0.17f, 1.0f);
const vec4 diffuse =  vec4(0.2f, 0.375f, 0.35f, 1.0f);
const vec4 specular =  vec4(0.3f, 0.3f, 0.3f, 1.0f);
const vec4 emision = vec4(.1f, .1f, .1f, 1.0f);
const float shininess = 100.0f;

const int maximal_allowed_lights = 10;
uniform bool enable_lighting;
const int nlights = 1;
uniform vec4 lightpositions[nlights]; 
uniform vec4 lightcolors[nlights];

// Output the frag color
out vec4 fragColor;


void main (void){
    vec4 PosEye = modelview * position;
    vec3 nEye   = normalize( mat3(inverse(transpose(modelview))) * normal );

    vec3 N = normalize(normal);
    if(!enable_lighting){
        fragColor = vec4(0.5f*N + 0.5f , 1.0f);
    }
    else{
    vec4 sum = emision;

        for(int i=0;i<nlights;++i)
        {
          vec4 Lpos = modelview * lightpositions[i];
          vec3 L = normalize(PosEye.w*Lpos.xyz - Lpos.w*PosEye.xyz);
          vec4 Origin = vec4(0.0f,0.0f,0.0f,1.0f);
          vec3 V = normalize(PosEye.w*Origin.xyz - Origin.w*PosEye.xyz);
          vec3 H = normalize(L + V);
          vec4 color = lightcolors[i];
          float angle = dot(nEye, L);

          vec4 diffuseColor = diffuse * max(angle, 0);
          vec4 specColor = specular * pow( max(dot(nEye, H),0), shininess );
          
          sum += color * (diffuseColor + specColor + ambient);
          
        }
        fragColor = sum;
        
    }
}
