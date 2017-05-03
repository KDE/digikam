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

#include "htmlfinalpage.h"

// Qt includes

#include <QSpacerItem>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "htmlwizard.h"
#include "abstractthemeparameter.h"
#include "galleryinfo.h"
#include "gallerygenerator.h"
#include "dwidgetutils.h"

namespace Digikam
{

HTMLFinalPage::HTMLFinalPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title)
{
    setObjectName(QLatin1String("FinalPage"));

    DVBox* const vb = new DVBox(this);

    mProgressView = new DHistoryView(vb);
    mProgressBar  = new DProgressWdg(vb);

    vb->setStretchFactor(mProgressBar, 10);

    setPageWidget(vb);
}

HTMLFinalPage::~HTMLFinalPage()
{
}

void HTMLFinalPage::initializePage()
{
    HTMLWizard* const wizard                = dynamic_cast<HTMLWizard*>(assistant());

    GalleryInfo* const info                 = wizard->galleryInfo();
    info->mCollectionList                   = wizard->albums();
    Theme::Ptr theme                        = wizard->theme();

    QString themeInternalName               = theme->internalName();
    info->setTheme(themeInternalName);

    Theme::ParameterList parameterList      = theme->parameterList();
    Theme::ParameterList::ConstIterator it  = parameterList.constBegin();
    Theme::ParameterList::ConstIterator end = parameterList.constEnd();

    for (; it != end ; ++it)
    {
        AbstractThemeParameter* const themeParameter = *it;
        QByteArray parameterInternalName             = themeParameter->internalName();
        QWidget* const widget                        = wizard->parametersWidget(parameterInternalName);
        QString value                                = themeParameter->valueFromWidget(widget);

        info->setThemeParameterValue(themeInternalName,
                                     QString::fromLatin1(parameterInternalName),
                                     value);
    }

    wizard->updateSettings();
    info->save();

    // Generate GalleryInfo

    GalleryGenerator generator(info);
    generator.setProgressWidgets(mProgressView, mProgressBar);

    if (!generator.run())
    {
        return;
    }

    if (generator.warnings())
    {
        mProgressView->addEntry(i18n("Finished, but some warnings occurred."), DHistoryView::WarningEntry);
    }

    if (info->openInBrowser())
    {
        QUrl url = info->destUrl();
        url.setPath(QLatin1String("index.html"));
        QDesktopServices::openUrl(url);
    }
}

} // namespace Digikam
