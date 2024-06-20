#version 330

#ifdef GL_ES
precision mediump float;
#endif


uniform sampler2D texture0; // underlying texture, automatically provided

uniform vec2 u_resolution;  // resolution of the texture
// uniform int u_current_time;


void main() {

    vec4 color = texture(texture0, gl_FragCoord.xy / u_resolution);

    //color.a *= 0.5;

    gl_FragColor = vec4(color.rgb, color.a * 0.5);
}