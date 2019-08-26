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
* \file pal_os_event_arduino.cpp
*
* \brief   This file implements the platform abstraction layer APIs for os event/scheduler.
*
* \ingroup  grPAL
*
* @{
*/

#include "pal_os_event.h"

LIBRARY_EXPORTS pal_os_event_t * pal_os_event_create(register_callback callback,
                                                     void * callback_args)
{
    
}

LIBRARY_EXPORTS void pal_os_event_destroy(pal_os_event_t * pal_os_event)
{

}

LIBRARY_EXPORTS void pal_os_event_register_callback_oneshot(pal_os_event_t * p_pal_os_event,
                                                            register_callback callback,
                                                            void * callback_args,
                                                            uint32_t time_us)
{

}

void pal_os_event_trigger_registered_callback(void)
{


    
}

LIBRARY_EXPORTS void pal_os_event_start(pal_os_event_t * p_pal_os_event,
                                        register_callback callback,
                                        void * callback_args)
{

}

LIBRARY_EXPORTS void pal_os_event_stop(pal_os_event_t * p_pal_os_event)
{

}