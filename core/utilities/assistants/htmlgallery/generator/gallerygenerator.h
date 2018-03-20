/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef GALLERY_GENERATOR_H
#define GALLERY_GENERATOR_H

// Qt includes

#include <QObject>

namespace Digikam
{

class DHistoryView;
class DProgressWdg;
class GalleryInfo;
class GalleryElementFunctor;

/**
 * This class is responsible for generating the HTML and scaling the images
 * according to the settings specified by the user.
 */
class GalleryGenerator : public QObject
{
    Q_OBJECT

public:

    explicit GalleryGenerator(GalleryInfo* const);
    virtual ~GalleryGenerator();

    void setProgressWidgets(DHistoryView* const, DProgressWdg* const);

    bool run();
    bool warnings() const;

    /**
     * Produce a web-friendly file name
     */
    static QString webifyFileName(const QString&);

Q_SIGNALS:

    /**
     * This signal is emitted from GalleryElementFunctor. It uses a
     * QueuedConnection to switch between the GalleryElementFunctor thread and
     * the gui thread.
     */
    void logWarningRequested(const QString&);

private Q_SLOTS:

    void logWarning(const QString&);
    void slotCancel();

private:

    class Private;
    Private* const d;

    friend class Private;
    friend class GalleryElementFunctor;
};

} // namespace Digikam

#endif // GALLERY_GENERATOR_H
