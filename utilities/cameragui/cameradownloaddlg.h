/* ============================================================
 * File  : cameradownloaddlg.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-18
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

#ifndef CAMERADOWNLOADDLG_H
#define CAMERADOWNLOADDLG_H

#include <kdialogbase.h>
#include <kfileitem.h>
#include <kurl.h>

namespace KIO
{
class Slave;
class Job;
}

class QLabel;
class QProgressBar;

class CameraDownloadDlg : public KDialogBase
{
    Q_OBJECT
    
public:

    CameraDownloadDlg(QWidget *parent, KIO::Slave *slave,
                      const KFileItemList& items,
                      const KURL& destURL);
    ~CameraDownloadDlg();

private:


    KIO::Slave*    m_slave;
    KFileItemList  m_itemList;
    KURL           m_destURL;
    
    QLabel*        m_label;
    QProgressBar*  m_currProgress;
    QProgressBar*  m_totalProgress;

    int            m_count;
    bool           m_overwriteAll;
    bool           m_autoSkip;

protected slots:

    void slotCancel();
    
private slots:

    void slotProcessNext();
    void slotResult(KIO::Job* job);
    void slotPercent(KIO::Job*, unsigned long val);
 
};

#endif /* CAMERADOWNLOADDLG_H */
