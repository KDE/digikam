/* ============================================================
 * File  : camerathumbjob.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-13
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef CAMERATHUMBJOB_H
#define CAMERATHUMBJOB_H

#include <kio/job.h>
#include <kfileitem.h>

namespace KIO
{
class Slave;
}

class CameraThumbJobPriv;

class CameraThumbJob : public KIO::Job
{
    Q_OBJECT
    
public:

    CameraThumbJob(KIO::Slave *slave, const KFileItemList& items, int size);
    ~CameraThumbJob();

signals:

    void signalThumbnail(const KFileItem* item, const QPixmap& pix);
    void signalCompleted();
    
protected slots:

    void slotResult(KIO::Job *job);
    void slotData(KIO::Job *job, const QByteArray &data);

private:

    void processNext();
    void createShmSeg();
    
    CameraThumbJobPriv *d;
};

#endif /* CAMERATHUMBJOB_H */
