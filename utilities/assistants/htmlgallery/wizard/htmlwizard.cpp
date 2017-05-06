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
#include "galleryinfo.h"
#include "invisiblebuttongroup.h"
#include "htmlintropage.h"
#include "htmloutputpage.h"
#include "htmlalbumselectorpage.h"
#include "htmlthemepage.h"
#include "htmlparameterspage.h"
#include "htmlimagesettingspage.h"
#include "htmlfinalpage.h"

namespace Digikam
{

class HTMLWizard::Private
{
public:

    Private()
      : mInfo(0),
        mConfigManager(0),
        mIntroPage(0),
        mCollectionSelectorPage(0),
        mThemePage(0),
        mParametersPage(0),
        mImageSettingsPage(0),
        mOutputPage(0),
        mFinalPage(0)
    {
    }

    GalleryInfo*           mInfo;
    KConfigDialogManager*  mConfigManager;

    HTMLIntroPage*         mIntroPage;
    HTMLAlbumSelectorPage* mCollectionSelectorPage;
    HTMLThemePage*         mThemePage;
    HTMLParametersPage*    mParametersPage;
    HTMLImageSettingsPage* mImageSettingsPage;
    HTMLOutputPage*        mOutputPage;
    HTMLFinalPage*         mFinalPage;
};

HTMLWizard::HTMLWizard(QWidget* const parent)
    : DWizardDlg(parent, QLatin1String("HTML Gallery Dialog")),
      d(new Private)
{
    setOption(QWizard::NoCancelButtonOnLastPage);
    setWindowTitle(i18n("Create Html Gallery"));

    d->mInfo = new GalleryInfo;
    d->mInfo->load();

    d->mIntroPage              = new HTMLIntroPage(this, i18n("Welcome to HTML Gallery Tool"));
    d->mCollectionSelectorPage = new HTMLAlbumSelectorPage(this, i18n("Albums Selection"));
    d->mThemePage              = new HTMLThemePage(this, i18n("Theme Selection"));
    d->mParametersPage         = new HTMLParametersPage(this, i18n("Theme Parameters"));
    d->mImageSettingsPage      = new HTMLImageSettingsPage(this, i18n("Image Settings"));
    d->mOutputPage             = new HTMLOutputPage(this, i18n("Output Settings"));
    d->mFinalPage              = new HTMLFinalPage(this, i18n("Generating Gallery"));
    d->mConfigManager          = new KConfigDialogManager(this, d->mInfo);
    d->mConfigManager->updateWidgets();
}

HTMLWizard::~HTMLWizard()
{
    delete d;
}

bool HTMLWizard::validateCurrentPage()
{
    if (!DWizardDlg::validateCurrentPage())
        return false;

    if (currentPage() == d->mOutputPage)
    {
        GalleryTheme::Ptr curtheme                     = galleryTheme();
        QString themeInternalName                      = curtheme->internalName();
        d->mInfo->setTheme(themeInternalName);

        GalleryTheme::ParameterList parameterList      = curtheme->parameterList();
        GalleryTheme::ParameterList::ConstIterator it  = parameterList.constBegin();
        GalleryTheme::ParameterList::ConstIterator end = parameterList.constEnd();

        for (; it != end ; ++it)
        {
            AbstractThemeParameter* const themeParameter = *it;
            QByteArray parameterInternalName             = themeParameter->internalName();
            QWidget* const widget                        = d->mParametersPage->themeParameterWidgetFromName(parameterInternalName);
            QString value                                = themeParameter->valueFromWidget(widget);

            d->mInfo->setThemeParameterValue(themeInternalName,
                                             QString::fromLatin1(parameterInternalName),
                                             value);
        }

        d->mConfigManager->updateSettings();
        d->mInfo->save();
    }

    return true;
}

int HTMLWizard::nextId() const
{
    if (currentPage() == d->mThemePage)
    {
        GalleryTheme::Ptr theme = galleryTheme();

        if (theme && theme->parameterList().size() > 0)
        {
            // Enable theme parameters page as next page if there is any parameter.
            return d->mParametersPage->id();
        }

        return d->mImageSettingsPage->id();
    }

    return DWizardDlg::nextId();
}

GalleryInfo* HTMLWizard::galleryInfo() const
{
    return d->mInfo;
}

GalleryTheme::Ptr HTMLWizard::galleryTheme() const
{
    return d->mThemePage->currentTheme();
}

} // namespace Digikam
