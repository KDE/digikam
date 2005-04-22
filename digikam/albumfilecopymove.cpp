/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-07
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

#include <qstringlist.h>
#include <qtimer.h>
#include <qprogressdialog.h>
#include <qfile.h>

#include <kio/renamedlg.h>
#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
}

#include "album.h"
#include "albummanager.h"
#include "albumdb.h"

#include "albumfilecopymove.h"

AlbumFileCopyMove::AlbumFileCopyMove(PAlbum *srcAlbum, PAlbum *destAlbum,
                                     const QStringList& fileList, bool move)
{
    m_move        = move;
    m_srcAlbum    = srcAlbum;
    m_destAlbum   = destAlbum;
    m_srcFileList = fileList;

    m_timer    = 0;
    m_progress = 0;
    m_overwriteAll = false;
    m_autoSkip     = false;
    
    if (fileList.isEmpty())
    {
        deleteLater();
        return;
    }
    else
    {
        m_timer = new QTimer();
        connect(m_timer, SIGNAL(timeout()),
                SLOT(slotNext()));
        m_timer->start(0, true);

        m_count = 0;
        m_countTotal = fileList.size();
        
        m_progress = new QProgressDialog();
        connect(m_progress, SIGNAL(canceled()),
                SLOT(slotCanceled()));
        
        if (m_move)
            m_progress->setLabelText(i18n("Moving file\n%1")
                                     .arg(m_srcFileList.first()));
        else
            m_progress->setLabelText(i18n("Copying file\n%1")
                                     .arg(m_srcFileList.first()));
        m_progress->setProgress(m_count, m_countTotal);
        m_progress->show();
    }
}

AlbumFileCopyMove::~AlbumFileCopyMove()
{
    if (m_timer)
        delete m_timer;
    if (m_progress)
        delete m_progress;
}

bool AlbumFileCopyMove::rename(PAlbum* album, const QString& srcFile,
                               const QString& destFile)
{
    QString srcPath = album->getKURL().path(1) + srcFile;
    QString dFile(destFile);
    QString newDestPath;

    bool overwrite = false;
    
    while (fileStat(album, dFile))
    {
        QString destPath = album->getKURL().path(1) + destFile;

        KIO::RenameDlg_Result result =
            KIO::open_RenameDlg(i18n("Rename File"), srcPath, destPath,
                                KIO::RenameDlg_Mode(KIO::M_SINGLE |
                                                    KIO::M_OVERWRITE),
                                newDestPath);

        dFile = KURL(newDestPath).fileName();

        switch (result)
        {
        case KIO::R_CANCEL:
        {
            return false;
        }
        case KIO::R_OVERWRITE:
        {
            overwrite = true;
            break;
        }
        default:
            break;
        }

        if (overwrite)
            break;
    }

    AlbumDB* db = AlbumManager::instance()->albumDB();
    if (fileMove(album, album, srcFile, dFile))
    {
        db->moveItem(album, srcFile,
                     album, dFile);
        return true;
    }

    KMessageBox::error(0, i18n("Failed to rename file\n%1")
                       .arg(srcFile), i18n("Rename Failed"));
    return false;    
}

