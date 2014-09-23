/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-22
 * Description : Slideshow video viewer
 *
 * Copyright (C) 2014 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SLIDEVIDEO_H
#define SLIDEVIDEO_H

// Qt includes

#include <QWidget>

// KDE includes

#include <kurl.h>
#include <phonon/mediaobject.h>

class QEvent;

namespace Digikam
{

class SlideVideo : public QWidget
{
    Q_OBJECT

public:

    explicit SlideVideo(QWidget* const parent);
    ~SlideVideo();

    void setCurrentUrl(const KUrl& url);
    void pause();
    void play();
    void stop();

Q_SIGNALS:

    void signalVideoLoaded(bool);
    void signalVideoFinished();

private Q_SLOTS:

    void slotPlayerFinished();
    void slotPlayerstateChanged(Phonon::State newState, Phonon::State oldState);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* SLIDEVIDEO_H */
