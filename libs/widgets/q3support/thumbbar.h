/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : a bar widget to display image thumbnails
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <kglobalsettings.h>

// Local includes

#include "ditemtooltip.h"
#include "digikam_export.h"

class KFileItem;

namespace Digikam
{

class LoadingDescription;
class ThumbBarItem;
class ThumbBarView;

// -------------------------------------------------------------------------

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
        font           = KGlobalSettings::generalFont();
    };

    bool  showToolTips;
    bool  showFileName;
    bool  showFileDate;
    bool  showFileSize;
    bool  showImageType;
    bool  showImageDim;
    bool  showPhotoMake;
    bool  showPhotoDate;
    bool  showPhotoFocal;
    bool  showPhotoExpo;
    bool  showPhotoMode;
    bool  showPhotoFlash;
    bool  showPhotoWB;

    QFont font;
};

// --------------------------------------------------------

class DIGIKAM_EXPORT ThumbBarToolTip : public DItemToolTip
{
public:

    ThumbBarToolTip(ThumbBarView* const view);
    virtual ~ThumbBarToolTip();

    void setItem(ThumbBarItem* const item);
    ThumbBarItem* item() const;

protected:

    ThumbBarToolTipSettings& toolTipSettings() const;

    virtual QRect   repositionRect();
    virtual QString tipContents();

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbBarView : public Q3ScrollView
{
    Q_OBJECT

public:

    explicit ThumbBarView(QWidget* const parent, int orientation=Qt::Vertical,
                          const ThumbBarToolTipSettings& settings=ThumbBarToolTipSettings());
    virtual ~ThumbBarView();

    void setOrientation(int orientation);
    int  getOrientation() const;

    void setToolTip(ThumbBarToolTip* const toolTip);

    int countItems()       const;
    KUrl::List itemsUrls() const;

    void triggerUpdate();

    void setSelected(ThumbBarItem* const item);

    void setToolTipSettings(const ThumbBarToolTipSettings& settings);
    ThumbBarToolTipSettings& getToolTipSettings() const;

    ThumbBarItem* currentItem()                  const;
    ThumbBarItem* highlightedItem()              const;
    ThumbBarItem* firstItem()                    const;
    ThumbBarItem* lastItem()                     const;
    ThumbBarItem* findItem(const QPoint& pos)    const;
    ThumbBarItem* findItemByUrl(const KUrl& url) const;

    void refreshThumbs(const KUrl::List& urls);
    void invalidateThumb(ThumbBarItem* item);
    void reloadThumbs(const KUrl::List& urls);
    void reloadThumb(ThumbBarItem* item);

    virtual void ensureItemVisible(ThumbBarItem* item);
    virtual void takeItem(ThumbBarItem* item);
    virtual void clear(bool updateView=true);
    virtual void removeItem(ThumbBarItem* item);

public Q_SLOTS:
    
    void slotUpdate();
    void slotDockLocationChanged(Qt::DockWidgetArea area);

Q_SIGNALS:

    void signalItemSelected(ThumbBarItem*);
    void signalUrlSelected(const KUrl&);
    void signalItemAdded(void);

protected:

    int  getTileSize() const;
    int  getMargin()   const;
    int  getRadius()   const;

    bool pixmapForItem(ThumbBarItem* const item, QPixmap& pix) const;
    void preloadPixmapForItem(ThumbBarItem* const item) const;

    void insertItem(ThumbBarItem* const item);
    void repaintItem(ThumbBarItem* const item);

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

    void slotContentsMoved();
    void checkPreload();

    virtual void slotPreload();

protected:

    ThumbBarToolTip* m_toolTip;

private Q_SLOTS:

    void slotGotThumbnail(const LoadingDescription&, const QPixmap&);
    void slotToolTip();

private:

    class Private;
    Private* const d;

    friend class ThumbBarItem;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbBarItem
{
public:

    ThumbBarItem(ThumbBarView* const view, const KUrl& url);
    virtual ~ThumbBarItem();

    KUrl          url()      const;
    ThumbBarItem* next()     const;
    ThumbBarItem* prev()     const;
    int           position() const;
    QRect         rect()     const;

    void          setTooltipRect(const QRect& rect);
    QRect         tooltipRect() const;

    void          repaint();

private:

    class Private;
    Private* const d;

    friend class ThumbBarView;
};

}  // namespace Digikam

#endif /* THUMBBAR_H */
