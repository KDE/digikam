/* ============================================================
 * File  : digikamcameraprocess.h
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

#ifndef DIGIKAMCAMERAPROCESS_H
#define DIGIKAMCAMERAPROCESS_H

#include <qobject.h>

class KProcess;

class DigikamCameraProcess : public QObject
{
    Q_OBJECT

public:

    DigikamCameraProcess(QObject *parent);
    ~DigikamCameraProcess();

    void start();
    void kill();
    void stop();

private:

    KProcess *process_;

private slots:
    
    void slotProcessEnded();
    
};

#endif /* DIGIKAMCAMERAPROCESS_H */
