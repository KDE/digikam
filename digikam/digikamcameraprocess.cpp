/* ============================================================
 * File  : digikamcameraprocess.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-19
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

#include <kprocess.h>
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <dcopclient.h>
#include <qcstring.h>

#include "digikamcameraprocess.h"


DigikamCameraProcess::DigikamCameraProcess(QObject *parent)
    : QObject(parent)
{
    process_ = new KProcess;
    *process_ << "digikamcameraclient";

    connect(process_, SIGNAL(processExited(KProcess*)),
            this,     SLOT(slotProcessEnded()));
}

DigikamCameraProcess::~DigikamCameraProcess()
{
    disconnect(process_, SIGNAL(processExited(KProcess*)),
               this,     SLOT(slotProcessEnded()));
    stop();
    delete process_;
}

void DigikamCameraProcess::start()
{
    if (!process_->start()) {
        kdError() << "DigikamCameraProcess: "
                  << "Failed to launch Camera Process" << endl;
        KMessageBox::error(0, i18n("Failed to launch Camera Client. "
                                   "You would not be able to access the camera(s) "
                                   "(Make sure that `digikamcameraclient' is"
                                   " installed correctly)"));
    }
    else
        kdDebug() << "DigikamCameraProcess: "
                  << "Launched Camera Process" << endl;
}

void DigikamCameraProcess::kill()
{
    process_->kill();
}

void DigikamCameraProcess::stop()
{
    QByteArray data, replyData;
    QCString replyType;
    
    DCOPClient *client = kapp->dcopClient();
    if (!client->call("digikamcameraclient", "DigikamCameraClient",
                      "close()", data, replyType, replyData))
        kdError() << "DigikamCameraProcess: DCOP communication error" << endl;
    else
        kdDebug() << "DigikamCameraProcess: Stopped client" << endl;
}

void DigikamCameraProcess::slotProcessEnded()
{
    if (KMessageBox::questionYesNo(0, i18n("Camera Client Died Unexpectedly. "
                                           "Shall I restart the process? "
                                           "(Note: Otherwise you would not be able to "
                                           "access the camera(s))")) ==
        KMessageBox::Yes) {
        start();
    }
}

#include "digikamcameraprocess.moc"
