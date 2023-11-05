#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;  // resolution of the scene
uniform vec2 u_focus_point; // focus point of the effect
uniform float u_duration; // how long the effect should run
uniform float u_current_time; // current timestamp of the effect
uniform int u_is_close; // if the effect is to close in a point, or to open from it

void main() {
    
    float time_normalized = u_current_time / u_duration;
    if (u_is_close == 1) time_normalized = 1.0 - time_normalized;

    // I don't know why the x2 is needed here
    float current_radius = u_resolution.y * time_normalized * 2.0;  

    float distance_from_focus = sqrt(pow(gl_FragCoord.y - u_focus_point.y, 2.0) +
                                    pow(gl_FragCoord.x - u_focus_point.x, 2.0));
    
    if (abs(distance_from_focus) > current_radius) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0);
    }
}
