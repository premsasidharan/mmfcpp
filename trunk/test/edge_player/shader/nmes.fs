#version 130
#define ne_dir 0
#define n_dir  1
#define nw_dir 2
#define e_dir  3
#define w_dir  5
#define se_dir 6
#define s_dir  7
#define sw_dir 8
in vec4 texCoord;
out vec4 fragColor;
uniform sampler2D texture_0;
const int first[]=int[](e_dir, se_dir, n_dir, ne_dir);
const int second[]=int[](w_dir, nw_dir, s_dir, sw_dir);
const vec2 offset[9] = vec2[9](vec2(-1.0f, +1.0f), vec2(+0.0f, +1.0f), vec2(+1.0f, +1.0f), 
                               vec2(-1.0f, +0.0f), vec2(+0.0f, +0.0f), vec2(+1.0f, +0.0f),
                               vec2(-1.0f, -1.0f), vec2(+0.0f, -1.0f), vec2(+1.0f, -1.0f));
void main(void)
{
    vec2 pos1, pos2;
    ivec2 tex_size = textureSize(texture_0, 0);
    vec2 current = vec2(texCoord.s, 1.0f-texCoord.t);
    float color = texture2D(texture_0, current).r;
    float angle = texture2D(texture_0, current).g;
    int i = int(ceil(4.0*angle));
    pos1.x = current.x+(offset[first[i]].x/float(tex_size.x));
    pos1.y = current.y+(offset[first[i]].y/float(tex_size.y));
    pos2.x = current.x+(offset[second[i]].x/float(tex_size.x));
    pos2.y = current.y+(offset[second[i]].y/float(tex_size.y));
    if ((color <= texture2D(texture_0, pos1).r) || (color <= texture2D(texture_0, pos2).r))
        color = 0.0f;
    fragColor = vec4(color, color, color, 1.0f);
}

