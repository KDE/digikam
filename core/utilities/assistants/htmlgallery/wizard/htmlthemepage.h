/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
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

#ifndef HTML_THEME_PAGE_H
#define HTML_THEME_PAGE_H

// Qt includes

#include <QListWidget>
#include <QTextBrowser>
#include <QWidget>

// Local includes

#include "dwizardpage.h"
#include "gallerytheme.h"

namespace Digikam
{

class GalleryInfo;

class ThemeListBoxItem : public QListWidgetItem
{
public:

    ThemeListBoxItem(QListWidget* const list, GalleryTheme::Ptr theme)
        : QListWidgetItem(theme->name(), list),
          m_theme(theme)
    {
    }

    GalleryTheme::Ptr m_theme;
};

// ------------------------------------------------------------------------

class HTMLThemePage : public DWizardPage
{
    Q_OBJECT

public:

    explicit HTMLThemePage(QWizard* const dialog, const QString& title);
    ~HTMLThemePage();

    void initializePage();
    bool validatePage();

    GalleryTheme::Ptr currentTheme() const;

private Q_SLOTS:

    void slotThemeSelectionChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // HTML_THEME_PAGE_H
