/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-11-11
 * Description : a popup menu with a decorative graphic banner
 *               at the left border.
 *
 * Copyright (C) 1996-2000 the kicker authors.
 * Copyright (C) 2005 Mark Kretschmann <markey@web.de>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dpopupmenu.h"

// Qt includes.

#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QFont>
#include <QFontMetrics>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcomponentdata.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <kiconeffect.h>
#include <kstandarddirs.h>

// Local includes.

#include "version.h"

namespace Digikam
{

static QImage s_dpopupmenu_sidePixmap;
static QColor s_dpopupmenu_sidePixmapColor;

DPopupMenu::DPopupMenu(QWidget* parent)
          : KMenu(parent)
{
    // Must be initialized so that we know the size on first invocation
    if (s_dpopupmenu_sidePixmap.isNull())
        generateSidePixmap();

    setContentsMargins(sidePixmapWidth(), 0, 0, 0);
}

DPopupMenu::~DPopupMenu()
{
}

int DPopupMenu::sidePixmapWidth() const
{
    return s_dpopupmenu_sidePixmap.width();
}

void DPopupMenu::generateSidePixmap()
{
    const QColor newColor = calcPixmapColor();

    if (newColor != s_dpopupmenu_sidePixmapColor)
    {
        s_dpopupmenu_sidePixmapColor = newColor;

        if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
            s_dpopupmenu_sidePixmap.load(KStandardDirs::locate("data","digikam/data/menusidepix-digikam.png"));
        else
            s_dpopupmenu_sidePixmap.load(KStandardDirs::locate("data","showfoto/data/menusidepix-showfoto.png"));

        KIconEffect::colorize(s_dpopupmenu_sidePixmap, newColor, 1.0);

        // Draw version string.

        QPainter p(&s_dpopupmenu_sidePixmap);
        p.rotate(-90.0);
        QFont fnt(KGlobalSettings::generalFont());
        int fntSize = fnt.pointSize();
        if (fntSize > 0)
        {
            fnt.setPointSize(fntSize-2);
        }
        else
        {
            fntSize = fnt.pixelSize();
            fnt.setPixelSize(fntSize-2);
        }
        QFontMetrics fontMt(fnt);
        p.setFont(fnt);
        p.setPen(Qt::white);
        p.drawText(-444, 12, QString(digikam_version_short));
    }
}

QRect DPopupMenu::sideImageRect() const
{
    int frameWidth = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
    return QStyle::visualRect(layoutDirection(), rect(),
                              QRect(frameWidth, frameWidth,
                                    s_dpopupmenu_sidePixmap.width(),
                                    height() - 2*frameWidth));
}

QColor DPopupMenu::calcPixmapColor()
{
    QColor color;
    QColor activeTitle   = QApplication::palette().color(QPalette::Active, QPalette::Background);
    QColor inactiveTitle = QApplication::palette().color(QPalette::Inactive, QPalette::Background);

    // figure out which color is most suitable for recoloring to
    int h1, s1, v1, h2, s2, v2, h3, s3, v3;
    activeTitle.getHsv(&h1, &s1, &v1);
    inactiveTitle.getHsv(&h2, &s2, &v2);
    QApplication::palette().color(QPalette::Active, QPalette::Background).getHsv(&h3, &s3, &v3);

    if ( (qAbs(h1-h3)+qAbs(s1-s3)+qAbs(v1-v3) < qAbs(h2-h3)+qAbs(s2-s3)+qAbs(v2-v3)) &&
         ((qAbs(h1-h3)+qAbs(s1-s3)+qAbs(v1-v3) < 32) || (s1 < 32)) && (s2 > s1))
        color = inactiveTitle;
    else
        color = activeTitle;

    // limit max/min brightness
    int r, g, b;
    color.getRgb(&r, &g, &b);
    int gray = qGray(r, g, b);
    if (gray > 180)
    {
        r = (r - (gray - 180) < 0 ? 0 : r - (gray - 180));
        g = (g - (gray - 180) < 0 ? 0 : g - (gray - 180));
        b = (b - (gray - 180) < 0 ? 0 : b - (gray - 180));
    }
    else if (gray < 76)
    {
        r = (r + (76 - gray) > 255 ? 255 : r + (76 - gray));
        g = (g + (76 - gray) > 255 ? 255 : g + (76 - gray));
        b = (b + (76 - gray) > 255 ? 255 : b + (76 - gray));
    }
    color.setRgb(r, g, b);

    return color;
}

void DPopupMenu::setMinimumSize(const QSize & s)
{
    KMenu::setMinimumSize(s.width() + s_dpopupmenu_sidePixmap.width(), s.height());
}

void DPopupMenu::setMaximumSize(const QSize & s)
{
    KMenu::setMaximumSize(s.width() + s_dpopupmenu_sidePixmap.width(), s.height());
}

void DPopupMenu::setMinimumSize(int w, int h)
{
    KMenu::setMinimumSize(w + s_dpopupmenu_sidePixmap.width(), h);
}

void DPopupMenu::setMaximumSize(int w, int h)
{
    KMenu::setMaximumSize(w + s_dpopupmenu_sidePixmap.width(), h);
}

void DPopupMenu::paintEvent(QPaintEvent* e)
{
    KMenu::paintEvent(e);

    {
        // scope for QPainter object

        generateSidePixmap();

        QPainter p(this);
        p.setClipRegion(e->region());

        QStyleOptionFrame frOpt;
        frOpt.init(this);
        frOpt.lineWidth    = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
        frOpt.midLineWidth = 0;
        style()->drawPrimitive(QStyle::PE_FrameMenu, &frOpt, &p, this);

        QRect r = sideImageRect();
        r.setTop(r.bottom()-s_dpopupmenu_sidePixmap.height()+1);
        if (r.intersects( e->rect()))
        {
            QRect drawRect = r.intersect(e->rect()).intersect( sideImageRect());
            QRect pixRect  = drawRect;
            pixRect.translate(-r.left(), -r.top());
            p.drawImage(drawRect.topLeft(), s_dpopupmenu_sidePixmap, pixRect);
        }
    }
}

}  // namespace Digikam
