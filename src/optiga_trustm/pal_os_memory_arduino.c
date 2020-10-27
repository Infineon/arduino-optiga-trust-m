/**
* \copyright
* MIT License
*
* Copyright (c) 2019 Infineon Technologies AG
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE
*
* \endcopyright
*
* \author Infineon Technologies AG
*
* \file pal_os_memory_arduino.cpp
*
* \brief   This file implements the platform abstraction layer APIs for os memory management
*
* \ingroup  grPAL
*
* @{
*/


#include "pal_os_memory.h"

LIBRARY_EXPORTS void * pal_os_malloc(uint32_t block_size)
{
    return malloc(block_size);
}

LIBRARY_EXPORTS void * pal_os_calloc(uint32_t number_of_blocks , uint32_t block_size)
{
    return calloc(number_of_blocks, block_size);
}

LIBRARY_EXPORTS void pal_os_free(void * block)
{
    free(block);
}

LIBRARY_EXPORTS void pal_os_memcpy(void * p_destination, const void * p_source, uint32_t size)
{
    (void) memcpy(p_destination, p_source, size);
}

LIBRARY_EXPORTS void pal_os_memset(void * p_buffer, uint32_t value, uint32_t size)
{
    (void) memset(p_buffer, value, size);
}
