/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/
#include <string.h>
#include <abstract_clock.h>

Abstract_clock::Abstract_clock(const char* _name)
    :clk_name(0)
{
    int length = 1+strlen(_name);
    clk_name = new char[length];
    strncpy(clk_name, _name, length);
}

Abstract_clock::~Abstract_clock()
{
    delete [] clk_name;
    clk_name = 0;
}
