/* ============================================================
 * File  : cameradownloaddlg.cpp
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

#include <klocale.h>
#include <kmessagebox.h>
#include <kio/scheduler.h>
#include <kio/slave.h>
#include <kio/jobclasses.h>
#include <kio/renamedlg.h>

#include <qcstring.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qtimer.h>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include "cameradownloaddlg.h"

CameraDownloadDlg::CameraDownloadDlg(QWidget *parent, KIO::Slave *slave,
                                     const KFileItemList& items,
                                     const KURL& destURL)
    : KDialogBase(Plain, i18n("Downloading..."), Cancel, Cancel,
                  parent, 0, false, true),
      m_slave(slave), m_itemList(items), m_destURL(destURL)
{
    QVBoxLayout* lay = new QVBoxLayout(plainPage(), 5, 5);

    m_label = new QLabel(plainPage());
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_label->setMinimumWidth(300);
    lay->addWidget(m_label);
    m_label->setText(i18n("Downloading %1 items...").arg(items.count()));

    m_currProgress = new QProgressBar(plainPage());
    m_currProgress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    lay->addWidget(m_currProgress);

    m_totalProgress = new QProgressBar(plainPage());
    m_totalProgress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    lay->addWidget(m_totalProgress);

    m_totalProgress->setTotalSteps(items.count());
    
    resize( sizeHint() );
    setMaximumHeight(sizeHint().height());

    m_count        = 0;
    m_overwriteAll = false;
    m_autoSkip     = false;

    QTimer::singleShot(0, this, SLOT(slotProcessNext()));
}

CameraDownloadDlg::~CameraDownloadDlg()
{
    
}

void CameraDownloadDlg::slotProcessNext()
{
    if (m_itemList.isEmpty())
    {
        close();
        return;
    }

    KFileItem* fileItem = m_itemList.first();
    m_itemList.remove();

    KURL dest(m_destURL);
    dest.addPath(fileItem->url().fileName());
    QString destPath(dest.path());
    QString srcPath(fileItem->url().url());
    QString newDestPath;

    bool overwrite = false;
    bool skip      = false;

    if (!m_overwriteAll)
    {
        struct stat info;
        while (::stat(QFile::encodeName(destPath), &info) == 0)
        {
            if (m_autoSkip)
            {
                skip = true;
                break;
            }

            KIO::RenameDlg dlg(this, i18n("Rename File"), srcPath, destPath,
                               KIO::RenameDlg_Mode(KIO::M_MULTI |
                                                   KIO::M_OVERWRITE |
                                                   KIO::M_SKIP));
            
            int result = dlg.exec();

            destPath = dlg.newDestURL().path();

            switch (result)
            {
            case KIO::R_CANCEL:
            {
                KDialogBase::slotCancel();
                return;
            }
            case KIO::R_SKIP:
            {
                skip = true;
                break;
            }
            case KIO::R_AUTO_SKIP:
            {
                m_autoSkip = true;
                skip       = true;
                break;
            }
            case KIO::R_OVERWRITE:
            {
                overwrite = true;
                break;
            }
            case KIO::R_OVERWRITE_ALL:
            {
                m_overwriteAll = true;
                overwrite      = true;
            }
            default:
                break;
            }

            if (skip || overwrite)
                break;
        }
    }

    if (skip)
    {
        m_count++;
        m_label->setText(i18n("Skipping File %1 ...")
                         .arg(fileItem->url().fileName()));
        m_totalProgress->setProgress(m_count);
        QTimer::singleShot(0, this, SLOT(slotProcessNext()));
        return;
    }
    else
    {
        m_label->setText(i18n("Downloading File %1 ...")
                         .arg(fileItem->url().fileName()));
    
        QByteArray ba;
        QDataStream ds(ba, IO_WriteOnly);
        ds << 3;
        ds << fileItem->url().path();
        ds << destPath;
    
        KIO::SimpleJob* job = new KIO::SimpleJob(fileItem->url(),
                                                 KIO::CMD_SPECIAL,
                                                 ba, false);
        KIO::Scheduler::assignJobToSlave(m_slave, job);

        connect(job, SIGNAL(result(KIO::Job*)),
                SLOT(slotResult(KIO::Job*)));
        connect(job, SIGNAL(percent(KIO::Job*, unsigned long)),
                SLOT(slotPercent(KIO::Job*, unsigned long)));
    }
}

void CameraDownloadDlg::slotResult(KIO::Job* job)
{
    if (job->error())
    {
        QString msg(i18n("The following error occurred; "
                         "would you like to continue?\n"));
        msg += job->errorString();
        
        if (KMessageBox::warningContinueCancel(this, msg)
            != KMessageBox::Continue)
        {
            KDialogBase::slotCancel();
            return;
        }
    }

    m_count++;
    m_totalProgress->setProgress(m_count);
    
    slotProcessNext();
}

void CameraDownloadDlg::slotPercent(KIO::Job*, unsigned long val)
{
    m_currProgress->setProgress(val);    
}

void CameraDownloadDlg::slotCancel()
{
    KURL url;
    url.setProtocol("digikamcamera");
    url.setPath("/");

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << 2;
    KIO::SimpleJob* job = new KIO::SimpleJob(url, KIO::CMD_SPECIAL,
                                             ba, false);
    KIO::Scheduler::assignJobToSlave(m_slave, job);

    KDialogBase::slotCancel();
}

#include "cameradownloaddlg.moc"
