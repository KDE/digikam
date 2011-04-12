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
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QFont>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QStyle>
#include <QStyleOptionFrame>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kdebug.h>

// Local includes

#include "version.h"

namespace Digikam
{

class DPopupMenu::DPopupMenuPriv
{
public:

    DPopupMenuPriv() 
        : gradientWidth(0)
    {
    };

    int     gradientWidth;

    QFont   fontAppName;
    QFont   fontVersion;
};

DPopupMenu::DPopupMenu(QWidget* parent)
    : KMenu(parent), d(new DPopupMenuPriv)
{
    d->fontAppName = KGlobalSettings::generalFont();
    d->fontVersion = KGlobalSettings::generalFont();

    d->fontAppName.setBold(true);
    d->fontAppName.setPixelSize(13);
    d->fontVersion.setBold(false);
    d->fontVersion.setPixelSize(11);

    // has to be an odd number to get the text centered
    d->gradientWidth = d->fontAppName.pixelSize() + 2;

    setContentsMargins(d->gradientWidth, 0, 0, 0);
}

DPopupMenu::~DPopupMenu()
{
    delete d;
}

QColor DPopupMenu::calcPixmapColor() const
{
    QColor color;
    QColor activeTitle   = QApplication::palette().color(QPalette::Active,   QPalette::Background);
    QColor inactiveTitle = QApplication::palette().color(QPalette::Inactive, QPalette::Background);

    // figure out which color is most suitable for recoloring to
    int h1, s1, v1, h2, s2, v2, h3, s3, v3;
    activeTitle.getHsv(&h1, &s1, &v1);
    inactiveTitle.getHsv(&h2, &s2, &v2);
    QApplication::palette().color(QPalette::Active, QPalette::Background).getHsv(&h3, &s3, &v3);

    if ( (qAbs(h1-h3)+qAbs(s1-s3)+qAbs(v1-v3) < qAbs(h2-h3)+qAbs(s2-s3)+qAbs(v2-v3)) &&
         ((qAbs(h1-h3)+qAbs(s1-s3)+qAbs(v1-v3) < 32) || (s1 < 32)) && (s2 > s1))
    {
        color = inactiveTitle;
    }
    else
    {
        color = activeTitle;
    }

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

void DPopupMenu::setMinimumSize(const QSize& s)
{
    KMenu::setMinimumSize(s.width() + d->gradientWidth, s.height());
}

void DPopupMenu::setMaximumSize(const QSize& s)
{
    KMenu::setMaximumSize(s.width() + d->gradientWidth, s.height());
}

void DPopupMenu::setMinimumSize(int w, int h)
{
    KMenu::setMinimumSize(w + d->gradientWidth, h);
}

void DPopupMenu::setMaximumSize(int w, int h)
{
    KMenu::setMaximumSize(w + d->gradientWidth, h);
}

void DPopupMenu::paintEvent(QPaintEvent* e)
{
    KMenu::paintEvent(e);

    {
        // scope for QPainter object
        QPainter p(this);

        QStyleOptionFrame frOpt;
        frOpt.init(this);
        frOpt.lineWidth    = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
        frOpt.midLineWidth = 0;
        style()->drawPrimitive(QStyle::PE_FrameMenu, &frOpt, &p, this);

        renderSidebarGradient(&p);
    }
}

void DPopupMenu::renderSidebarGradient(QPainter* p)
{
    p->setRenderHint(QPainter::TextAntialiasing);
    p->setPen(Qt::white);

    int frameWidth = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
    kDebug() << frameWidth;
    QRect drawRect = QStyle::visualRect(layoutDirection(), rect(),
                                        QRect(frameWidth, frameWidth,
                                        d->gradientWidth, height() - 2*frameWidth));
    p->setClipRect(drawRect);

    // ----------------------------------------

    // draw gradient
    QLinearGradient linearGrad;
    linearGrad.setCoordinateMode(QGradient::ObjectBoundingMode);
    linearGrad.setStart(0.0, 0.0);
    linearGrad.setFinalStop(0.0, 1.0);
    linearGrad.setColorAt(0, QColor(255, 255, 255, 25));
    linearGrad.setColorAt(1, QColor(98, 98, 98));

    // FIXME: this doesn't seem to work in 0.10.x versions, so I disable it for now
    //    linearGrad.setColorAt(1, calcPixmapColor());

    p->fillRect(drawRect, QBrush(linearGrad));

    // ----------------------------------------

    p->resetTransform();
    p->translate(drawRect.bottomLeft());
    p->rotate(-90.0);

    // ----------------------------------------

    const int    spacing = 8;
    const int    margin  = 4;
    QPixmap      appIcon;
    QString      appName;
    QFontMetrics fontMt(d->fontAppName);
    QFontMetrics fontMt2(d->fontVersion);

    if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
    {
        appIcon = SmallIcon("digikam", d->fontAppName.pixelSize());
        appName = QString("digiKam");
    }
    else
    {
        appIcon = SmallIcon("showfoto", d->fontAppName.pixelSize());
        appName = QString("showFoto");
    }

    QRect fontRect = QRect(appIcon.width() + spacing, 1, fontMt.width(appName), drawRect.width());
    int   shift    = fontMt.ascent() - fontMt2.ascent();

    // ----------------------------------------
    // draw application icon.

    p->drawPixmap(margin, 1, appIcon);

    // ----------------------------------------
    // draw app name.

    p->setFont(d->fontAppName);
    p->drawText(fontRect, Qt::AlignLeft|Qt::AlignVCenter, appName);

    // ----------------------------------------
    // draw version string.

    fontRect.moveLeft(fontRect.right() + spacing);
    fontRect.setY(shift);
    p->setFont(d->fontVersion);
    p->drawText(fontRect, Qt::AlignLeft|Qt::AlignVCenter, QString(digikam_version_short));
}

}  // namespace Digikam
