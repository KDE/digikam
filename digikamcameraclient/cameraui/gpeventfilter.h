/* ============================================================
 * File  : gpeventfilter.h
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

#ifndef GPEVENTFILTER_H
#define GPEVENTFILTER_H

#include <qobject.h>

#include "gpfileiteminfo.h"

class QEvent;
class QImage;
class QString;

class CameraUIView;

class GPEventFilter : public QObject
{
    Q_OBJECT
    
public:

    GPEventFilter(QObject *parent);
    ~GPEventFilter();

protected:

    bool eventFilter(QObject *obj, QEvent *e);

private:

    CameraUIView *view_;
    
signals:

    void signalCameraError(const QString&);

    void signalStatusMsg(const QString&);
    void signalProgressVal(int);
    void signalBusy(bool);
    
};

#endif /* GPEVENTFILTER_H */
