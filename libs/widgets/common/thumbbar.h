/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : a bar widget to display image thumbnails
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <Q3ScrollView>
#include <QString>
#include <QEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"

class KFileItem;

namespace Digikam
{

class LoadingDescription;
class ThumbBarItem;
class ThumbBarToolTip;
class ThumbBarViewPriv;
class ThumbBarItemPriv;

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

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbBarView : public Q3ScrollView
{
    Q_OBJECT

public:

    ThumbBarView(QWidget* parent, int orientation=Qt::Vertical, bool exifRotate=true,
                 ThumbBarToolTipSettings settings=ThumbBarToolTipSettings());
    virtual ~ThumbBarView();

    int countItems();
    KUrl::List itemsURLs();

    void clear(bool updateView=true);
    void triggerUpdate();

    void removeItem(ThumbBarItem* item);

    void setSelected(ThumbBarItem* item);
    void ensureItemVisible(ThumbBarItem* item);

    void setExifRotate(bool exifRotate);
    bool getExifRotate();

    void setToolTipSettings(const ThumbBarToolTipSettings &settings);
    ThumbBarToolTipSettings& getToolTipSettings();

    ThumbBarItem* currentItem() const;
    ThumbBarItem* firstItem() const;
    ThumbBarItem* lastItem()  const;
    ThumbBarItem* findItem(const QPoint& pos) const;
    ThumbBarItem* findItemByURL(const KUrl& url) const;

    void refreshThumbs(const KUrl::List& urls);
    void invalidateThumb(ThumbBarItem* item);

signals:

    void signalItemSelected(ThumbBarItem*);
    void signalURLSelected(const KUrl&);
    void signalItemAdded(void);

protected:

    int  getOrientation();
    int  getTileSize();
    int  getMargin();

    void resizeEvent(QResizeEvent*);
    void contentsMousePressEvent(QMouseEvent*);
    void contentsMouseMoveEvent(QMouseEvent*);
    void contentsMouseReleaseEvent(QMouseEvent*);

    void insertItem(ThumbBarItem* item);
    void rearrangeItems();
    void repaintItem(ThumbBarItem* item);
    bool event(QEvent*);

    virtual ThumbBarToolTip* toolTip() const;
    virtual void viewportPaintEvent(QPaintEvent*);
    virtual void startDrag();

protected slots:

    void slotUpdate();

private slots:

    void slotGotThumbnail(const LoadingDescription&, const QPixmap&);

private:

    ThumbBarViewPriv* d;

    friend class ThumbBarItem;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbBarItem
{
public:

    ThumbBarItem(ThumbBarView *view, const KUrl& url);
    virtual ~ThumbBarItem();

    KUrl          url() const;

    ThumbBarItem* next() const;
    ThumbBarItem* prev() const;
    int           position() const;
    QRect         rect() const;
    bool          hasPixmap() const;
    QPixmap       pixmap() const;

    void          setPixmap(const QPixmap& pixmap);

    void          repaint();

private:

    ThumbBarItemPriv* d;

    friend class ThumbBarView;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT ThumbBarToolTip
{

public:

    ThumbBarToolTip(ThumbBarView *parent);
    virtual ~ThumbBarToolTip();

    /** Return the thummbar item area if a item is found at 'pos' else an empty QRect.
        tipText will contains the tool tip string to show.*/
    QRect maybeTip(const QPoint& pos, QString &tipText);

protected:

    const int     m_maxStringLen;

    QString       m_headBeg;
    QString       m_headEnd;
    QString       m_cellBeg;
    QString       m_cellMid;
    QString       m_cellEnd;
    QString       m_cellSpecBeg;
    QString       m_cellSpecMid;
    QString       m_cellSpecEnd;

    ThumbBarView *m_view;

protected:

    QString breakString(const QString& input) const;

    virtual QString tipContents(ThumbBarItem*) const;
};

}  // NameSpace Digikam

#endif /* THUMBBAR_H */
