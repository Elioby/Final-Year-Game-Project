FSHo><     textureSampler    �   varying vec2 v_texcoord0;
uniform sampler2D textureSampler;
void main ()
{
  gl_FragColor = texture2D (textureSampler, v_texcoord0);
}

 