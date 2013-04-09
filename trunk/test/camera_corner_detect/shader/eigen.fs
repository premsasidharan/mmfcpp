#version 130
in vec4 texCoord;
out vec4 fragColor;
uniform sampler2D texture_0;
void main(void)
{
    vec2 current = vec2(texCoord.x, (1.0f-texCoord.y));
    float a = texture2D(texture_0, current).r;
    float c = texture2D(texture_0, current).g;
    float b = texture2D(texture_0, current).b;
    float temp1 = a-c;
    float temp2 = a+c;
    float temp3 = sqrt((temp1*temp1)+(4.0*b*b));
    float lamda1 = (temp2+temp3)/2.0;
    float lamda2 = (temp2-temp3)/2.0;
    float temp4 = lamda1+lamda2;
    float R = (lamda1*lamda2)-(0.06*temp4*temp4);

    /*vec2 pos = vec2(texCoord.s, (1.0f-texCoord.t));
    float ix_2 = texture2D(texture_0, pos).r;
    float iy_2 = texture2D(texture_0, pos).g;
    float ix_iy = texture2D(texture_0, pos).b;

    float temp1 = (ix_2+iy_2);
    float temp2 = sqrt((temp1*temp1)-4*((ix_2*iy_2)-(ix_iy*ix_iy)));
    float lamda1 = (temp1+temp2)/2.0;
    float lamda2 = (temp1-temp2)/2.0;

    float temp3 = (lamda1+lamda2);
    float R = (lamda1*lamda2)-(0.06*temp3*temp3);*/

    /*if (R < threshold)
    {
        R = 0.0f;
    }*/

    fragColor = vec4(R, R, R, 1.0f);
}

