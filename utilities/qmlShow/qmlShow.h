/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : slide show tool using preview of pictures.
 *
 * Copyright (C) 2005-2011 by Dhruv Patel <dhruvkumarr dot patel51 at gmail dot com>
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

#ifndef QML_SHOW_H
#define QML_SHOW_H

// Qt includes

#include <QMainWindow>


namespace Digikam
{
class Imageinfo;
class QmlShow : public QMainWindow
{
    Q_OBJECT

public:

    QmlShow(const QStringList&);
    ~QmlShow();

public slots:

    void nextImage();
    void prevImage();
    void pause();
    void play();
    void gridChanged(int);
private:

    class QmlShowPriv;
    QmlShowPriv* const d;
};
}
#endif
