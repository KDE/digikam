/* ============================================================
 * File  : gpstatus.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-21
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef GPSTATUS_H
#define GPSTATUS_H


#include <qobject.h>
#include <qstring.h>

extern "C" {
	#include <stdio.h>
	#include <gphoto2.h>
}

class GPStatus : public QObject {

    Q_OBJECT

    friend class GPCamera;

public:

    GPStatus();
    ~GPStatus();
    void cancelOperation();

private:
    
    GPContext   *context;
    static bool  cancel;

private:

    static GPContextFeedback cancel_func(GPContext *context,
                                         void *data);

    static void error_func(GPContext *context,
                           const char *format,  va_list args,
                           void *data);

    static void status_func(GPContext *context,
                            const char *format, va_list args,
                            void *data);

    static unsigned int progress_start_func (GPContext *context,
                                             float target,
                                             const char *format,
                                             va_list args,
                                             void *data);

    static void progress_update_func (GPContext *context,
                                      unsigned int id,
                                      float current,
                                      void *data);

    static void progress_stop_func(GPContext *context,
                                   unsigned int id,
                                   void *data);

    static float target;

};

#endif /* GPSTATUS_H */
