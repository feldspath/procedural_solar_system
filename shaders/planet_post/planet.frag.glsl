std::string s = R"(
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

uniform vec4 planetCenter;

uniform mat4 viewMatrix;
uniform mat4 perspectiveInverse;

uniform float oceanLevel;
uniform float depthMultiplier = 6.0f;
uniform float waterBlendMultiplier = 60.0f;

uniform vec3 waterColorDeep;
uniform vec3 waterColorSurface;

uniform float near = 0.1;
uniform float far  = 100.0;

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
    FragColor = texture(screenTexture, TexCoords);
    float depth = LinearizeDepth(texture(depthTexture, TexCoords).r);
    vec3 direction = cameraDirection(2 * TexCoords - 1);
    vec3 camSpaceFrag = direction * depth / direction.z;
    float depthFromCamera = length(camSpaceFrag);

    vec2 hitInfo = raySphere(vec3(viewMatrix * planetCenter), oceanLevel, vec3(0.0f, 0.0f, 0.0f), direction);
    float dstToOcean = hitInfo.x;
    float dstThroughOcean = hitInfo.y;
    float oceanViewDepth = min(dstThroughOcean, depthFromCamera - dstToOcean);
    if (oceanViewDepth > 0) {
      float opticalDepth = 1 - exp(-oceanViewDepth * depthMultiplier);
      float alpha = 1 - exp(-oceanViewDepth * waterBlendMultiplier);
      //vec4 oceanCol = mix(vec4(0.3, 0.5, 1.0, 1.0), vec4(0.1, 0.1, 0.1, 1.0), opticalDepth);
      vec4 oceanCol = mix(vec4(waterColorSurface, 1.0), vec4(waterColorDeep, 1.0), opticalDepth);
      FragColor = mix(FragColor, oceanCol, alpha);
    }




}
)";
