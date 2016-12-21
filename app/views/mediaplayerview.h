/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-20-12
 * Description : a view to embed Phonon media player.
 *
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QUrl>

// QtAV includes

#include <QtAV/QtAV.h>

class QEvent;

namespace Digikam
{

class MediaPlayerView : public QStackedWidget
{
    Q_OBJECT

public:

    explicit MediaPlayerView(QWidget* const parent);
    ~MediaPlayerView();

    void setCurrentItem(const QUrl& url   = QUrl(),
                        bool  hasPrevious = false,
                        bool  hasNext     = false);
    void escapePreview();
    void reload();

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    void signalEscapePreview();

public Q_SLOTS:

    void slotEscapePressed();

private Q_SLOTS:

    void slotThemeChanged();
    void slotPlayerStateChanged(QtAV::AVPlayer::State state);
    void slotMediaStatusChanged(QtAV::MediaStatus status);
    void slotHandlePlayerError(const QtAV::AVError& err);

    // Slidebar slots
    void slotPositionChanged(qint64 position);
    void slotDurationChanged(qint64 duration);
    void slotPosition(int position);
    void slotPausePlay();

private:

    int  previewMode();
    void setPreviewMode(int mode);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // MEDIAPLAYERVIEW_H
