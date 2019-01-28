/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * Copyright (C) 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
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

#ifndef DIGIKAM_JALBUM_GENERATOR_H
#define DIGIKAM_JALBUM_GENERATOR_H

// Qt includes

#include <QObject>

// Local includes

#include "dprogresswdg.h"
#include "dhistoryview.h"

using namespace Digikam;

namespace DigikamGenericJAlbumPlugin
{

class JAlbumSettings;
class JAlbumElementFunctor;

/**
 * This class is responsible for generating the HTML and scaling the images
 * according to the settings specified by the user.
 */
class JAlbumGenerator : public QObject
{
    Q_OBJECT

public:

    explicit JAlbumGenerator(JAlbumSettings* const);
    virtual ~JAlbumGenerator();

    void setProgressWidgets(DHistoryView* const, DProgressWdg* const);

    bool run();
    bool warnings() const;

    /**
     * Produce a web-friendly file name
     */
    static QString webifyFileName(const QString&);

Q_SIGNALS:

    /**
     * This signal is emitted from JAlbumElementFunctor. It uses a
     * QueuedConnection to switch between the JAlbumElementFunctor thread and
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
    friend class JAlbumElementFunctor;
};

} // namespace DigikamGenericJAlbumPlugin

#endif // DIGIKAM_JALBUM_GENERATOR_H
