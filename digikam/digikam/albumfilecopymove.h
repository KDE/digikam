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

#ifndef ALBUMFILECOPYMOVE_H
#define ALBUMFILECOPYMOVE_H

#include <qobject.h>
#include <qstringlist.h>

class QProgressDialog;
class QTimer;
class PAlbum;
class KURL;


class AlbumFileCopyMove : public QObject
{
    Q_OBJECT
    
public:
    
    AlbumFileCopyMove(PAlbum *srcAlbum, PAlbum *destAlbum,
                      const QStringList& fileList, bool move);
    ~AlbumFileCopyMove();

    static bool rename(PAlbum* album, const QString& srcFile,
                       const QString& destFile);
    
private:

    static bool fileCopy(PAlbum* srcAlbum, PAlbum* destAlbum,
                         const QString& srcFile, const QString& destFile);
    static bool fileMove(PAlbum* srcAlbum, PAlbum* destAlbum,
                         const QString& srcFile, const QString& destFile);
    static bool fileStat(PAlbum* album, const QString& name);

    PAlbum           *m_srcAlbum;
    PAlbum           *m_destAlbum;
    QStringList       m_srcFileList;
    QTimer           *m_timer;
    QProgressDialog  *m_progress;
    bool              m_move;
    int               m_count;
    int               m_countTotal;
    bool              m_autoSkip;
    bool              m_overwriteAll;

signals:

    void signalFinished();
    
private slots:

    void slotNext();
    void slotCanceled();
};
    
#endif /* ALBUMFILECOPYMOVE_H */
