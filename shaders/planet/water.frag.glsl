#version 330 core

out vec4 FragColor;

in vec2 uv_frag;

uniform sampler2D image_texture;
uniform sampler2D image_texture_2; // Depth buffer

uniform mat4 viewMatrix;
uniform mat4 perspectiveInverse;
uniform float near;
uniform float far;

uniform vec4 worldPlanetCenter;
uniform float planetRadius;

uniform float oceanLevel;
uniform float depthMultiplier;
uniform float waterBlendMultiplier;
uniform vec4 waterColorDeep;
uniform vec4 waterColorSurface;

uniform bool hasAtmosphere;
uniform float atmosphereHeight;
uniform int nScatteringPoints;
uniform int nOpticalDepthPoints;
uniform float densityFalloff;
uniform vec3 scatteringCoeffs;

uniform bool isSun;
uniform bool waterGlow;
uniform bool specularWater;


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

float densityAtPoint(vec3 point, vec3 planetCenter) {
  float altitude = length(point - planetCenter) - planetRadius;
  float normalizedAltitude = altitude / (atmosphereHeight - planetRadius);
  return exp(-normalizedAltitude * densityFalloff) * (1 - normalizedAltitude);
}

float opticalDepth(vec3 point, vec3 direction, float rayLength, vec3 planetCenter) {
  vec3 samplePoint = point;
  float stepSize = rayLength / (nOpticalDepthPoints - 1);
  float opticalDepth = 0.0;
  for (int i = 0; i < nOpticalDepthPoints; i++) {
    opticalDepth += densityAtPoint(samplePoint, planetCenter) * stepSize;
    samplePoint += direction * stepSize;
  }
  return opticalDepth;
}

vec3 calculateLight(vec3 rayOrigin, vec3 rayDirection, float rayLength, vec3 planetCenter, vec3 lightPosition, vec3 originalColor) {
  vec3 scatterPoint = rayOrigin;
  float stepSize = rayLength / (nScatteringPoints - 1);
  vec3 scatteredLight = vec3(0.0);
  float viewRayOpticalDepth = 0.0;

  for (int i = 0; i < nScatteringPoints; i++) {
    vec3 dirToSun = normalize(lightPosition-scatterPoint);
    float sunRayLength = raySphere(planetCenter, atmosphereHeight, scatterPoint, dirToSun).y;
    if (isSun)
      sunRayLength = min(sunRayLength, 1.0f);
    float sunRayOpticalDepth = opticalDepth(scatterPoint, dirToSun, sunRayLength, planetCenter);
    viewRayOpticalDepth = opticalDepth(scatterPoint, -rayDirection, stepSize * i, planetCenter);
    vec3 transmittance = exp(-(sunRayOpticalDepth + viewRayOpticalDepth) * scatteringCoeffs);
    float localDensity = densityAtPoint(scatterPoint, planetCenter);

    scatteredLight += localDensity * transmittance * scatteringCoeffs * stepSize;
    scatterPoint += rayDirection * stepSize;
  }
  float originalColorTransmittance = exp(-viewRayOpticalDepth);

  return scatteredLight + originalColor * originalColorTransmittance;
}

void main() {
    FragColor = texture(image_texture, uv_frag);
    float depth = LinearizeDepth(texture(image_texture_2, uv_frag).r);
    vec3 direction = cameraDirection(2 * uv_frag - 1);
    vec3 camSpaceFrag = direction * depth / direction.z;
    float depthFromCamera = length(camSpaceFrag);
    vec3 camSpacePlanet = vec3(viewMatrix * worldPlanetCenter);
    vec3 camSpaceLight = vec3(viewMatrix * vec4(lightSource, 1.0));

    // Water shader
    vec2 hitInfo = raySphere(camSpacePlanet, oceanLevel, vec3(0.0f, 0.0f, 0.0f), direction);
    float dstToOcean = hitInfo.x;
    float dstThroughOcean = hitInfo.y;
    float oceanViewDepth = min(dstThroughOcean, depthFromCamera - dstToOcean);
    if (oceanViewDepth > 0) {
      float opticalDepth = 1 - exp(-oceanViewDepth * depthMultiplier);
      vec4 oceanCol = mix(waterColorSurface, waterColorDeep, opticalDepth);

      float alpha = 1 - exp(-oceanViewDepth * oceanCol.w * waterBlendMultiplier);
      vec3 color = mix(vec3(FragColor), vec3(oceanCol), alpha);

      vec3 camSpaceActual = direction * dstToOcean;
      vec3 N = normalize(camSpaceActual - camSpacePlanet);
      if (isSun)
        N = -N;
      vec3 L = normalize(camSpaceLight - camSpaceActual);

      float diffuse = max(dot(N,L),0.0);
      float specular = 0.0;
      if(diffuse>0.0 && !isSun){
        vec3 R = reflect(-L,N);
        vec3 V = normalize(-camSpaceActual);
        float specular_exp = 128;
        specular = pow( max(dot(R,V),0.0), specular_exp );
      }

      if (waterGlow)
        FragColor = vec4(color, 1.0f);
      else if (specularWater)
        FragColor = vec4(color * (diffuse + 0.05)+ vec3(1.0f, 1.0f, 1.0f) * specular * 0.3, 1.0f);
      else
        FragColor = vec4(color * (diffuse + 0.05), 1.0f);
      depthFromCamera = dstToOcean;
    }

    if (hasAtmosphere) {
      // Atmosphere shader
      hitInfo = raySphere(camSpacePlanet, atmosphereHeight, vec3(0.0f, 0.0f, 0.0f), direction);
      float dstToAtmosphere = hitInfo.x;
      float dstThroughAtmosphere = hitInfo.y;
      float atmosphereViewDepth = min(dstThroughAtmosphere, depthFromCamera - dstToAtmosphere);
      if (atmosphereViewDepth > 0) {
        vec3 firstPointInAtmosphere = direction * dstToAtmosphere;
        vec3 intensity = calculateLight(firstPointInAtmosphere, direction, atmosphereViewDepth, camSpacePlanet, camSpaceLight, vec3(FragColor));
        FragColor = vec4(intensity, 1.0);
      }
    }
}
