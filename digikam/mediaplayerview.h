/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-20-12
 * Description : a view to embed Phonon media player.
 *
 * Copyright (C) 2006-2010 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MEDIAPLAYERVIEW_H
#define MEDIAPLAYERVIEW_H

// Qt includes

#include <QStackedWidget>

// KDE includes

#include <kurl.h>
#include <phonon/mediaobject.h>

// Local includes

#include "digikam_export.h"
#include "imageinfo.h"

class QEvent;

namespace Digikam
{

class StackedView;

class MediaPlayerMouseClickFilter : public QObject
{
    Q_OBJECT

public:

    MediaPlayerMouseClickFilter(QObject* parent);

protected:

    bool eventFilter(QObject* obj, QEvent* event);

private:

    QObject* m_parent;
};

// --------------------------------------------------------

class MediaPlayerViewPriv;
class MediaPlayerView : public QStackedWidget
{
    Q_OBJECT

public:

    MediaPlayerView(StackedView* parent);
    ~MediaPlayerView();

    void setImageInfo(const ImageInfo& info = ImageInfo(),
                      const ImageInfo& previous = ImageInfo(),
                      const ImageInfo& next = ImageInfo());
    void escapePreview();

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    void signalBack2Album();

public Q_SLOTS:

    void slotEscapePressed();

private Q_SLOTS:

    void slotPlayerFinished();
    void slotThemeChanged();
    void slotPlayerstateChanged(Phonon::State newState, Phonon::State oldState);

private:

    int  previewMode();
    void setPreviewMode(int mode);

private:

    MediaPlayerViewPriv* const d;
};

}  // namespace Digikam

#endif /* MEDIAPLAYERVIEW_H */
