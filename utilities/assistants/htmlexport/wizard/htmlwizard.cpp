/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
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

#include "htmlwizard.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include <QComboBox>
#include <QListWidget>
#include <QTextBrowser>

// KDE includes

#include <kconfigdialogmanager.h>
#include <klocalizedstring.h>

// Local includes

#include "dwizardpage.h"
#include "digikam_debug.h"
#include "abstractthemeparameter.h"
#include "albumselecttabs.h"
#include "galleryinfo.h"
#include "invisiblebuttongroup.h"
#include "theme.h"
#include "htmloutputpage.h"
#include "htmlthemepage.h"
#include "htmlparameterspage.h"
#include "htmlimagesettingspage.h"

namespace Digikam
{

class HTMLWizard::Private
{
public:

    GalleryInfo*                    mInfo;
    KConfigDialogManager*           mConfigManager;

    AlbumSelectTabs*                mCollectionSelector;
    DWizardPage*                    mCollectionSelectorPage;
    HTMLParametersPage*             mParametersPage;
    HTMLImageSettingsPage*          mImageSettingsPage;
    HTMLOutputPage*                 mOutputPage;
    HTMLThemePage*                  mThemePage;
};

HTMLWizard::HTMLWizard(QWidget* const parent, GalleryInfo* const info)
    : QWizard(parent),
      d(new Private)
{
    d->mInfo                   = info;

    setWindowTitle(i18n("Export Albums to HTML Pages"));

    // ---------------------------------------------------------------

    d->mCollectionSelector     = new AlbumSelectTabs(this);
    d->mCollectionSelectorPage = new DWizardPage(this, i18n("Album Selection"));
    d->mCollectionSelectorPage->setPageWidget(d->mCollectionSelector);
    updateCollectionSelectorPageValidity();

    connect(d->mCollectionSelector, SIGNAL(signalAlbumSelectionChanged()),
            this, SLOT(updateCollectionSelectorPageValidity()));

    // ---------------------------------------------------------------

    d->mThemePage              = new HTMLThemePage(this, i18n("Theme Selection"));
    d->mThemePage->initThemePage(d->mInfo);

    connect(d->mThemePage->mThemeList, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotThemeSelectionChanged()));

    // ---------------------------------------------------------------

    d->mParametersPage         = new HTMLParametersPage(this, i18n("Theme Parameters"));
    d->mImageSettingsPage      = new HTMLImageSettingsPage(this, i18n("Image Settings"));

    // ---------------------------------------------------------------

    d->mOutputPage             = new HTMLOutputPage(this, i18n("Output Settings"));
    d->mOutputPage->kcfg_destUrl->setFileDlgMode(QFileDialog::Directory);

    connect(d->mOutputPage->kcfg_destUrl, SIGNAL(textChanged(QString)),
            this, SLOT(updateFinishPageValidity()));

    // ---------------------------------------------------------------

    d->mConfigManager          = new KConfigDialogManager(this, d->mInfo);
    d->mConfigManager->updateWidgets();

    // Set page states
    // Pages can only be disabled after they have *all* been added!
    slotThemeSelectionChanged();
    updateFinishPageValidity();
}

HTMLWizard::~HTMLWizard()
{
    delete d;
}

void HTMLWizard::updateFinishPageValidity()
{
    setValid(d->mOutputPage->page(), !d->mOutputPage->kcfg_destUrl->fileDlgPath().isEmpty());
}

void HTMLWizard::updateCollectionSelectorPageValidity()
{
    setValid(d->mCollectionSelectorPage, !d->mCollectionSelector->selectedAlbums().empty());
}

void HTMLWizard::slotThemeSelectionChanged()
{
    QListWidget* const listWidget = d->mThemePage->mThemeList;
    QTextBrowser* const browser   = d->mThemePage->mThemeInfo;

    if (listWidget->currentItem())
    {
        Theme::Ptr theme=static_cast<ThemeListBoxItem*>(listWidget->currentItem())->mTheme;

        QString url                   = theme->authorUrl();
        QString author                = theme->authorName();
        bool allowNonsquareThumbnails = theme->allowNonsquareThumbnails();

        if (!url.isEmpty())
        {
            author = QString::fromUtf8("<a href='%1'>%2</a>").arg(url).arg(author);
        }

        QString preview               = theme->previewUrl();
        QString image                 = QLatin1String("");

        if (!preview.isEmpty())
        {
            image = QString::fromUtf8("<img src='%1/%2' /><br/><br/>").arg(theme->directory(),
                                                                           theme->previewUrl());
        }

        QString txt = image +
                      QString::fromUtf8("<b>%3</b><br/><br/>%4<br/><br/>")
                          .arg(theme->name(), theme->comment()) + 
                      i18n("Author: %1", author);

        browser->setHtml(txt);
        setValid(d->mThemePage->page(), true);

        // Enable theme parameter page if there is any parameter
        Theme::ParameterList parameterList = theme->parameterList();
        setAppropriate(d->mThemeParametersPage->page(), parameterList.size() > 0);

        d->mImageSettingsPage->kcfg_thumbnailSquare->setEnabled(allowNonsquareThumbnails);

        if (!allowNonsquareThumbnails)
        {
            d->mImageSettingsPage->kcfg_thumbnailSquare->setChecked(true);
        }

        d->fillThemeParametersPage(theme);
    }
    else
    {
        browser->clear();
        setValid(d->mThemePage->page(), false);
    }
}

/**
 * Update mInfo
 */
void HTMLWizard::accept()
{
    d->mInfo->mCollectionList               = d->mCollectionSelector->selectedAlbums();
    Theme::Ptr theme                        = static_cast<ThemeListBoxItem*>(d->mThemePage->mThemeList->currentItem())->mTheme;
    QString themeInternalName               = theme->internalName();
    d->mInfo->setTheme(themeInternalName);

    Theme::ParameterList parameterList      = theme->parameterList();
    Theme::ParameterList::ConstIterator it  = parameterList.constBegin();
    Theme::ParameterList::ConstIterator end = parameterList.constEnd();

    for (; it != end ; ++it)
    {
        AbstractThemeParameter* const themeParameter = *it;
        QByteArray parameterInternalName             = themeParameter->internalName();
        QWidget* const widget                        = d->mParametersPage->mThemeParameterWidgetFromName[parameterInternalName];
        QString value                                = themeParameter->valueFromWidget(widget);

        d->mInfo->setThemeParameterValue(themeInternalName,
                                         QString::fromLatin1(parameterInternalName),
                                         value);
    }

    d->mConfigManager->updateSettings();

    QWizard::accept();
}

DHistoryView* HTMLWizard::progressView() const
{
    return d->mOutputPage->progressView;
}

DProgressWdg* HTMLWizard::progressBar() const
{
    return d->mOutputPage->progressBar;
}

} // namespace Digikam
