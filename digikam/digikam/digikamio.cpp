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

#include <kdebug.h>
#include <klocale.h>
#include <klargefile.h>
#include <kmessagebox.h>
#include <kdatastream.h>
#include <kio/renamedlg.h>
#include <kio/global.h>
#include <kio/slaveinterface.h>

#include <qtimer.h>
#include <qfile.h>
#include <qprogressdialog.h>

extern "C" 
{
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
}

#include "albumsettings.h"
#include "busyprogressbar.h"
#include "digikamio.h"

DigikamIO::DigikamIO(const KURL::List& srcList, const KURL& dest,
                     bool move, bool showProgress)
    : KIO::Job(false)
{
    m_srcList = srcList;
    m_dest    = dest;
    m_move    = move;
    m_showProgress = showProgress;
    m_autoSkip     = false;
    m_overwriteAll = false;
    m_progress     = 0;

    if (m_showProgress)
    {
        m_progress = new QProgressDialog();
        m_progress->setBar(new BusyProgressBar(m_progress));
        connect(m_progress, SIGNAL(canceled()),
                SLOT(slotCanceled()));
        m_progress->show();
    }        
    
    QTimer::singleShot(0, this, SLOT(slotProcessNext()));
}

DigikamIO::~DigikamIO()
{
    if (m_progress)
    {
        delete m_progress;
    }
}

void DigikamIO::slotResult(KIO::Job *job)
{
    subjobs.remove(job);
    Q_ASSERT( subjobs.isEmpty() );

    if (job->error())
    {
        job->showErrorDialog(0);
        emitResult();
        return;
    }

    slotProcessNext();
}

void DigikamIO::slotProcessNext()
{
    if (m_srcList.isEmpty())
    {
        emitResult();
        return;
    }

    KURL src(m_srcList.first());
    m_srcList.pop_front();
    KURL dest(m_dest);
        
    bool useInternalCopy = false;
    bool srcIsDir  = false;
    bool destIsDir = false;
    
    if (src.isLocalFile())
    {
        // stat the src
        KDE_struct_stat buff_src;
        if ( KDE_stat( QFile::encodeName(src.path()), &buff_src ) == -1 )
        {
            if ( errno == EACCES )
                KMessageBox::error(0, i18n("Access denied to source\n%1")
                                   .arg(src.prettyURL()));
            else
                KMessageBox::error(0, i18n("Source\n%1\ndoes not exist")
                                   .arg(src.prettyURL()));
            emitResult();
            return;
        }

        // stat the dest
        KDE_struct_stat buff_dest;
        if ( KDE_stat( QFile::encodeName(dest.path()), &buff_dest ) == -1 )
        {
            if ( errno == EACCES )
                KMessageBox::error(0, i18n("Access denied to destination\n%1")
                                   .arg(dest.prettyURL()));
            else
                KMessageBox::error(0, i18n("Destination folder\n%1\ndoes not exist")
                                   .arg(dest.prettyURL()));
            emitResult();
            return;
        }

        // check that dest is directory if src is directory
        srcIsDir  = S_ISDIR( buff_src.st_mode );
        destIsDir = S_ISDIR( buff_dest.st_mode );

        if ( srcIsDir && !destIsDir )
        {
            KMessageBox::error(0, i18n("Source is a directory, but destination is not."));
            emitResult();
            return;
        }

        // check that src is not a parent of dest
        if ( src.isParentOf( dest ) )
        {
            KMessageBox::error(0, i18n("Trying to copy/move a folder to its subfolder"));
            emitResult();
            return;
        }
    
        // check if source url is within album library path
        KURL libURL = AlbumSettings::instance()->getAlbumLibraryPath();
        if ( libURL.isParentOf( src ) )
        {
            useInternalCopy = true;
        }

    }        

    if (useInternalCopy)
    {
        // copy/move within album library. handle it using the digikamio kioslave
        
        if ( destIsDir )
            dest.addPath( src.fileName() );

        QString newDestPath;

        bool overwrite = m_overwriteAll;
        bool skip      = false;
        
        if (!m_overwriteAll)
        {
            KDE_struct_stat buff_dest;
            while ( KDE_stat( QFile::encodeName(dest.path()), &buff_dest ) == 0 )
            {
                if (m_autoSkip)
                {
                    skip = true;
                    break;
                }

                destIsDir = S_ISDIR( buff_dest.st_mode );

                KIO::RenameDlg_Result result = KIO::R_CANCEL;
                
                if (srcIsDir)
                {
                    result =
                        KIO::open_RenameDlg(i18n("Rename Folder"), src.path(), dest.path(),
                                            KIO::RenameDlg_Mode(KIO::M_MULTI |
                                                                KIO::M_SKIP),
                                            newDestPath);
                }
                else
                {
                    result =
                        KIO::open_RenameDlg(i18n("Rename File"), src.path(), dest.path(),
                                            KIO::RenameDlg_Mode(KIO::M_MULTI |
                                                                KIO::M_OVERWRITE |
                                                                KIO::M_SKIP),
                                            newDestPath);
                }

                dest = KURL(newDestPath);

                switch (result)
                {
                case KIO::R_CANCEL:
                {
                    emitResult();
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
            QTimer::singleShot(0, this, SLOT(slotProcessNext()));
            return;
        }
        
        src.setProtocol("digikamio");
        dest.setProtocol("digikamio");

        int permissions = -1;

        QByteArray packedArgs;
        QDataStream stream( packedArgs, IO_WriteOnly );
        stream << src << dest << permissions << (Q_INT8) overwrite;
        
        KIO::SimpleJob* job =
            new KIO::SimpleJob(src, m_move ? KIO::CMD_RENAME : KIO::CMD_COPY,
                               packedArgs, false);

        if (m_showProgress)
        {
            connect(job, SIGNAL(infoMessage(KIO::Job*, const QString&)),
                    SLOT(slotInfoMessage(KIO::Job*, const QString&)));
        }
        
        addSubjob(job);
    }
    else
    {
        // copy/move from external source. use kio::copyjob to handle this

        KIO::CopyJob* job = 0;
        
        if ( m_move )
           job = KIO::move(src, dest, false);
        else
           job = KIO::copy(src, dest, false);

        if (m_showProgress)
        {
            connect(job, SIGNAL(copying(KIO::Job*, const KURL&, const KURL&)),
                    SLOT(slotCopying(KIO::Job*, const KURL&, const KURL&)));
            connect(job, SIGNAL(moving(KIO::Job*, const KURL&, const KURL&)),
                    SLOT(slotMoving(KIO::Job*, const KURL&, const KURL&)));
        }

        addSubjob(job);
    }
}

void DigikamIO::slotCanceled()
{
    deleteLater();
}

void DigikamIO::slotCopying(KIO::Job*, const KURL& from, const KURL&)
{
    if (!m_showProgress || !m_progress)
        return;

    m_progress->setLabelText(i18n("Copying\n%1")
                             .arg(from.prettyURL()));
}

void DigikamIO::slotMoving(KIO::Job*, const KURL& from, const KURL&)
{
    if (!m_showProgress || !m_progress)
        return;
    
    m_progress->setLabelText(i18n("Moving\n%1")
                             .arg(from.prettyURL()));
}

void DigikamIO::slotInfoMessage(KIO::Job*, const QString& msg)
{
    if (!m_showProgress || !m_progress)
        return;
    
    m_progress->setLabelText(msg);
}

#include "digikamio.moc"

