/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : a command line tool to load video thumbnail
 *
 * Copyright (C) 2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes

#include <QVideoFrame>
#include <QObject>
#include <QString>

class QMediaPlayer;
class QVideoProbe;

class VideoThumbnailer : public QObject
{
    Q_OBJECT

public:

    VideoThumbnailer(QObject* const parent=0);
    ~VideoThumbnailer();

    bool getThumbnail(const QString& file);

Q_SIGNALS:

    void signalVideoThumbDone();

private Q_SLOTS:

    void slotProcessframe(QVideoFrame);

private:

    QMediaPlayer* m_player;
    QVideoProbe*  m_probe;
    QString       m_videoFile;
};
