FSH�5�'      �   varying vec4 v_color0;
varying vec3 v_normal;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = (v_color0.xyz * (dot (
    normalize(v_normal)
  , vec3(1.0, 3.0, 0.0)) / 3.162278));
  gl_FragColor = tmpvar_1;
}

 