void AlbumFileCopyMove::slotNext()
{
    if (m_srcFileList.isEmpty())
    {
        emit signalFinished();
        deleteLater();
        return;
    }

    QString srcFile  = m_srcFileList.first();
    QString destFile = srcFile;

    QString srcPath = m_srcAlbum->getKURL().path(1) + srcFile;
    QString newDestPath;

    bool overwrite = false;
    bool skip      = false;

    if (!m_overwriteAll)
    {
        while (fileStat(m_destAlbum, destFile))
        {
            if (m_autoSkip)
            {
                skip = true;
                break;
            }
        
            QString destPath = m_destAlbum->getKURL().path(1) + destFile;

            KIO::RenameDlg_Result result =
                KIO::open_RenameDlg(i18n("Rename File"), srcPath, destPath,
                                    KIO::RenameDlg_Mode(KIO::M_MULTI |
                                                        KIO::M_OVERWRITE |
                                                        KIO::M_SKIP),
                                    newDestPath);

            destFile = KURL(newDestPath).fileName();

            switch (result)
            {
            case KIO::R_CANCEL:
            {
                deleteLater();
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

    m_srcFileList.pop_front();

    if (!skip)
    {
        AlbumDB* db = AlbumManager::instance()->albumDB();

        if (m_move)
        {
            if (fileMove(m_srcAlbum, m_destAlbum, srcFile, destFile))
            {
                db->moveItem(m_srcAlbum, srcFile,
                             m_destAlbum, destFile);
            }
            else
            {
                int result = KMessageBox::
                             warningContinueCancel(0, i18n("Failed to move file\n%1")
                                                   .arg(srcFile), i18n("Move Failed"));
                if (result == KMessageBox::Cancel)
                {
                    emit signalFinished();
                    deleteLater();
                    return;
                }
            }
        }
        else
        {
            if (fileCopy(m_srcAlbum, m_destAlbum, srcFile, destFile))
            {
                db->copyItem(m_srcAlbum, srcFile,
                             m_destAlbum, destFile);
            }
            else
            {
                int result = KMessageBox::
                             warningContinueCancel(0, i18n("Failed to copy file\n%1")
                                                   .arg(srcFile), i18n("Copy Failed"));
                if (result == KMessageBox::Cancel)
                {
                    emit signalFinished();
                    deleteLater();
                    return;
                }
            }
        }

        m_count++;
    }
    else
    {
        m_countTotal--;
    }

    if (m_move)
        m_progress->setLabelText(i18n("Moving file\n%1")
                                 .arg(m_srcFileList.first()));
    else
        m_progress->setLabelText(i18n("Copying file\n%1")
                                 .arg(m_srcFileList.first()));
    m_progress->setProgress(m_count, m_countTotal);
    
    m_timer->start(0, true);
}

void AlbumFileCopyMove::slotCanceled()
{
    emit signalFinished();
    deleteLater();    
}

bool AlbumFileCopyMove::fileCopy(PAlbum* srcAlbum, PAlbum* destAlbum,
                                 const QString& srcFile, const QString& destFile)
{
    QString src  = srcAlbum->getKURL().path(1)  + srcFile;
    QString dest = destAlbum->getKURL().path(1) + destFile;

    QFile sFile(src);
    QFile dFile(dest);

    if ( !sFile.open(IO_ReadOnly) )
        return false;
    
    if ( !dFile.open(IO_WriteOnly) )
    {
        sFile.close();
        return false;
    }

    const int MAX_IPC_SIZE = (1024*32);
    char buffer[MAX_IPC_SIZE];

    Q_LONG len;
    while ((len = sFile.readBlock(buffer, MAX_IPC_SIZE)) != 0)
    {
        if (len == -1 || dFile.writeBlock(buffer, (Q_ULONG)len) == -1)
        {
            sFile.close();
            dFile.close();
            return false;
        }
    }

    sFile.close();
    dFile.close();
    
    return true;
}

bool AlbumFileCopyMove::fileMove(PAlbum* srcAlbum, PAlbum* destAlbum,
                                 const QString& srcFile, const QString& destFile)
{
    QString src  = srcAlbum->getKURL().path(1)  + srcFile;
    QString dest = destAlbum->getKURL().path(1) + destFile;
    
    return (::rename(QFile::encodeName(src), QFile::encodeName(dest)) == 0);
}

bool AlbumFileCopyMove::fileStat(PAlbum* album, const QString& name)
{
    QString path = album->getKURL().path(1) + name;
    
    struct stat info;
    return (::stat(QFile::encodeName(path), &info) == 0);
}

#include "albumfilecopymove.moc"

