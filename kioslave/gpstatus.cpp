/* ============================================================
 * File  : gpstatus.cpp
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

#include <unistd.h>

#include "gpstatus.h"
#include "gpmessages.h"

bool  GPStatus::cancel = false;

GPStatus::GPStatus() : QObject() {

  context = gp_context_new();
  cancel = false;

  gp_context_set_cancel_func(context, cancel_func, 0);
  gp_context_set_error_func(context, error_func, 0);
  gp_context_set_status_func(context, status_func, 0);
  gp_context_set_progress_funcs(context,
                                progress_start_func,
                                progress_update_func,
                                progress_stop_func,
                                0);
}

GPStatus::~GPStatus() {

    if (context) 
        gp_context_unref(context);

}

void GPStatus::cancelOperation()
{
    cancel = true;    
}

GPContextFeedback GPStatus::cancel_func(GPContext *, void *)
{
    return (cancel ? GP_CONTEXT_FEEDBACK_CANCEL : GP_CONTEXT_FEEDBACK_OK);
}

void GPStatus::error_func(GPContext *, const char *format,
                          va_list args, void *) {

  char buf[4096] = "";
  int nSize = vsnprintf( buf, 4096, format, args );
  if( nSize > 4094 ) nSize = 4094;
  buf[nSize] = '\0';

  QString error;
  error = error.fromLocal8Bit(buf);
  GPMessages::gpMessagesWrapper()->emit errorMessage(error);

}

void GPStatus::status_func (GPContext *, const char *format,
                            va_list args, void *) {

  char buf[4096] = "";
  int nSize = vsnprintf( buf, 4096, format, args );
  if( nSize > 4094 ) nSize = 4094;
  buf[nSize] = '\0';

  QString status;
  status = status.fromLocal8Bit(buf);
  GPMessages::gpMessagesWrapper()->emit statusChanged(status);

}

unsigned int GPStatus::progress_start_func(GPContext *,
                                           float target,
                                           const char *format,
                                           va_list args,
                                           void *) {
  char buf[4096] = "";
  int nSize = vsnprintf( buf, 4096, format, args );
  if( nSize > 4094 ) nSize = 4094;
  buf[nSize] = '\0';

  QString prog;
  prog = prog.fromLocal8Bit(buf);

  GPMessages::gpMessagesWrapper()->emit progressStart((int)target);

  return 0;
}

void GPStatus::progress_update_func(GPContext *,
                                    unsigned int,
                                    float current,
                                    void *)
{

   GPMessages::gpMessagesWrapper()->emit progressUpdate((int)current);
}

void  GPStatus::progress_stop_func(GPContext *,
                                   unsigned int,
                                   void *) {

    //GPMessages::gpMessagesWrapper()->emit progressUpdate(0);
}


#include "gpstatus.moc"
