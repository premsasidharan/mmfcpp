#version 130
in vec4 texCoord;
out vec4 fragColor;
uniform sampler2D texture_0;

uniform float coeffs[25] = float[25]((2.0/159.0), (4.0/159.0), (5.0/159.0), (4.0/159.0), (2.0/159.0), 
                          (4.0/159.0), (9.0/159.0), (12.0/159.0), (9.0/159.0), (4.0/159.0),
                          (5.0/159.0), (12.0/159.0), (15.0/159.0), (12.0/159.0), (5.0/159.0),
                          (4.0/159.0), (9.0/159.0), (12.0/159.0), (9.0/159.0), (4.0/159.0),
                          (2.0/159.0), (4.0/159.0), (5.0/159.0), (4.0/159.0), (2.0/159.0));

uniform vec2 coord[25] = vec2[25](vec2(-2.0, +2.0), vec2(-1.0, +2.0), vec2(+0.0, +2.0), vec2(+1.0, +2.0), vec2(+2.0, +2.0), 
                                vec2(-2.0, +1.0), vec2(-1.0, +1.0), vec2(+0.0, +1.0), vec2(+1.0, +1.0), vec2(+2.0, +1.0), 
                                vec2(-2.0, +0.0), vec2(-1.0, +0.0), vec2(+0.0, +0.0), vec2(+1.0, +0.0), vec2(+2.0, +0.0), 
                                vec2(-2.0, -1.0), vec2(-1.0, -1.0), vec2(+0.0, -1.0), vec2(+1.0, -1.0), vec2(+2.0, -1.0),
                                vec2(-2.0, -2.0), vec2(-1.0, -2.0), vec2(+0.0, -2.0), vec2(+1.0, -2.0), vec2(+2.0, -2.0));

/*uniform float coeffs[9] = float[9]((1.0/9.0), (1.0/9.0), (1.0/9.0), 
                                   (1.0/9.0), (1.0/9.0), (1.0/9.0),
                                   (1.0/9.0), (1.0/9.0), (1.0/9.0));

uniform vec2 coord[9] = vec2[9](vec2(-1.0, +1.0), vec2(+0.0, +1.0), vec2(+1.0, +1.0), 
                                vec2(-1.0, +0.0), vec2(+0.0, +0.0), vec2(+1.0, +0.0), 
                                vec2(-1.0, -1.0), vec2(+0.0, -1.0), vec2(+1.0, -1.0));*/

void main(void)
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    vec2 pos = texCoord.st;
    ivec2 tex_size = textureSize(texture_0, 0);
    vec2 current = vec2(pos.x, (1.0f-pos.y));
    for (int i = 0; i < coord.length(); i++) {
        pos.x = current.x+(coord[i].x/float(tex_size.x));
        pos.y = current.y+(coord[i].y/float(tex_size.y));
        r += (coeffs[i]*texture2D(texture_0, pos).r);
        g += (coeffs[i]*texture2D(texture_0, pos).g);
        b += (coeffs[i]*texture2D(texture_0, pos).b);
    }
    fragColor = vec4(r, g, b, 1.0);
}

