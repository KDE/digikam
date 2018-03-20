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

#ifndef BLACKFRAMELISTVIEW_H
#define BLACKFRAMELISTVIEW_H

// Qt includes

#include <QList>
#include <QImage>
#include <QString>
#include <QSize>
#include <QPoint>
#include <QPixmap>
#include <QUrl>
#include <QTreeWidget>

// Local includes

#include "blackframeparser.h"
#include "hotpixel.h"

namespace Digikam
{

class BlackFrameListView : public QTreeWidget
{
    Q_OBJECT

public:

    explicit BlackFrameListView(QWidget* const parent=0);
    ~BlackFrameListView() {};

Q_SIGNALS:

    void signalBlackFrameSelected(const QList<HotPixel>&, const QUrl&);

private Q_SLOTS:

    void slotParsed(const QList<HotPixel>& hotPixels, const QUrl& blackFrameURL)
    {
        emit signalBlackFrameSelected(hotPixels, blackFrameURL);
    };
};

//-----------------------------------------------------------------------------------

class BlackFrameListViewItem : public QObject, QTreeWidgetItem
{
    Q_OBJECT

public:

    BlackFrameListViewItem(BlackFrameListView* const parent, const QUrl& url);
    ~BlackFrameListViewItem() {};

Q_SIGNALS:

    void signalParsed(const QList<HotPixel>&, const QUrl&);
    void signalLoadingProgress(float);
    void signalLoadingComplete();

protected:

    void activate();

private:

    QPixmap thumb(const QSize& size);

private Q_SLOTS:

    void slotParsed(const QList<HotPixel>&);

private:

    // Data contained within each listview item
    QImage              m_thumb;
    QImage              m_image;

    QSize               m_imageSize;

    QList<HotPixel>     m_hotPixels;

    QString             m_blackFrameDesc;

    QUrl                m_blackFrameURL;

    BlackFrameParser*   m_parser;

    BlackFrameListView* m_parent;
};

}  // namespace Digikam

#endif  // BLACKFRAMELISTVIEW_H
