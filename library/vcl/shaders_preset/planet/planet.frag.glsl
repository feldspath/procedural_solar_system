std::string s = R"(
#version 330 core

in struct fragment_data
{
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 uv;
    vec3 eye;
} fragment;

layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture;

in vec4 pos;

uniform vec3 light = vec3(1.0, 1.0, 1.0);

uniform vec3 color = vec3(1.0, 1.0, 1.0); // Unifor color of the object
uniform float alpha = 1.0f; // alpha coefficient
uniform float Ka = 0.4; // Ambient coefficient
uniform float Kd = 0.8; // Diffuse coefficient
uniform float Ks = 0.4f;// Specular coefficient
uniform float specular_exp = 64.0; // Specular exponent
uniform bool use_texture = false;
uniform bool texture_inverse_y = false;
uniform mat4 view;
uniform mat4 perspectiveInverse;

uniform vec4 planetCenter = vec4(0.0f, 0.0f, 0.0f, 1.0f);
uniform float oceanLevel = 1.5f;

uniform vec3 groundColor;

float near = 0.1;
float far  = 100.0;

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

  return vec2(-1.0, 0.0);
}

vec3 cameraDirection(vec2 screenPos) {
  return normalize(vec3(perspectiveInverse * vec4(screenPos, -1.0f, 1.0f)));
}

void main() {
  vec3 newFragCol = groundColor;
  vec3 N = normalize(fragment.normal);
  // if (gl_FrontFacing == false) {
  // 	N = -N;
  // }
	vec3 L = normalize(light-fragment.position);

  float diffuse = max(dot(N,L),0.0);
  float specular = 0.0;
  if(diffuse>0.0){
  	vec3 R = reflect(-L,N);
  	vec3 V = normalize(fragment.eye-fragment.position);
  	specular = pow( max(dot(R,V),0.0), specular_exp );
  }


  vec2 uv_image = vec2(fragment.uv.x, 1.0-fragment.uv.y);
  if(texture_inverse_y) {
  	uv_image.y = 1.0-uv_image.y;
  }
  vec4 color_image_texture = texture(image_texture, uv_image);
  if(use_texture==false) {
  	color_image_texture=vec4(1.0,1.0,1.0,1.0);
  }
  vec3 color_object  = fragment.color * color * color_image_texture.rgb;
  vec3 color_shading = (Ka + Kd * diffuse) * newFragCol + Ks * specular * vec3(1.0, 1.0, 1.0) + 0.0f * color_object;

  FragColor = vec4(color_shading, alpha * color_image_texture.a);







  vec3 direction = cameraDirection(vec2(gl_FragCoord.x * 2 / 1280.0, gl_FragCoord.y * 2 / 1024.0) - 1);
  float depth = LinearizeDepth(gl_FragCoord.z);
  vec2 hitInfo = raySphere(vec3(view * planetCenter), oceanLevel, vec3(0.0, 0.0, 0.0), direction);
  float dstToOcean = hitInfo.x;
  float dstThroughOcean = hitInfo.y;
  float oceanViewDepth = min(dstThroughOcean, length(vec3(view * vec4(fragment.position, 1.0f))) - dstToOcean);
  if (dstToOcean > 0 && oceanViewDepth > 0) {
    float alpha = 1 - exp(-5*oceanViewDepth);
    FragColor = FragColor *(1-alpha) + vec4(0.0, 0.0, 1.0, 1.0) * alpha;
  }
}
)";
