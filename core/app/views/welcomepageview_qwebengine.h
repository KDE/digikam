/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef WELCOME_PAGE_VIEW_QWEBENGINE_H
#define WELCOME_PAGE_VIEW_QWEBENGINE_H

// Qt includes

#include <QString>
#include <QByteArray>
#include <QUrl>
#include <QWebEngineView>
#include <QWebEnginePage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class WelcomePageViewPage : public QWebEnginePage
{
    Q_OBJECT

public:

    explicit WelcomePageViewPage(QObject* const parent = 0);
    virtual ~WelcomePageViewPage();

    bool acceptNavigationRequest(const QUrl&, QWebEnginePage::NavigationType, bool);

Q_SIGNALS:

    void linkClicked(const QUrl&);

};

// -------------------------------------------------------------------

class WelcomePageView : public QWebEngineView
{
    Q_OBJECT

public:

    explicit WelcomePageView(QWidget* const parent);
    ~WelcomePageView();

private:

    QByteArray  fileToString(const QString& aFileName) const;
    QStringList featuresTabContent() const;
    QStringList aboutTabContent() const;

private Q_SLOTS:

    void slotUrlOpen(const QUrl&);
    void slotThemeChanged();
};

} // namespace Digikam

#endif // WELCOME_PAGE_VIEW_QWEBENGINE_H
