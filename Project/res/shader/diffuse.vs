VSH    !8� u_model     u_modelViewProj   �  attribute highp vec3 a_normal;
attribute highp vec3 a_position;
varying highp vec3 v_fragpos;
varying highp vec3 v_normal;
uniform mat4 u_model[32];
uniform highp mat4 u_modelViewProj;
void main ()
{
  highp vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = a_position;
  gl_Position = (u_modelViewProj * tmpvar_1);
  highp vec4 tmpvar_2;
  tmpvar_2.w = 1.0;
  tmpvar_2.xyz = a_position;
  v_fragpos = (u_model[0] * tmpvar_2).xyz;
  v_normal = a_normal;
}

 