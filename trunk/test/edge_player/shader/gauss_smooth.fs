#version 130
in vec4 texCoord;
out vec4 fragColor;
uniform sampler2D texture_0;

const float coeffs[25] = float[25]((2.0/159.0), (4.0/159.0), (5.0/159.0), (4.0/159.0), (2.0/159.0), 
                          (4.0/159.0), (9.0/159.0), (12.0/159.0), (9.0/159.0), (4.0/159.0),
                          (5.0/159.0), (12.0/159.0), (15.0/159.0), (12.0/159.0), (5.0/159.0),
                          (4.0/159.0), (9.0/159.0), (12.0/159.0), (9.0/159.0), (4.0/159.0),
                          (2.0/159.0), (4.0/159.0), (5.0/159.0), (4.0/159.0), (2.0/159.0));

const vec2 coord[25] = vec2[25](vec2(-2.0, +2.0), vec2(-1.0, +2.0), vec2(+0.0, +2.0), vec2(+1.0, +2.0), vec2(+2.0, +2.0), 
                                vec2(-2.0, +1.0), vec2(-1.0, +1.0), vec2(+0.0, +1.0), vec2(+1.0, +1.0), vec2(+2.0, +1.0), 
                                vec2(-2.0, +0.0), vec2(-1.0, +0.0), vec2(+0.0, +0.0), vec2(+1.0, +0.0), vec2(+2.0, +0.0), 
                                vec2(-2.0, -1.0), vec2(-1.0, -1.0), vec2(+0.0, -1.0), vec2(+1.0, -1.0), vec2(+2.0, -1.0),
                                vec2(-2.0, -2.0), vec2(-1.0, -2.0), vec2(+0.0, -2.0), vec2(+1.0, -2.0), vec2(+2.0, -2.0));
void main(void)
{
    float y = 0.0f;
    vec2 pos = texCoord.st;
    ivec2 tex_size = textureSize(texture_0, 0);
    vec2 current = vec2(pos.x, (1.0f-pos.y));
    for (int i = 0; i < coord.length(); i++) {
        pos.x = current.x+(coord[i].x/float(tex_size.x));
        pos.y = current.y+(coord[i].y/float(tex_size.y));
        y += (coeffs[i]*texture2D(texture_0, pos).r);
    }
    fragColor = vec4(y, y, y, 1.0);
}

