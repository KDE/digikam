/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
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

#include "dinfointerface.h"
#include "dwizardpage.h"
#include "digikam_debug.h"
#include "abstractthemeparameter.h"
#include "galleryinfo.h"
#include "invisiblebuttongroup.h"
#include "htmlintropage.h"
#include "htmloutputpage.h"
#include "htmlselectionpage.h"
#include "htmlthemepage.h"
#include "htmlparameterspage.h"
#include "htmlimagesettingspage.h"
#include "htmlfinalpage.h"

namespace Digikam
{

class HTMLWizard::Private
{
public:

    explicit Private()
      : info(0),
        configManager(0),
        introPage(0),
        selectionPage(0),
        themePage(0),
        parametersPage(0),
        imageSettingsPage(0),
        outputPage(0),
        finalPage(0)
    {
    }

    GalleryInfo*           info;
    KConfigDialogManager*  configManager;

    HTMLIntroPage*         introPage;
    HTMLSelectionPage*     selectionPage;
    HTMLThemePage*         themePage;
    HTMLParametersPage*    parametersPage;
    HTMLImageSettingsPage* imageSettingsPage;
    HTMLOutputPage*        outputPage;
    HTMLFinalPage*         finalPage;
};

HTMLWizard::HTMLWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("HTML Gallery Dialog")),
      d(new Private)
{
    setOption(QWizard::NoCancelButtonOnLastPage);
    setWindowTitle(i18n("Create HTML Gallery"));

    d->info = new GalleryInfo(iface);
    d->info->load();

    d->introPage         = new HTMLIntroPage(this,         i18n("Welcome to HTML Gallery Tool"));
    d->selectionPage     = new HTMLSelectionPage(this,     i18n("Items Selection"));
    d->themePage         = new HTMLThemePage(this,         i18n("Theme Selection"));
    d->parametersPage    = new HTMLParametersPage(this,    i18n("Theme Parameters"));
    d->imageSettingsPage = new HTMLImageSettingsPage(this, i18n("Image Settings"));
    d->outputPage        = new HTMLOutputPage(this,        i18n("Output Settings"));
    d->finalPage         = new HTMLFinalPage(this,         i18n("Generating Gallery"));
    d->configManager     = new KConfigDialogManager(this, d->info);
    d->configManager->updateWidgets();
}

HTMLWizard::~HTMLWizard()
{
    delete d;
}

void HTMLWizard::setItemsList(const QList<QUrl>& urls)
{
    d->selectionPage->setItemsList(urls);
}

bool HTMLWizard::validateCurrentPage()
{
    if (!DWizardDlg::validateCurrentPage())
        return false;

    if (currentPage() == d->outputPage)
    {
        GalleryTheme::Ptr curtheme                     = galleryTheme();
        QString themeInternalName                      = curtheme->internalName();
        d->info->setTheme(themeInternalName);

        GalleryTheme::ParameterList parameterList      = curtheme->parameterList();
        GalleryTheme::ParameterList::ConstIterator it  = parameterList.constBegin();
        GalleryTheme::ParameterList::ConstIterator end = parameterList.constEnd();

        for (; it != end ; ++it)
        {
            AbstractThemeParameter* const themeParameter = *it;
            QByteArray parameterInternalName             = themeParameter->internalName();
            QWidget* const widget                        = d->parametersPage->themeParameterWidgetFromName(parameterInternalName);
            QString value                                = themeParameter->valueFromWidget(widget);

            d->info->setThemeParameterValue(themeInternalName,
                                             QString::fromLatin1(parameterInternalName),
                                             value);
        }

        d->configManager->updateSettings();
        d->info->save();
    }

    return true;
}

int HTMLWizard::nextId() const
{
    if (currentPage() == d->themePage)
    {
        GalleryTheme::Ptr theme = galleryTheme();

        if (theme && theme->parameterList().size() > 0)
        {
            // Enable theme parameters page as next page if there is any parameter.
            return d->parametersPage->id();
        }

        return d->imageSettingsPage->id();
    }

    return DWizardDlg::nextId();
}

GalleryInfo* HTMLWizard::galleryInfo() const
{
    return d->info;
}

GalleryTheme::Ptr HTMLWizard::galleryTheme() const
{
    return d->themePage->currentTheme();
}

} // namespace Digikam
