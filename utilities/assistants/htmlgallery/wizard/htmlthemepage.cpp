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

    connect(mThemeList, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotThemeSelectionChanged()));

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

    // Set page states, whoch can only be disabled after they have *all* been added.
    slotThemeSelectionChanged();
}

bool HTMLThemePage::validatePage()
{
    if (!mThemeList->currentItem())
    {
        return false;
    }

    return true;
}

void HTMLThemePage::slotThemeSelectionChanged()
{
    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());
    GalleryTheme::Ptr theme  = wizard->galleryTheme();

    if (mThemeList->currentItem())
    {
        GalleryTheme::Ptr curTheme    = static_cast<ThemeListBoxItem*>(mThemeList->currentItem())->mTheme;
        QString url                   = curTheme->authorUrl();
        QString author                = curTheme->authorName();

        if (!url.isEmpty())
        {
            author = QString::fromUtf8("<a href='%1'>%2</a>").arg(url).arg(author);
        }

        QString preview               = curTheme->previewUrl();
        QString image                 = QLatin1String("");

        if (!preview.isEmpty())
        {
            image = QString::fromUtf8("<img src='%1/%2' /><br/><br/>")
                        .arg(curTheme->directory(), curTheme->previewUrl());
        }

        QString advSet = (theme->parameterList().size() > 0) ? i18n("can be customized")
                                                             : i18n("no customization available");
        QString txt    = image + QString::fromUtf8("<b>%3</b><br/><br/>%4<br/><br/>")
                                   .arg(curTheme->name(), curTheme->comment())
                               + i18n("Author: %1<br/><br/>", author)
                               + QString::fromUtf8("<i>%1</i>").arg(advSet);

        mThemeInfo->setHtml(txt);
        theme       = curTheme;
    }
    else
    {
        mThemeInfo->clear();
    }
}

} // namespace Digikam
