/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "theme.h"

namespace Digikam
{

class GalleryInfo;

class ThemeListBoxItem : public QListWidgetItem
{
public:

    ThemeListBoxItem(QListWidget* const list, Theme::Ptr theme)
        : QListWidgetItem(theme->name(), list),
          mTheme(theme)
    {
    }

    Theme::Ptr mTheme;
};

// ------------------------------------------------------------------------

class HTMLThemePage : public DWizardPage
{
public:

    explicit HTMLThemePage(QWizard* const dialog, const QString& title);
    ~HTMLThemePage();

    void initThemePage(GalleryInfo* const info);

public:

    QListWidget*  mThemeList;
    QTextBrowser* mThemeInfo;
};

} // namespace Digikam

#endif // HTML_THEME_PAGE_H
