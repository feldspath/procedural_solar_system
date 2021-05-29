#version 330 core

in struct fragment_data
{
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 uv;

	vec3 eye;
} fragment;

in vec3 localCoords;
in vec3 localNormal;

layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture;

uniform vec3 light = vec3(1.0, 1.0, 1.0);

uniform vec3 color = vec3(1.0, 1.0, 1.0); // Unifor color of the object
uniform float alpha = 1.0f; // alpha coefficient
uniform float Ka = 0.4; // Ambient coefficient
uniform float Kd = 0.8; // Diffuse coefficient
uniform float Ks = 0.4f;// Specular coefficient
uniform float specular_exp = 64.0; // Specular exponent
uniform bool use_texture = true;
uniform bool texture_inverse_y = false;

uniform float textureScale = 1.0f;
uniform float textureSharpness = 1.0f;
uniform float normalMapInfluence = 0.2f;

uniform vec3 steepColor;
uniform vec3 flatLowColor;
uniform vec3 flatHighColor;

uniform bool isSun;

vec4 colorFromTriplanarMapping() {
  vec2 uvxy = localCoords.xy / textureScale;
  vec2 uvyz = localCoords.yz / textureScale;
  vec2 uvxz = localCoords.xz / textureScale;

  vec4 texturexy = texture(image_texture, uvxy);
  vec4 textureyz = texture(image_texture, uvyz);
  vec4 texturexz = texture(image_texture, uvxz);

  vec3 weights = pow(abs(localNormal), vec3(textureSharpness));
  weights /= weights.x + weights.y + weights.z;

  return texturexy * weights.z + textureyz * weights.x + texturexz * weights.y;
}


void main()
{
  vec4 textureSample = colorFromTriplanarMapping();

  vec3 normMap = normalize(vec3(textureSample) * 2.0f - 1.0f);
  vec3 N = normalize(fragment.normal);
  N = normalize(N + normMap * normalMapInfluence);
  if (isSun)
    N = -N;
	vec3 L = normalize(light-fragment.position);

	float diffuse = max(dot(N,L),0.0);
	float specular = 0.0;
	if(diffuse>0.0 && !isSun){
		vec3 R = reflect(-L,N);
		vec3 V = normalize(fragment.eye-fragment.position);
		specular = pow( max(dot(R,V),0.0), specular_exp );
	}


  vec4 color_image_texture=vec4(1.0,1.0,1.0,1.0);
  //vec4 color_image_texture=textureSample;

	//vec3 color_object  = fragment.color * color * color_image_texture.rgb;
  float altitude = fragment.color.x * 2-1.0;
  float slopeBlending = fragment.color.y;
  // vec3 slopeColor = mix(flatColor, steepColor, slopeBlending);
  // vec3 heightColor = mix(flatColor, steepColor, clamp(altitude / 0.1, 0.0, 1.0));
  // vec3 color_object = color*mix(slopeColor, heightColor, 0.8);
  vec3 plateauColor = mix(flatLowColor, flatHighColor, clamp(altitude / 0.1, 0.0, 1.0));
  vec3 color_object = color*mix(plateauColor, steepColor, slopeBlending);




	vec3 color_shading = (Ka + Kd * diffuse) * color_object + Ks * specular * vec3(1.0, 1.0, 1.0);

	FragColor = vec4(color_shading, alpha * color_image_texture.a);
}
