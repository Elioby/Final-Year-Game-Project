VSH    �5�' u_model     u_modelViewProj   �  attribute vec3 a_normal;
attribute vec3 a_position;
varying vec4 v_color0;
varying vec3 v_normal;
varying vec3 v_position;
uniform mat4 u_model[32];
uniform mat4 u_modelViewProj;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = a_position;
  gl_Position = (u_modelViewProj * tmpvar_1);
  v_position = gl_Position.xyz;
  vec4 tmpvar_2;
  tmpvar_2.w = 0.0;
  tmpvar_2.xyz = a_normal;
  v_normal = (u_model[0] * tmpvar_2).xyz;
  v_color0 = vec4(0.862, 0.862, 0.862, 1.0);
}

 