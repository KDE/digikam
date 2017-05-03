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

    setPageWidget(hbox);
}

HTMLThemePage::~HTMLThemePage()
{
}

void HTMLThemePage::initThemePage(GalleryInfo* const info)
{
    Theme::List list               = Theme::getList();
    Theme::List::ConstIterator it  = list.constBegin();
    Theme::List::ConstIterator end = list.constEnd();

    for (; it != end ; ++it)
    {
        Theme::Ptr theme             = *it;
        ThemeListBoxItem* const item = new ThemeListBoxItem(mThemeList, theme);

        if (theme->internalName() == info->theme())
        {
            mThemeList->setCurrentItem(item);
        }
    }
}

bool HTMLThemePage::validatePage()
{
    if (mThemeList->currentItem())
    {
        return true;
    }

    return false;
}

int HTMLThemePage::nextId() const
{
    if (mThemeList->currentItem())
    {
        Theme::Ptr theme = static_cast<ThemeListBoxItem*>(mThemeList->currentItem())->mTheme;

        // Enable theme parameter page as next page if there is any parameter

        if (theme->parameterList().size() > 0)
        {
            return dynamic_cast<HTMLWizard*>(assistant())->parametersPageId();
        }
    }

    return dynamic_cast<HTMLWizard*>(assistant())->imageSettingPageId();
}

} // namespace Digikam
