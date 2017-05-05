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

#include "htmlthemepage.h"

// Qt includes

#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "galleryinfo.h"
#include "htmlwizard.h"
#include "dwidgetutils.h"

namespace Digikam
{

HTMLThemePage::HTMLThemePage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title)
{
    setObjectName(QLatin1String("ThemePage"));

    DHBox* const hbox = new DHBox(this);

    mThemeList        = new QListWidget(hbox);
    mThemeList->setObjectName(QLatin1String("mThemeList"));

    mThemeInfo = new QTextBrowser(hbox);
    mThemeInfo->setObjectName(QLatin1String("mThemeInfo"));

    hbox->setContentsMargins(QMargins());
    hbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    setPageWidget(hbox);
}

HTMLThemePage::~HTMLThemePage()
{
}

void HTMLThemePage::initializePage()
{
    HTMLWizard* const wizard              = dynamic_cast<HTMLWizard*>(assistant());
    GalleryInfo* const info               = wizard->galleryInfo();
    GalleryTheme::List list               = GalleryTheme::getList();
    GalleryTheme::List::ConstIterator it  = list.constBegin();
    GalleryTheme::List::ConstIterator end = list.constEnd();

    for (; it != end ; ++it)
    {
        GalleryTheme::Ptr theme      = *it;
        ThemeListBoxItem* const item = new ThemeListBoxItem(mThemeList, theme);

        if (theme->internalName() == info->theme())
        {
            mThemeList->setCurrentItem(item);
        }
    }
}

bool HTMLThemePage::validatePage()
{
    if (!mThemeList->currentItem())
    {
        return false;
    }

    return true;
}

int HTMLThemePage::nextId() const
{
    if (mThemeList->currentItem())
    {
        GalleryTheme::Ptr theme = static_cast<ThemeListBoxItem*>(mThemeList->currentItem())->mTheme;

        // Enable theme parameter page as next page if there is any parameter

        if (theme->parameterList().size() > 0)
        {
            return dynamic_cast<HTMLWizard*>(assistant())->parametersPageId();
        }
    }

    return dynamic_cast<HTMLWizard*>(assistant())->imageSettingsPageId();
}

} // namespace Digikam
