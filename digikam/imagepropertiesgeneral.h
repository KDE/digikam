/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2004 by Gilles Caulier
 *
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
 * ============================================================ */

#ifndef IMAGEPROPERTIESGENERAL_H
#define IMAGEPROPERTIESGENERAL_H

#include <qobject.h>
#include <qguardedptr.h>

class QWidget;
class QLabel;
class KSqueezedTextLabel;
class ThumbnailJob;

class ImagePropertiesGeneral : public QObject
{
    Q_OBJECT
    
public:

    ImagePropertiesGeneral(QWidget* page);
    ~ImagePropertiesGeneral();

    void setCurrentURL(const KURL& url);

private slots:

    void slotGotThumbnail(const KURL&, const QPixmap& pix, const KFileMetaInfo*);
    void slotFailedThumbnail(const KURL&);
    
private:
    
    QLabel                       *m_thumbLabel;
    QLabel                       *m_albumLabel;
    QLabel                       *m_commentsLabel;
    QLabel                       *m_tagsLabel;
    
    KSqueezedTextLabel           *m_filename;
    KSqueezedTextLabel           *m_filetype;
    KSqueezedTextLabel           *m_filedim;
    KSqueezedTextLabel           *m_filedate;
    KSqueezedTextLabel           *m_filesize;
    KSqueezedTextLabel           *m_fileowner;
    KSqueezedTextLabel           *m_filepermissions;
    KSqueezedTextLabel           *m_filealbum;
    KSqueezedTextLabel           *m_filecomments;
    KSqueezedTextLabel           *m_filetags;
    
    QGuardedPtr<ThumbnailJob>     m_thumbJob;
};

#endif /* IMAGEPROPERTIESGENERAL_H */
