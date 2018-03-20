/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-31
 * Description : a widget to display free space for a mount-point.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QString>
#include <QWidget>

namespace Digikam
{

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

    FreeSpaceWidget(QWidget* const parent, int width);
    ~FreeSpaceWidget();

    void          setEstimatedDSizeKb(unsigned long dSize);
    unsigned long estimatedDSizeKb() const;

    bool          isValid()                    const;
    int           percentUsed()                const;
    unsigned long kBSize()                     const;
    unsigned long kBUsed()                     const;
    unsigned long kBAvail()                    const;
    unsigned long kBAvail(const QString& path) const;

    void          refresh();

    void setMode(FreeSpaceMode mode);
    void setPath(const QString& path);
    void setPaths(const QStringList& paths);
    void addInformation(unsigned long kBSize,
                        unsigned long kBUsed, unsigned long kBAvail,
                        const QString& mountPoint);

protected:

    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void paintEvent(QPaintEvent*);
    void updateToolTip();

private Q_SLOTS:

    void slotTimeout();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* FREESPACEWIDGET_H */
