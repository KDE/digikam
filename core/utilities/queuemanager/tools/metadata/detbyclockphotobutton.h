/* ============================================================
 *
 * This file is a part of the digikam project
 * http://www.digikam.org
 *
 * Date        : 2017-01-01
 * Description : button for choosing time difference photo which accepts drag & drop
 *
 * Copyright (C) 2017 by Markus Leuthold <kusi at forum dot titlis dot org>
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

#ifndef DET_BY_CLOCK_PHOTO_BUTTON_H
#define DET_BY_CLOCK_PHOTO_BUTTON_H

// Qt includes

#include <QWidget>
#include <QDragEnterEvent>
#include <QPushButton>

class DetByClockPhotoButton : public QPushButton
{
    Q_OBJECT

public:

    DetByClockPhotoButton(const QString& text)
        : QPushButton(text)
    {
        setAcceptDrops(true);
    };

    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

Q_SIGNALS:

    void signalClockPhotoDropped(const QUrl&);
};

#endif // DET_BY_CLOCK_PHOTO_BUTTON_H
