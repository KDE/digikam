/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : slide show tool using preview of pictures.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SLIDE_SHOW_H
#define SLIDE_SHOW_H

// Qt includes

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QStackedWidget>

// Local includes

#include "digikam_export.h"
#include "loadingdescription.h"
#include "slideshowsettings.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT SlideShow : public QStackedWidget
{
    Q_OBJECT

public:

    enum SlideShowViewMode
    {
        ErrorView=0,
        ImageView,
        VideoView,
        EndView
    };

public:

    explicit SlideShow(const SlideShowSettings& settings);
    virtual ~SlideShow();

    void setCurrentItem(const QUrl& url);
    QUrl currentItem() const;

    void toggleTag(int tag);
    void updateTags(const QUrl& url, const QStringList& tags);

Q_SIGNALS:

    void signalRatingChanged(const QUrl&, int);
    void signalColorLabelChanged(const QUrl&, int);
    void signalPickLabelChanged(const QUrl&, int);
    void signalToggleTag(const QUrl&, int);
    void signalLastItemUrl(const QUrl&);

public Q_SLOTS:

    void slotLoadNextItem();
    void slotLoadPrevItem();
    void slotPause();
    void slotPlay();

    void slotAssignRating(int);
    void slotAssignColorLabel(int);
    void slotAssignPickLabel(int);

protected:

    void mousePressEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
    void wheelEvent(QWheelEvent*);

private Q_SLOTS:

    void slotMouseMoveTimeOut();
    void slotImageLoaded(bool);
    void slotScreenSelected(int);
    void slotVideoLoaded(bool);
    void slotVideoFinished();

private:

    void setCurrentView(SlideShowViewMode);
    bool eventFilter(QObject* obj, QEvent* ev);
    void preloadNextItem();
    void endOfSlide();
    void inhibitScreenSaver();
    void allowScreenSaver();
    void dispatchCurrentInfoChange(const QUrl& url);
    void makeCornerRectangles(const QRect& desktopRect, const QSize& size,
                              QRect* const topLeft, QRect* const topRight,
                              QRect* const topLeftLarger, QRect* const topRightLarger);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SLIDE_SHOW_H
