#version 130
in vec4 texCoord;
out vec4 fragColor;
uniform float pos_coeff[4];
uniform float yuv_coeff[7];
uniform float rgb_coeff[12];
uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform sampler2D texture_2;
void main(void)
{
    float y, u, v, red, green, blue;
    vec2 pos = texCoord.st;
    pos.x = pos_coeff[0]+(pos_coeff[1]*pos.x);
    pos.y = pos_coeff[2]+(pos_coeff[3]*pos.y);
    y = (yuv_coeff[0]*texture2D(texture_0, pos).r)+(yuv_coeff[1]*texture2D(texture_0, pos).g);
    u = (yuv_coeff[2]*texture2D(texture_1, pos).r)+(yuv_coeff[3]*texture2D(texture_1, pos).g);
    v = (yuv_coeff[4]*texture2D(texture_2, pos).r)+(yuv_coeff[5]*texture2D(texture_2, pos).a)+(yuv_coeff[6]*texture2D(texture_2, pos).b);
    red = (rgb_coeff[0]*y)+(rgb_coeff[1]*u)+(rgb_coeff[2]*v)+rgb_coeff[3];
    green = (rgb_coeff[4]*y)+(rgb_coeff[5]*u)+(rgb_coeff[6]*v)+rgb_coeff[7];
    blue = (rgb_coeff[8]*y)+(rgb_coeff[9]*u)+(rgb_coeff[10]*v)+rgb_coeff[11];
    fragColor = vec4(red, green, blue, 1.0);
}

/*
red = 1.1643y+1.5958v-0.87066875
green = 1.1643y-0.39173u-0.81290v+0.52954625
blue = 1.1643y+2.017u-1.08126875
*/

