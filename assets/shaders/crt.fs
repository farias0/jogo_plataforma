#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;  // resolution of the scene
uniform float u_time; // current time

// vec2 crt_distort(vec2 uv) {
	
// 	uv = uv * 2.0 - 1.0;
// 	uv *= vec2(1.0 + pow(length(uv) * 0.5, 3.0) * 0.1);
// 	uv = (uv + 1.0) * 0.5;
// 	return uv;
// }

void main() {

	// vec2 uv = gl_FragCoord.xy / u_resolution.xy;
	
	// // Apply CRT distortion
	// uv = crt_distort(uv);
	
	// // Apply scanlines effect
	// float scanline = mod(gl_FragCoord.y, 2.0) * 0.1;
	
	// // Apply color bleeding effect
	// vec3 color = texture(u_texture, uv).rgb;
	// vec3 colorBleed = vec3(color.r, 0.0, color.b);
	
	// // Apply vignette effect
	// vec2 center = vec2(0.5, 0.5);
	// float vignette = smoothstep(0.8, 1.0, distance(uv, center));
	
	// // Combine all effects
	// vec3 finalColor = mix(colorBleed, color, vignette);
	// finalColor *= 1.0 - scanline;
	
	// gl_FragColor = vec4(finalColor, 1.0);

	gl_FragColor = vec4(1.0, 1.0, 1.0, 0.5);
}
