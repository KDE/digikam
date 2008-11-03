/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-31
 * Description : a widget to display free space for a mount-point.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FREESPACEWIDGET_H
#define FREESPACEWIDGET_H

// Qt includes.

#include <QString>
#include <QWidget>

// KDE includes.

#include <kaction.h>
#include <kdeversion.h>

#if KDE_IS_VERSION(4,1,68)
#include <kdiskfreespaceinfo.h>
#else
#include <kmountpoint.h>
#include <kdiskfreespace.h>
#endif

namespace Digikam
{

class FreeSpaceWidgetPriv;

class FreeSpaceWidget : public QWidget
{
    Q_OBJECT

public:

    enum FreeSpaceMode
    {
        AlbumLibrary = 0,
        UMSCamera,
        GPhotoCamera
    };

public:

    FreeSpaceWidget(QWidget* parent, int width);
    ~FreeSpaceWidget();

    void          setEstimatedDSizeKb(unsigned long dSize);
    unsigned long estimatedDSizeKb();

    bool          isValid();
    int           percentUsed();
    unsigned long kBSize();
    unsigned long kBUsed();
    unsigned long kBAvail();
    QString       mountPoint();
    void          refresh();

    void setMode(FreeSpaceMode mode);
    void setPath(const QString& path);
    void setInformations(unsigned long kBSize,
                         unsigned long kBUsed, unsigned long kBAvail,
                         const QString& mountPoint);

protected:

    void paintEvent(QPaintEvent*);
    void updatePixmap();

private slots:

    void slotTimeout();

    void slotAvailableFreeSpace(const QString& mountPoint, quint64 kBSize, quint64 kBUsed, quint64 kBAvail);

private:

    FreeSpaceWidgetPriv* d;
};

}  // namespace Digikam

#endif /* FREESPACEWIDGET_H */
