#version 130
in vec4 texCoord;
out vec4 fragColor;
uniform sampler2D texture_0;

const float pi = 3.14159265358979323846;
const float pi_mul_2 = (pi*2.0);
/*const float theta_360 = (pi*2.0);
const float theta_22_5 = (pi/8.0);
const float theta_67_5 = ((3.0*pi)/8.0);
const float theta_112_5 = ((5.0*pi)/8.0);
const float theta_157_5 = ((7.0*pi)/8.0);
const float theta_202_5 = ((9.0*pi)/8.0);
const float theta_247_5 = ((11.0*pi)/8.0);
const float theta_292_5 = ((13.0*pi)/8.0);
const float theta_337_5 = ((15.0*pi)/8.0);*/

const float theta_360 = 360.0f;
const float theta_22_5 = 22.5f;
const float theta_67_5 = 67.5f;
const float theta_112_5 = 112.5f;
const float theta_157_5 = 157.5f;
const float theta_202_5 = 202.5f;
const float theta_247_5 = 247.5f;
const float theta_292_5 = 292.5f;
const float theta_337_5 = 337.5f;

const float coeffs_fx[9] = float[9](-1.0f, +0.0f, +1.0f,
                                    -2.0f, +0.0f, +2.0f,
                                    -1.0f, +0.0f, +1.0f);

const float coeffs_fy[9] = float[9](+1.0f, +2.0f, +1.0f, 
                                    +0.0f, +0.0f, +0.0f,
                                    -1.0f, -2.0f, -1.0f);

const vec2 offset[9] = vec2[9](vec2(-1.0f, +1.0f), vec2(+0.0f, +1.0f), vec2(+1.0f, +1.0f), 
                               vec2(-1.0f, +0.0f), vec2(+0.0f, +0.0f), vec2(+1.0f, +0.0f), 
                               vec2(-1.0f, -1.0f), vec2(+0.0f, -1.0f), vec2(+1.0f, -1.0f));
void main(void)
{
    vec2 pos;
    float theta;
    float y = 0.0f, gx = 0.0f, gy = 0.0f;
    ivec2 tex_size = textureSize(texture_0, 0);
    vec2 current = vec2(texCoord.s, (1.0f-texCoord.t));
    for (int i = 0; i < offset.length(); i++) {
        pos.x = current.x+(offset[i].x/float(tex_size.x));
        pos.y = current.y+(offset[i].y/float(tex_size.y));
        y = texture2D(texture_0, pos).r;
        gx += (y*coeffs_fx[i]);
        gy += (y*coeffs_fy[i]);
    }
	y = sqrt((gx*gx)+(gy*gy));
    theta = degrees(atan(abs(gy), abs(gx)));
    if (gx >= 0.0f && gy >= 0.0f)
        theta += 0.0f;
    else if (gx < 0.0f && gy >= 0.0f)
        theta = 180.0f-theta;
    else if (gx < 0.0f && gy < 0.0f)
        theta = 180.0f+theta;
    else
        theta = 360.0f-theta;
    if ((theta >= theta_22_5 && theta < theta_67_5)||(theta >= theta_202_5 && theta < theta_247_5))
        theta = 0.25f;
    else if ((theta >= theta_67_5 && theta < theta_112_5)||(theta >= theta_247_5 && theta < theta_292_5))
        theta = 0.5f;
    else if ((theta >= theta_112_5 && theta < theta_157_5)||(theta >= theta_292_5 && theta < theta_337_5))
        theta = 0.75f;
    else
        theta = 0.0f;
    fragColor = vec4(y, theta, 0.0f, 1.0f);
}

