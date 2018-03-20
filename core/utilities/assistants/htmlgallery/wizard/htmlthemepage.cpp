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

#include "htmlthemepage.h"

// Qt includes

#include <QIcon>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "galleryinfo.h"
#include "htmlwizard.h"
#include "dlayoutbox.h"

namespace Digikam
{

class HTMLThemePage::Private
{
public:

    explicit Private()
      : themeList(0),
        themeInfo(0)
    {
    }

    QListWidget*  themeList;
    QTextBrowser* themeInfo;
};

HTMLThemePage::HTMLThemePage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
{
    setObjectName(QLatin1String("ThemePage"));

    DHBox* const hbox = new DHBox(this);

    d->themeList      = new QListWidget(hbox);
    d->themeList->setObjectName(QLatin1String("d->themeList"));

    d->themeInfo      = new QTextBrowser(hbox);
    d->themeInfo->setObjectName(QLatin1String("d->themeInfo"));

    hbox->setContentsMargins(QMargins());
    hbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    connect(d->themeList, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotThemeSelectionChanged()));

    setPageWidget(hbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("application-x-theme")));
}

HTMLThemePage::~HTMLThemePage()
{
    delete d;
}

void HTMLThemePage::initializePage()
{
    HTMLWizard* const wizard              = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
        return;

    GalleryInfo* const info               = wizard->galleryInfo();
    GalleryTheme::List list               = GalleryTheme::getList();
    GalleryTheme::List::ConstIterator it  = list.constBegin();
    GalleryTheme::List::ConstIterator end = list.constEnd();

    d->themeList->clear();

    for (; it != end ; ++it)
    {
        GalleryTheme::Ptr theme      = *it;
        ThemeListBoxItem* const item = new ThemeListBoxItem(d->themeList, theme);

        if (theme->internalName() == info->theme())
        {
            d->themeList->setCurrentItem(item);
        }
    }

    // Set page states, whoch can only be disabled after they have *all* been added.
    slotThemeSelectionChanged();
}

bool HTMLThemePage::validatePage()
{
    if (!d->themeList->currentItem())
    {
        return false;
    }

    return true;
}

void HTMLThemePage::slotThemeSelectionChanged()
{
    if (d->themeList->currentItem())
    {
        GalleryTheme::Ptr curTheme = currentTheme();
        QString url                = curTheme->authorUrl();
        QString author             = curTheme->authorName();

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

        QString advSet = (curTheme->parameterList().size() > 0) ? i18n("can be customized")
                                                                : i18n("no customization available");
        QString txt    = image + QString::fromUtf8("<b>%3</b><br/><br/>%4<br/><br/>")
                                   .arg(curTheme->name(), curTheme->comment())
                               + i18n("Author: %1<br/><br/>", author)
                               + QString::fromUtf8("<i>%1</i>").arg(advSet);

        d->themeInfo->setHtml(txt);
    }
    else
    {
        d->themeInfo->clear();
    }
}

GalleryTheme::Ptr HTMLThemePage::currentTheme() const
{
    ThemeListBoxItem* const item = dynamic_cast<ThemeListBoxItem*>(d->themeList->currentItem());

    if (item)
    {
        return item->m_theme;
    }

    return GalleryTheme::Ptr(0);
}

} // namespace Digikam
