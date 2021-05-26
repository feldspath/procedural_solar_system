#version 330 core

out vec4 FragColor;

in vec2 uv_frag;

uniform sampler2D image_texture;
uniform sampler2D image_texture_2; // Depth buffer

uniform vec4 planetCenter;

uniform mat4 viewMatrix;
uniform mat4 perspectiveInverse;

uniform float oceanLevel=1.0f;
uniform float depthMultiplier = 6.0f;
uniform float waterBlendMultiplier = 60.0f;

uniform vec4 waterColorDeep;
uniform vec4 waterColorSurface;

uniform float near = 0.1;
uniform float far  = 100.0;

uniform vec3 lightSource;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

vec2 raySphere(vec3 center, float radius, vec3 rayOrigin, vec3 rayDir) {
  vec3 offset = rayOrigin - center;
  float b = 2 * dot(offset, rayDir);
  float c = dot(offset, offset) - radius * radius;

  float disc = b*b - 4*c;

  if (disc > 0) {
    float s = sqrt(disc);
    float dstToSphereNear = max(0, (-b - s) / 2);
    float dstToSphereFar =(-b+s) / 2;

    if (dstToSphereFar >= 0) {
      return vec2(dstToSphereNear, dstToSphereFar-dstToSphereNear);
    }
  }

  return vec2(far, 0.0);
}

vec3 cameraDirection(vec2 screenPos) {
  return normalize(vec3(perspectiveInverse * vec4(screenPos, -1.0f, 1.0f)));
}

void main() {
    FragColor = texture(image_texture, uv_frag);
    float depth = LinearizeDepth(texture(image_texture_2, uv_frag).r);
    vec3 direction = cameraDirection(2 * uv_frag - 1);
    vec3 camSpaceFrag = direction * depth / direction.z;
    float depthFromCamera = length(camSpaceFrag);

    vec3 camSpacePlanet = vec3(viewMatrix * planetCenter);
    vec2 hitInfo = raySphere(camSpacePlanet, oceanLevel, vec3(0.0f, 0.0f, 0.0f), direction);
    float dstToOcean = hitInfo.x;
    float dstThroughOcean = hitInfo.y;
    float oceanViewDepth = min(dstThroughOcean, depthFromCamera - dstToOcean);
    if (oceanViewDepth > 0) {
      float opticalDepth = 1 - exp(-oceanViewDepth * depthMultiplier);
      vec4 oceanCol = mix(waterColorSurface, waterColorDeep, opticalDepth);

      float alpha = 1 - exp(-oceanViewDepth * oceanCol.w * waterBlendMultiplier);
      vec3 color = mix(vec3(FragColor), vec3(oceanCol), alpha);

      vec3 camSpaceLight = vec3(viewMatrix * vec4(lightSource, 1.0));
      vec3 camSpaceActual = direction * dstToOcean;
      vec3 N = normalize(camSpaceActual - camSpacePlanet);
      vec3 L = normalize(camSpaceLight - camSpaceActual);

      float diffuse = max(dot(N,L),0.0);
      float specular = 0.0;
      if(diffuse>0.0){
        vec3 R = reflect(-L,N);
        vec3 V = normalize(-camSpaceActual);
        float specular_exp = 64;
        specular = pow( max(dot(R,V),0.0), specular_exp );
      }

      FragColor = vec4(vec3(color) * diffuse+ vec3(1.0f, 1.0f, 1.0f) * specular, 1.0f);


    }

}
