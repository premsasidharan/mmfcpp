/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#include "complex.h"

complex complex::operator/(const complex& data)
{
    complex temp = data.conjugate();
    return ((*this)*temp)/data.abs_square();
}

complex operator/(const double value, const complex& data)
{
    complex temp = (data.conjugate()*value)/data.abs_square();
    return temp;
}

void complex::operator*=(const complex& data)
{
    complex temp = (*this)*data;
    *this = temp;
}

void complex::operator/=(const complex& data)
{
    complex temp = (*this)/data;
    *this = temp;
}

std::ostream& operator<<(std::ostream& os, const complex& data)
{
    os << "(" << data.real_data << ", " << data.img_data << ")";
    return os;
}

