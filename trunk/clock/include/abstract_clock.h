/*
 *  Copyright (C) 2011 Prem Sasidharan.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * published by the Free Software Foundation.
*/

#ifndef _ABSTRACT_CLOCK_H_
#define _ABSTRACT_CLOCK_H_

class Abstract_clock
{
public:
    Abstract_clock(const char* _name);
    virtual ~Abstract_clock();
    
public:
    const char* name() const { return clk_name; };
    
private:
    char* clk_name;
};

#endif
