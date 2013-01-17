#version 130
in vec4 texCoord;
out vec4 fragColor;
uniform float pos_coeff[4];
uniform float yuv_coeff1[7];
uniform float yuv_coeff2[7];
uniform float rgb_coeff[12];
uniform float blend_coeff[6];
uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform sampler2D texture_2;
uniform sampler2D texture_3;
uniform sampler2D texture_4;
uniform sampler2D texture_5;
void main(void)
{
    float y1, u1, v1;
    float y2, u2, v2;
    float r1, g1, b1;
    float r2, g2, b2;
    float red, green, blue;

    vec2 pos = texCoord.st;
    pos.x = pos_coeff[0]+(pos_coeff[1]*pos.x);
    pos.y = pos_coeff[2]+(pos_coeff[3]*pos.y);

    y1 = (yuv_coeff1[0]*texture2D(texture_0, pos).r)+(yuv_coeff1[1]*texture2D(texture_0, pos).g);
    u1 = (yuv_coeff1[2]*texture2D(texture_1, pos).r)+(yuv_coeff1[3]*texture2D(texture_1, pos).g);
    v1 = (yuv_coeff1[4]*texture2D(texture_2, pos).r)+(yuv_coeff1[5]*texture2D(texture_2, pos).a)+(yuv_coeff1[6]*texture2D(texture_2, pos).b);

    y2 = (yuv_coeff2[0]*texture2D(texture_3, pos).r)+(yuv_coeff2[1]*texture2D(texture_3, pos).g);
    u2 = (yuv_coeff2[2]*texture2D(texture_4, pos).r)+(yuv_coeff2[3]*texture2D(texture_4, pos).g);
    v2 = (yuv_coeff2[4]*texture2D(texture_5, pos).r)+(yuv_coeff2[5]*texture2D(texture_5, pos).a)+(yuv_coeff2[6]*texture2D(texture_5, pos).b);

    r1 = (rgb_coeff[0]*y1)+(rgb_coeff[1]*u1)+(rgb_coeff[2]*v1)+rgb_coeff[3];
    g1 = (rgb_coeff[4]*y1)+(rgb_coeff[5]*u1)+(rgb_coeff[6]*v1)+rgb_coeff[7];
    b1 = (rgb_coeff[8]*y1)+(rgb_coeff[9]*u1)+(rgb_coeff[10]*v1)+rgb_coeff[11];

    r2 = (rgb_coeff[0]*y2)+(rgb_coeff[1]*u2)+(rgb_coeff[2]*v2)+rgb_coeff[3];
    g2 = (rgb_coeff[4]*y2)+(rgb_coeff[5]*u2)+(rgb_coeff[6]*v2)+rgb_coeff[7];
    b2 = (rgb_coeff[8]*y2)+(rgb_coeff[9]*u2)+(rgb_coeff[10]*v2)+rgb_coeff[11];

    red = (r1*blend_coeff[0])+(r2*blend_coeff[1]);
    green = (g1*blend_coeff[2])+(g2*blend_coeff[3]);
    blue = (b1*blend_coeff[4])+(b2*blend_coeff[5]);

    fragColor = vec4(red, green, blue, 1.0);
}

/*
red = 1.1643y+1.5958v-0.87066875
green = 1.1643y-0.39173u-0.81290v+0.52954625
blue = 1.1643y+2.017u-1.08126875
*/

