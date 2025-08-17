uniform float u_red;
uniform float u_green;
uniform float u_blue;
uniform vec3 lightPos;

varying vec3 ec_pos;


void main(void)
{
    vec3 normal = normalize(cross(dFdx(ec_pos), dFdy(ec_pos)));

    vec3 L = normalize(lightPos - ec_pos);
    float NL = max(dot(normalize(normal), L), 0.0);
    //vec3 color = vec3(0.39, 1.0, 0.0);
    vec3 color = vec3(u_red, u_green, u_blue);
    vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);
    gl_FragColor = vec4(col, 1.0);
}



