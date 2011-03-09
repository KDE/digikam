/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : a bar widget to display image thumbnails
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef THUMBBAR_H
#define THUMBBAR_H

// Qt includes

#include <Qt3Support/Q3ScrollView>
#include <QtCore/QString>
#include <QtCore/QEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QWheelEvent>

// KDE includes

#include <kurl.h>

// Local includes

#include "thumbbartooltip.h"
#include "digikam_export.h"

class KFileItem;

namespace Digikam
{

class LoadingDescription;
class ThumbBarItem;
class ThumbBarViewPriv;
class ThumbBarItemPriv;

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbBarView : public Q3ScrollView
{
    Q_OBJECT

public:

    explicit ThumbBarView(QWidget* parent, int orientation=Qt::Vertical, bool exifRotate=true,
                          ThumbBarToolTipSettings settings=ThumbBarToolTipSettings());
    virtual ~ThumbBarView();

    void setOrientation(int orientation);
    int  getOrientation();
    void setToolTip(ThumbBarToolTip* toolTip);

    int countItems();
    KUrl::List itemsUrls();

    void triggerUpdate();

    void setSelected(ThumbBarItem* item);

    void setExifRotate(bool exifRotate);
    bool getExifRotate();

    void setToolTipSettings(const ThumbBarToolTipSettings& settings);
    ThumbBarToolTipSettings& getToolTipSettings() const;

    ThumbBarItem* currentItem() const;
    ThumbBarItem* highlightedItem() const;
    ThumbBarItem* firstItem() const;
    ThumbBarItem* lastItem()  const;
    ThumbBarItem* findItem(const QPoint& pos) const;
    ThumbBarItem* findItemByUrl(const KUrl& url) const;

    void refreshThumbs(const KUrl::List& urls);
    void invalidateThumb(ThumbBarItem* item);
    void reloadThumbs(const KUrl::List& urls);
    void reloadThumb(ThumbBarItem* item);

    virtual void ensureItemVisible(ThumbBarItem* item);
    virtual void takeItem(ThumbBarItem* item);
    virtual void clear(bool updateView=true);
    virtual void removeItem(ThumbBarItem* item);

    static QPixmap generateFuzzyRect(const QSize& size, const QColor& color, int radius);

public Q_SLOTS:

    void slotUpdate();

Q_SIGNALS:

    void signalItemSelected(ThumbBarItem*);
    void signalUrlSelected(const KUrl&);
    void signalItemAdded(void);

protected:

    int  getTileSize();
    int  getMargin();
    int  getRadius();

    bool pixmapForItem(ThumbBarItem* item, QPixmap& pix) const;
    void preloadPixmapForItem(ThumbBarItem* item) const;

    void insertItem(ThumbBarItem* item);
    void repaintItem(ThumbBarItem* item);

    virtual void rearrangeItems();
    virtual void resizeEvent(QResizeEvent*);
    virtual void contentsMousePressEvent(QMouseEvent*);
    virtual void contentsMouseMoveEvent(QMouseEvent*);
    virtual void contentsMouseReleaseEvent(QMouseEvent*);
    virtual void contentsWheelEvent(QWheelEvent*);
    virtual void leaveEvent(QEvent*);
    virtual void focusOutEvent(QFocusEvent*);

    virtual bool acceptToolTip(ThumbBarItem*, const QPoint&);
    virtual void viewportPaintEvent(QPaintEvent*);
    virtual void startDrag();

protected Q_SLOTS:

    virtual void slotPreload();
    void slotContentsMoved();
    void checkPreload();

protected:

    ThumbBarToolTip* m_toolTip;

private Q_SLOTS:

    void slotGotThumbnail(const LoadingDescription&, const QPixmap&);
    void slotToolTip();

private:

    ThumbBarViewPriv* const d;

    friend class ThumbBarItem;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbBarItem
{
public:

    ThumbBarItem(ThumbBarView* view, const KUrl& url);
    virtual ~ThumbBarItem();

    KUrl          url() const;

    ThumbBarItem* next() const;
    ThumbBarItem* prev() const;
    int           position() const;
    QRect         rect() const;

    void          setTooltipRect(const QRect& rect);
    QRect         tooltipRect() const;

    void          repaint();

private:

    ThumbBarItemPriv* const d;

    friend class ThumbBarView;
};

}  // namespace Digikam

#endif /* THUMBBAR_H */
