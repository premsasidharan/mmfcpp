/*
 *  Copyright (C) 2012 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include <video_widget.h>

const GLfloat Video_widget::pos_coeff[][4] =
    {
        {
            0.0f, 1.0f, //default 
            0.0f, 1.0f 
        },
        {
            1.0f, -1.0f, //horz flip
            0.0f, 1.0f
        },
        {
            0.0f, 1.0f, //vert flip
            1.0f, -1.0f
        },
        {
            1.0f, -1.0f, //horz & vert flip
            1.0f, -1.0f
        }
    };

const GLfloat Video_widget::yuv_coeff[][7] =
    {
        {//yuv420  
            1.0f, 0.0f,         //y from r
            1.0f, 0.0f,         // u from r
            1.0f, 0.0f, 0.0f,   //v from r
        },
        {//yuy2
            1.0f, 0.0f,         //y from r
            0.0f, 1.0f,         // u from g
            0.0f, 1.0f, 0.0f,   //v from a
        },
        {//uyvy
            0.0f, 1.0f,         //y from g
            1.0f, 0.0f,         // u from r
            0.0f, 0.0f, 1.0f,   //v from b
        }
    };

const GLfloat Video_widget::rgb_coeff[][12] =
    {
        { //y
            1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f, 0.0f
        },
        { //u
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f
        },
        { //v
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f
        },
        { //red
            1.1643f, 0.0f, 1.5958f, -0.87066875f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f
        },
        { //green
            0.0f, 0.0f, 0.0f, 0.0f,
            1.1643f, -0.39173f, -0.81290f, 0.52954625f,
            0.0f, 0.0f, 0.0f, 0.0f
        },
        { //blue
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            1.1643f, 2.017f, 0.0f, -1.08126875f
        },
        { //rgb
            1.1643f, 0.0f, 1.5958f, -0.87066875f,
            1.1643f, -0.39173f, -0.81290f, 0.52954625f,
            1.1643f, 2.017f, 0.0f, -1.08126875f
        },
    };

const GLfloat Video_widget::blend_coeff[][6] =
    {
        { //r1, g1, b1
            1.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 0.0f
        },
        { //r2, g2, b2
            0.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 1.0f
        },
        { //r1+r2, g1+g2, b1+b2
            1.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 1.0f
        },
        { //r1-r2, g1-g2, b1-b2
            1.0f, -1.0f,
            1.0f, -1.0f,
            1.0f, -1.0f
        }
    };

const char Video_widget::vertex_shader_text[] =
"#version 130\n"
"in vec4 inVertex;\n"
"in vec2 inTexCoord;\n"
"out vec4 texCoord;\n"
"void main(void)\n"
"{\n"
"    gl_Position = inVertex;\n"
"    texCoord = vec4(inTexCoord, 0.0, 0.0);\n"
"}\n";

const char Video_widget::yuv_rgb_shader_text[] =
"#version 130\n"
"in vec4 texCoord;\n"
"out vec4 fragColor;\n"
"uniform float pos_coeff[4];\n"
"uniform float yuv_coeff[7];\n"
"uniform float rgb_coeff[12];\n"
"uniform sampler2D texture_0;\n"
"uniform sampler2D texture_1;\n"
"uniform sampler2D texture_2;\n"
"void main(void)\n"
"{\n"
"    float y, u, v, red, green, blue;\n"
"    vec2 pos = texCoord.st;\n"
"    pos.x = pos_coeff[0]+(pos_coeff[1]*pos.x);\n"
"    pos.y = pos_coeff[2]+(pos_coeff[3]*pos.y);\n"
"    y = (yuv_coeff[0]*texture2D(texture_0, pos).r)+(yuv_coeff[1]*texture2D(texture_0, pos).g);\n"
"    u = (yuv_coeff[2]*texture2D(texture_1, pos).r)+(yuv_coeff[3]*texture2D(texture_1, pos).g);\n"
"    v = (yuv_coeff[4]*texture2D(texture_2, pos).r)+(yuv_coeff[5]*texture2D(texture_2, pos).a)+(yuv_coeff[6]*texture2D(texture_2, pos).b);\n"
"    red = (rgb_coeff[0]*y)+(rgb_coeff[1]*u)+(rgb_coeff[2]*v)+rgb_coeff[3];\n"
"    green = (rgb_coeff[4]*y)+(rgb_coeff[5]*u)+(rgb_coeff[6]*v)+rgb_coeff[7];\n"
"    blue = (rgb_coeff[8]*y)+(rgb_coeff[9]*u)+(rgb_coeff[10]*v)+rgb_coeff[11];\n"
"    fragColor = vec4(red, green, blue, 1.0);\n"
"}\n";

const char Video_widget::yuv_rgb_blend_shader_text[] =
"#version 130\n"
"in vec4 texCoord;\n"
"out vec4 fragColor;\n"
"uniform float pos_coeff[4];\n"
"uniform float yuv_coeff1[7];\n"
"uniform float yuv_coeff2[7];\n"
"uniform float rgb_coeff[12];\n"
"uniform float blend_coeff[6];\n"
"uniform sampler2D texture_0;\n"
"uniform sampler2D texture_1;\n"
"uniform sampler2D texture_2;\n"
"uniform sampler2D texture_3;\n"
"uniform sampler2D texture_4;\n"
"uniform sampler2D texture_5;\n"
"void main(void)\n"
"{\n"
"    float y1, u1, v1;\n"
"    float y2, u2, v2;\n"
"    float r1, g1, b1;\n"
"    float r2, g2, b2;\n"
"    float red, green, blue;\n"
"    vec2 pos = texCoord.st;\n"
"    pos.x = pos_coeff[0]+(pos_coeff[1]*pos.x);\n"
"    pos.y = pos_coeff[2]+(pos_coeff[3]*pos.y);\n"
"    y1 = (yuv_coeff1[0]*texture2D(texture_0, pos).r)+(yuv_coeff1[1]*texture2D(texture_0, pos).g);\n"
"    u1 = (yuv_coeff1[2]*texture2D(texture_1, pos).r)+(yuv_coeff1[3]*texture2D(texture_1, pos).g);\n"
"    v1 = (yuv_coeff1[4]*texture2D(texture_2, pos).r)+(yuv_coeff1[5]*texture2D(texture_2, pos).a)+(yuv_coeff1[6]*texture2D(texture_2, pos).b);\n"
"    y2 = (yuv_coeff2[0]*texture2D(texture_3, pos).r)+(yuv_coeff2[1]*texture2D(texture_3, pos).g);\n"
"    u2 = (yuv_coeff2[2]*texture2D(texture_4, pos).r)+(yuv_coeff2[3]*texture2D(texture_4, pos).g);\n"
"    v2 = (yuv_coeff2[4]*texture2D(texture_5, pos).r)+(yuv_coeff2[5]*texture2D(texture_5, pos).a)+(yuv_coeff2[6]*texture2D(texture_5, pos).b);\n"
"    r1 = (rgb_coeff[0]*y1)+(rgb_coeff[1]*u1)+(rgb_coeff[2]*v1)+rgb_coeff[3];\n"
"    g1 = (rgb_coeff[4]*y1)+(rgb_coeff[5]*u1)+(rgb_coeff[6]*v1)+rgb_coeff[7];\n"
"    b1 = (rgb_coeff[8]*y1)+(rgb_coeff[9]*u1)+(rgb_coeff[10]*v1)+rgb_coeff[11];\n"
"    r2 = (rgb_coeff[0]*y2)+(rgb_coeff[1]*u2)+(rgb_coeff[2]*v2)+rgb_coeff[3];\n"
"    g2 = (rgb_coeff[4]*y2)+(rgb_coeff[5]*u2)+(rgb_coeff[6]*v2)+rgb_coeff[7];\n"
"    b2 = (rgb_coeff[8]*y2)+(rgb_coeff[9]*u2)+(rgb_coeff[10]*v2)+rgb_coeff[11];\n"
"    red = (r1*blend_coeff[0])+(r2*blend_coeff[1]);\n"
"    green = (g1*blend_coeff[2])+(g2*blend_coeff[3]);\n"
"    blue = (b1*blend_coeff[4])+(b2*blend_coeff[5]);\n"
"    fragColor = vec4(red, green, blue, 1.0);\n"
"}\n";

const char Video_widget::yuv_rgb_split_shader_text[] =
"#version 130\n"
"in vec4 texCoord;\n"
"out vec4 fragColor;\n"
"uniform int stereo_mode;\n"
"uniform float pos_coeff1[4];\n"
"uniform float pos_coeff2[4];\n"
"uniform float yuv_coeff1[7];\n"
"uniform float yuv_coeff2[7];\n"
"uniform float rgb_coeff[12];\n"
"uniform sampler2D texture_0;\n"
"uniform sampler2D texture_1;\n"
"uniform sampler2D texture_2;\n"
"uniform sampler2D texture_3;\n"
"uniform sampler2D texture_4;\n"
"uniform sampler2D texture_5;\n"
"void main(void)\n"
"{\n"
"    float y1, u1, v1;\n"
"    float y2, u2, v2;\n"
"    float r1, g1, b1;\n"
"    float r2, g2, b2;\n"
"    float red, green, blue;\n"
"    vec2 pos1 = texCoord.st;\n"
"    pos1.x = pos_coeff1[0]+(pos_coeff1[1]*pos1.x);\n"
"    pos1.y = pos_coeff1[2]+(pos_coeff1[3]*pos1.y);\n"
"    vec2 pos2 = texCoord.st;\n"
"    pos2.x = pos_coeff2[0]+(pos_coeff2[1]*pos2.x);\n"
"    pos2.y = pos_coeff2[2]+(pos_coeff2[3]*pos2.y);\n"
"    y1 = (yuv_coeff1[0]*texture2D(texture_0, pos1).r)+(yuv_coeff1[1]*texture2D(texture_0, pos1).g);\n"
"    u1 = (yuv_coeff1[2]*texture2D(texture_1, pos1).r)+(yuv_coeff1[3]*texture2D(texture_1, pos1).g);\n"
"    v1 = (yuv_coeff1[4]*texture2D(texture_2, pos1).r)+(yuv_coeff1[5]*texture2D(texture_2, pos1).a)+(yuv_coeff1[6]*texture2D(texture_2, pos1).b);\n"
"    y2 = (yuv_coeff2[0]*texture2D(texture_3, pos2).r)+(yuv_coeff2[1]*texture2D(texture_3, pos2).g);\n"
"    u2 = (yuv_coeff2[2]*texture2D(texture_4, pos2).r)+(yuv_coeff2[3]*texture2D(texture_4, pos2).g);\n"
"    v2 = (yuv_coeff2[4]*texture2D(texture_5, pos2).r)+(yuv_coeff2[5]*texture2D(texture_5, pos2).a)+(yuv_coeff2[6]*texture2D(texture_5, pos2).b);\n"
"    r1 = (rgb_coeff[0]*y1)+(rgb_coeff[1]*u1)+(rgb_coeff[2]*v1)+rgb_coeff[3];\n"
"    g1 = (rgb_coeff[4]*y1)+(rgb_coeff[5]*u1)+(rgb_coeff[6]*v1)+rgb_coeff[7];\n"
"    b1 = (rgb_coeff[8]*y1)+(rgb_coeff[9]*u1)+(rgb_coeff[10]*v1)+rgb_coeff[11];\n"
"    r2 = (rgb_coeff[0]*y2)+(rgb_coeff[1]*u2)+(rgb_coeff[2]*v2)+rgb_coeff[3];\n"
"    g2 = (rgb_coeff[4]*y2)+(rgb_coeff[5]*u2)+(rgb_coeff[6]*v2)+rgb_coeff[7];\n"
"    b2 = (rgb_coeff[8]*y2)+(rgb_coeff[9]*u2)+(rgb_coeff[10]*v2)+rgb_coeff[11];\n"
"    switch (stereo_mode)\n"
"    {\n"
"        case 1:\n"
"            if ((gl_FragCoord.y/2.0-float(int(gl_FragCoord.y/2.0))) < 0.5) {\n"
"                red = r1;\n"
"                green = g1;\n"
"                blue = b1;\n"
"            } else {\n"
"                red = r2;\n"
"                green = g2;\n"
"                blue = b2;\n"
"            }\n"
"            break;\n"
"        case 2:\n"
"            if (texCoord.x > 0.5) {\n"
"                red = r1;\n"
"                green = g1;\n"
"                blue = b1;\n"
"            } else {\n"
"                red = r2;\n"
"                green = g2;\n"
"                blue = b2;\n"
"            }\n"
"            break;\n"
"        case 3:\n"
"            if (texCoord.x < 0.5) {\n"
"                red = r1;\n"
"                green = g1;\n"
"                blue = b1;\n"
"            } else {\n"
"                red = r2;\n"
"                green = g2;\n"
"                blue = b2;\n"
"            }\n"
"            break;\n"
"        case 4:\n"
"            if (texCoord.y > 0.5) {\n"
"                red = r1;\n"
"                green = g1;\n"
"                blue = b1;\n"
"            } else {\n"
"                red = r2;\n"
"                green = g2;\n"
"                blue = b2;\n"
"            }\n"
"            break;\n"
"        case 5:\n"
"            if (texCoord.y < 0.5) {\n"
"                red = r1;\n"
"                green = g1;\n"
"                blue = b1;\n"
"            } else {\n"
"                red = r2;\n"
"                green = g2;\n"
"                blue = b2;\n"
"            }\n"
"            break;\n"
"        default:\n"
"            red = 0.0f;\n"
"            green = 0.0f;\n"
"            blue = 0.0f;\n"
"    }\n"
"    fragColor = vec4(red, green, blue, 1.0f);\n"
"}\n";

