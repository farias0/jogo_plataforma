#version 330

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D texture0;

uniform vec2 u_resolution;  // resolution of the scene


vec2 crt_distort(vec2 uv) {
	
	uv = uv * 2.0 - 1.0;
	uv *= vec2(1.0 + pow(length(uv) * 0.5, 3.0) * 0.1);
	uv = (uv + 1.0) * 0.5;
	return uv;
}

void main() {

	vec3 color;
	vec2 uv = gl_FragCoord.xy / u_resolution.xy;


	// Apply CRT distortion
	uv = crt_distort(uv);
	

	color = texture(texture0, uv).rgb;
	

	// Apply scanlines effect
	float scanline = mod(gl_FragCoord.y, 2.0) * 0.1;
	color *= 1.0 - scanline;
	
	
	gl_FragColor = vec4(color, 1.0);
}
