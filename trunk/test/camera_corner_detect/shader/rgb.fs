#version 130
in vec4 texCoord;
out vec4 fragColor;
uniform sampler2D texture_0;
void main(void)
{
    float r = texture2D(texture_0, texCoord.st).r;
    float g = texture2D(texture_0, texCoord.st).g;
    float b = texture2D(texture_0, texCoord.st).b;
    float a = texture2D(texture_0, texCoord.st).a;
    fragColor = vec4(r, g, b, a);
}

