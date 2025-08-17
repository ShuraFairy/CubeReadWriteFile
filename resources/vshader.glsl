
attribute highp vec4 a_position;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

varying vec2 v_texcoord;

varying vec3 ec_pos;

void main()
{
   gl_Position = projMatrix * mvMatrix * a_position; 
   ec_pos = gl_Position.xyz;
}
