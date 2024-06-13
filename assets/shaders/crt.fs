#version 330

#ifdef GL_ES
precision mediump float;
#endif


uniform sampler2D texture0; // underlying texture, automatically provided

uniform vec2 u_resolution;  // resolution of the scene


/*
	Applies CRT distortion to the screen.
*/
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

/*
	Greys out every other line to simulate scanlines.
*/
vec3 scanlines(vec3 pixelColors, float pixelY) {

	if (mod(pixelY, 5.0) < 2.0) {
		return pixelColors * 0.9;
	}
	else {
		return pixelColors;
	}
}

/*
	Shift the colors of the pixel so it looks like the screen
	is made of big RGB pixels.
*/
vec3 big_rgb(vec3 pixelColors, float pixelX) {

	switch(int(mod(pixelX, 3.0))) {
		case 0:
			pixelColors.r *= 1.1;
			pixelColors.g *= 0.8;
			pixelColors.b *= 0.8;
			break;
		case 1:
			pixelColors.r *= 0.8;
			pixelColors.g *= 1.1;
			pixelColors.b *= 0.8;
			break;
		case 2:
			pixelColors.r *= 0.8;
			pixelColors.g *= 0.8;
			pixelColors.b *= 1.1;
			break;		
	}

	return pixelColors;
}

void main() {

	vec3 colors;
	vec2 uv = gl_FragCoord.xy / u_resolution.xy;


	// Apply CRT distortion
	uv = crt_distort(uv);
	

	colors = texture(texture0, uv).rgb;
	

	// Apply Big RBG effect
	colors = big_rgb(colors, gl_FragCoord.x);


	// Apply scanlines
	colors = scanlines(colors, gl_FragCoord.y);
	
	
	gl_FragColor = vec4(colors, 1.0);
}
