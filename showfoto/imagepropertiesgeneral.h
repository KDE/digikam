/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2004-2005 by Gilles Caulier
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

class QWidget;
class QLabel;
class QPixmap;

class KSqueezedTextLabel;
class KURL;
class KFileItem;

class ImagePropertiesGeneral : public QObject
{
    Q_OBJECT
    
public:

    ImagePropertiesGeneral(QWidget* page);
    ~ImagePropertiesGeneral();

    void setCurrentURL(const KURL& url);

private slots:

    void slotGotThumbnail(const KFileItem *, const QPixmap &);
    void slotFailedThumbnail(const KFileItem *);
    
private:
    
    QLabel                       *m_thumbLabel;
    
    KSqueezedTextLabel           *m_filename;
    KSqueezedTextLabel           *m_filetype;
    KSqueezedTextLabel           *m_filedim;
    KSqueezedTextLabel           *m_filedate;
    KSqueezedTextLabel           *m_filesize;
    KSqueezedTextLabel           *m_fileowner;
    KSqueezedTextLabel           *m_filepermissions;
};

#endif /* IMAGEPROPERTIESGENERAL_H */
