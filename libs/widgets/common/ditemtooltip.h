/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : tool tip widget for iconview or thumbbar items
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

#ifndef DITEMTOOLTIP_H
#define DITEMTOOLTIP_H

// Qt includes.

#include <QFrame>
#include <QString>
#include <QEvent>
#include <QResizeEvent>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DItemToolTipPriv;

class DIGIKAM_EXPORT DItemToolTip : public QFrame
{
public:

    DItemToolTip();
    virtual ~DItemToolTip();

protected:

    bool    event(QEvent*);
    void    resizeEvent(QResizeEvent*);
    void    paintEvent(QPaintEvent*);
    QString breakString(const QString&);
    void    updateToolTip();
    void    reposition();
    void    renderArrows();

    virtual QRect   repositionRect()=0;
    virtual QString tipContents()=0;

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

    DItemToolTipPriv* const d;
};

}  // namespace Digikam

#endif /* DITEMTOOLTIP_H */
