/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-08-30
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef DIGIKAMIO_H
#define DIGIKAMIO_H

#include <kio/job.h>
#include <kurl.h>

class QProgressDialog;

class DigikamIO : public KIO::Job
{
    Q_OBJECT

public:

    DigikamIO(const KURL::List& srcList, const KURL& dest,
              bool move, bool showProgress=true);
    ~DigikamIO();

private:

    KURL::List        m_srcList;
    KURL              m_dest;
    bool              m_move;
    bool              m_showProgress;
    bool              m_overwriteAll;
    bool              m_autoSkip;
    QProgressDialog  *m_progress;
    
private slots:

    void slotProcessNext();
    void slotCanceled();
    void slotCopying(KIO::Job*, const KURL& from, const KURL& to);
    void slotMoving(KIO::Job*, const KURL& from, const KURL& to);
    void slotInfoMessage(KIO::Job*, const QString& msg);
    
protected slots:

    void slotResult(KIO::Job *job);
};

#endif /* DIGIKAMIO_H */
