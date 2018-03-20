/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-05
 * Description : a ListView to display black frames
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
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

#define THUMB_WIDTH 150

#include "blackframelistview.h"

// Qt includes

#include <QList>
#include <QPainter>
#include <QPixmap>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

BlackFrameListView::BlackFrameListView(QWidget* const parent)
    : QTreeWidget(parent)
{
    setColumnCount(3);
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setIconSize(QSize(THUMB_WIDTH, THUMB_WIDTH));

    QStringList labels;
    labels.append( i18n("Preview") );
    labels.append( i18n("Size") );
    labels.append( i18nc("This is a column which will contain the amount of HotPixels "
                         "found in the black frame file", "HP") );
    setHeaderLabels(labels);
}

// ----------------------------------------------------------------------------

BlackFrameListViewItem::BlackFrameListViewItem(BlackFrameListView* const parent, const QUrl& url)
    : QObject(parent),
      QTreeWidgetItem(parent)
{
    m_parent        = parent;
    m_blackFrameURL = url;
    m_parser        = new BlackFrameParser(parent);
    m_parser->parseBlackFrame(url);

    connect(m_parser, SIGNAL(signalParsed(QList<HotPixel>)),
            this, SLOT(slotParsed(QList<HotPixel>)));

    connect(this, SIGNAL(signalParsed(QList<HotPixel>,QUrl)),
            parent, SLOT(slotParsed(QList<HotPixel>,QUrl)));

    connect(m_parser, SIGNAL(signalLoadingProgress(float)),
            this, SIGNAL(signalLoadingProgress(float)));

    connect(m_parser, SIGNAL(signalLoadingComplete()),
            this, SIGNAL(signalLoadingComplete()));
}

void BlackFrameListViewItem::activate()
{
    m_parent->setToolTip( m_blackFrameDesc);
    emit signalParsed(m_hotPixels, m_blackFrameURL);
}

void BlackFrameListViewItem::slotParsed(const QList<HotPixel>& hotPixels)
{
    m_hotPixels = hotPixels;
    m_image     = m_parser->image();
    m_imageSize = m_image.size();
    m_thumb     = thumb(QSize(THUMB_WIDTH, THUMB_WIDTH/3*2)).toImage();
    setIcon(0, QPixmap::fromImage(m_thumb));

    if (!m_imageSize.isEmpty())
    {
        setText(1, QString::fromUtf8("%1x%2").arg(m_imageSize.width()).arg(m_imageSize.height()));
    }

    setText(2, QString::number(m_hotPixels.count()));

    m_blackFrameDesc = QString::fromUtf8("<p><b>%1</b>:<p>").arg(m_blackFrameURL.fileName());

    for (QList <HotPixel>::const_iterator it = m_hotPixels.constBegin() ; it != m_hotPixels.constEnd() ; ++it)
    {
        m_blackFrameDesc.append( QString::fromUtf8("[%1,%2] ").arg((*it).x()).arg((*it).y()) );
    }

    emit signalParsed(m_hotPixels, m_blackFrameURL);
}

QPixmap BlackFrameListViewItem::thumb(const QSize& size)
{
    //First scale it down to the size
    QPixmap thumb = QPixmap::fromImage(m_image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    //And draw the hot pixel positions on the thumb
    QPainter p(&thumb);

    //Take scaling into account
    float xRatio, yRatio;
    float hpThumbX, hpThumbY;
    QRect hpRect;

    xRatio = (float)size.width()/(float)m_image.width();
    yRatio = (float)size.height()/(float)m_image.height();

    //Draw hot pixels one by one
    QList<HotPixel>::const_iterator it;

    for (it = m_hotPixels.constBegin(); it != m_hotPixels.constEnd(); ++it)
    {
        hpRect   = (*it).rect;
        hpThumbX = (hpRect.x() + hpRect.width()  / 2) * xRatio;
        hpThumbY = (hpRect.y() + hpRect.height() / 2) * yRatio;

        p.setPen(QPen(Qt::black));
        p.drawLine((int) hpThumbX,      (int) hpThumbY - 1, (int) hpThumbX, (int) hpThumbY + 1);
        p.drawLine((int) hpThumbX  - 1, (int) hpThumbY, (int) hpThumbX + 1, (int) hpThumbY);
        p.setPen(QPen(Qt::white));
        p.drawPoint((int) hpThumbX - 1, (int) hpThumbY - 1);
        p.drawPoint((int) hpThumbX + 1, (int) hpThumbY + 1);
        p.drawPoint((int) hpThumbX - 1, (int) hpThumbY + 1);
        p.drawPoint((int) hpThumbX + 1, (int) hpThumbY - 1);
    }

    return thumb;
}

}  // namespace Digikam
