/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-09
 * Description : thumbbar tool tip
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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
 *
 * ============================================================ */

#ifndef THUMBBARTOOLTIP_H
#define THUMBBARTOOLTIP_H

// Qt includes.

#include <QFrame>
#include <QString>
#include <QEvent>
#include <QResizeEvent>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class ThumbBarView;
class ThumbBarItem;
class ThumbBarToolTipPriv;

class DIGIKAM_EXPORT ThumbBarToolTipSettings
{
public:

    ThumbBarToolTipSettings()
    {
        showToolTips   = true;
        showFileName   = true;
        showFileDate   = false;
        showFileSize   = false;
        showImageType  = false;
        showImageDim   = true;
        showPhotoMake  = true;
        showPhotoDate  = true;
        showPhotoFocal = true;
        showPhotoExpo  = true;
        showPhotoMode  = true;
        showPhotoFlash = false;
        showPhotoWB    = false;
    };

    bool showToolTips;
    bool showFileName;
    bool showFileDate;
    bool showFileSize;
    bool showImageType;
    bool showImageDim;
    bool showPhotoMake;
    bool showPhotoDate;
    bool showPhotoFocal;
    bool showPhotoExpo;
    bool showPhotoMode;
    bool showPhotoFlash;
    bool showPhotoWB;
};

// --------------------------------------------------------

class DIGIKAM_EXPORT ThumbBarToolTip : public QFrame
{
public:

    ThumbBarToolTip(ThumbBarView* view);
    ~ThumbBarToolTip();

    void setItem(ThumbBarItem* item);
    ThumbBarItem* item() const;

protected:

    bool event(QEvent*);
    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent*);
    ThumbBarToolTipSettings& toolTipSettings() const;
    QString breakString(const QString& str);

    virtual QString tipContents();

protected:

    const int m_maxStringLen;

    QString   m_headBeg;
    QString   m_headEnd;
    QString   m_cellBeg;
    QString   m_cellMid;
    QString   m_cellEnd;
    QString   m_cellSpecBeg;
    QString   m_cellSpecMid;
    QString   m_cellSpecEnd;

private:

    void    reposition();
    void    renderArrows();

private:

    ThumbBarToolTipPriv* const d;
};

}  // namespace Digikam

#endif /* THUMBBARTOOLTIP_H */
