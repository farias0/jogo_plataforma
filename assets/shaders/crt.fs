#version 330

#ifdef GL_ES
precision mediump float;
#endif


uniform sampler2D texture0; // underlying texture, automatically provided

uniform vec2 u_resolution;  // resolution of the scene


vec2 crt_distort(vec2 uv) {
	
	uv = uv * 2.0 - 1.0;
	uv *= vec2(1.0 + pow(length(uv) * 0.5, 3.0) * 0.1);
	uv = (uv + 1.0) * 0.5;
	
	// Discard pixels that have been wrapped around
	if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
		discard;
	}
	
	return uv;
}

vec3 scanlines(vec3 pixelColors, float pixelY) {

	return pixelColors *= 1.0 - mod(pixelY, 2.0) * 0.1;
}

void main() {

	vec3 colors;
	vec2 uv = gl_FragCoord.xy / u_resolution.xy;


	// Apply CRT distortion
	uv = crt_distort(uv);
	

	colors = texture(texture0, uv).rgb;
	

	// Apply scanlines
	colors = scanlines(colors, gl_FragCoord.y);
	
	
	gl_FragColor = vec4(colors, 1.0);
}
