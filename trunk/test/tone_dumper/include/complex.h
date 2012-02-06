/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _COMPLEX_H
#define _COMPLEX_H

#include <math.h>
#include <iostream>

class complex
{
public:
    complex():real_data(0.0), img_data(0.0) {};

    complex(const complex& data):real_data(data.real_data), img_data(data.img_data) {};	
    complex& operator=(double value) { real_data = value; img_data = 0.0; return *this; };
    complex& operator=(const complex& data) { real_data = data.real_data; img_data = data.img_data; return *this; };

    ~complex() {};

public:
    double img_part() const { return img_data; };
    double real_part() const { return real_data; };

    double abs() const { return sqrt(abs_square()); }
    double arg() const { return (real_data == 0.0)?M_PI/2.0:tanh(img_data/real_data); }

    complex conjugate() const { return complex(real_data, -img_data); };

    static complex cartesian(const double rd, const double id) { return complex(rd, id); };
    static complex polar(const double r, const double theta) { return complex(r*cos(theta), r*sin(theta)); };

    complex operator+(const complex& data) { return complex(real_data+data.real_data, img_data+data.img_data);  };
    complex operator+(const double value) { return complex(real_data+value, img_data); };
    friend complex operator+(const double value, const complex& data) { return complex(value+data.real_data, data.img_data); };

    complex operator-(const complex& data) { return complex(real_data-data.real_data, img_data-data.img_data); };
    complex operator-(const double value) { return complex(real_data-value, -img_data); };
    friend complex operator-(const double value, const complex& data) { return complex(value-data.real_data, -data.img_data); };

    complex operator*(const complex& data) { return complex(real_data*data.real_data-img_data*data.img_data, real_data*data.img_data+img_data*data.real_data); };
    complex operator*(const double value) { return complex(real_data*value, img_data*value); };
    friend complex operator*(const double value, const complex& data) { return complex(data.real_data*value, data.img_data*value); };

    complex operator/(const complex& data);
    complex operator/(const double value) { return complex(real_data/value, img_data/value); };
    friend complex operator/(const double value, const complex& data);

    void operator+=(const double value) { real_data += value; };
    void operator+=(const complex& data) { real_data += data.real_data; img_data += data.img_data;};

    void operator-=(const double value) { real_data -= value; };
    void operator-=(const complex& data) { real_data -= data.real_data; img_data -= data.img_data; };

    void operator*=(const double value) { real_data *= value; img_data *= value; };
    void operator*=(const complex& data);

    void operator/=(const double value) { real_data /= value, img_data /= value; };
    void operator/=(const complex& data);

    friend std::ostream& operator<<(std::ostream& os, const complex& data);

private:
    complex(double rd, double id):real_data(rd), img_data(id) {};
    double abs_square() const { return (real_data*real_data+img_data*img_data); };

private:
    double real_data;
    double img_data;
};


#endif
