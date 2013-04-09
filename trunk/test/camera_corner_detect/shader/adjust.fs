#version 130
in vec4 texCoord;
out vec4 fragColor;
uniform sampler2D texture_0;

uniform vec2 coord[8] = vec2[8](vec2(+0.0, +1.0), vec2(+0.0, +2.0), vec2(-1.0, +0.0), vec2(-2.0, +0.0), vec2(+1.0, +0.0), vec2(+2.0, +0.0), vec2(+0.0, -1.0), vec2(+0.0, -2.0));

void main(void)
{
    float r;
    vec2 pos = texCoord.st;
    ivec2 tex_size = textureSize(texture_0, 0);
    vec2 current = vec2(pos.x, (1.0f-pos.y));
    r = texture2D(texture_0, current).r;
    if (r == 0.0f)
    {
        for (int i = 0; (i < coord.length()) && (r == 0.0f); i++) {
            pos.x = current.x+(coord[i].x/float(tex_size.x));
            pos.y = current.y+(coord[i].y/float(tex_size.y));
            r = texture2D(texture_0, pos).r;
        }
    }
     
    fragColor = vec4(r, 0, 0, r);
}

