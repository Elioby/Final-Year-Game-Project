VSH         u_modelViewProj   �   attribute highp vec2 a_position;
uniform highp mat4 u_modelViewProj;
void main ()
{
  highp vec4 tmpvar_1;
  tmpvar_1.zw = vec2(0.0, 1.0);
  tmpvar_1.xy = a_position;
  gl_Position = (u_modelViewProj * tmpvar_1);
}

 