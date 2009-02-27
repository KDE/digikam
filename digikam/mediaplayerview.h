/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-20-12
 * Description : a view to embed Phonon media player.
 *
 * Copyright (C) 2006-2008 Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QStackedWidget>

// KDE includes.

#include <kurl.h>
#include <phonon/mediaobject.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class MediaPlayerViewPriv;

class MediaPlayerView : public QStackedWidget
{
Q_OBJECT

public:

    MediaPlayerView(QWidget *parent=0);
    ~MediaPlayerView();

    void setMediaPlayerFromUrl(const KUrl& url);
    void escapePreview();

private Q_SLOTS:

    void slotThemeChanged();
    void slotPlayerFinished();
    void slotPlayerstateChanged(Phonon::State newState, Phonon::State oldState);

private:

    int  previewMode();
    void setPreviewMode(int mode);

private:

    MediaPlayerViewPriv* const d;
};

}  // namespace Digikam

#endif /* MEDIAPLAYERVIEW_H */
