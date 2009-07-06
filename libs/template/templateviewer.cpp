/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-29
 * Description : metadata template viewer.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "templateviewer.h"

// Qt includes

#include <QGridLayout>

// KDE includes

#include <klocale.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kvbox.h>

// Local includes

#include "imagepropertiestxtlabel.h"
#include "template.h"
#include "templatemanager.h"

namespace Digikam
{

class TemplateViewerPriv
{
public:

    TemplateViewerPriv()
    {
        names                      = 0;
        position                   = 0;
        credit                     = 0;
        copyright                  = 0;
        usages                     = 0;
        source                     = 0;
        instructions               = 0;
        labelNames                 = 0;
        labelPosition              = 0;
        labelCredit                = 0;
        labelCopyright             = 0;
        labelUsages                = 0;
        labelSource                = 0;
        labelInstructions          = 0;
        locationCountry            = 0;
        locationCountryCode        = 0;
        locationProvinceState      = 0;
        locationCity               = 0;
        locationLocation           = 0;
        labelLocationCountry       = 0;
        labelLocationCountryCode   = 0;
        labelLocationProvinceState = 0;
        labelLocationCity          = 0;
        labelLocationLocation      = 0;
    }

    // IPTC Rights info.

    DTextLabelName *names;
    DTextLabelName *position;
    DTextLabelName *credit;
    DTextLabelName *copyright;
    DTextLabelName *usages;
    DTextLabelName *source;
    DTextLabelName *instructions;

    DTextBrowser   *labelPosition;
    DTextBrowser   *labelCredit;
    DTextBrowser   *labelCopyright;
    DTextBrowser   *labelUsages;
    DTextBrowser   *labelSource;
    DTextBrowser   *labelNames;
    DTextBrowser   *labelInstructions;

    // IPTC Location info.

    DTextLabelName *locationCountry;
    DTextLabelName *locationCountryCode;
    DTextLabelName *locationProvinceState;
    DTextLabelName *locationCity;
    DTextLabelName *locationLocation;

    DTextBrowser   *labelLocationCountry;
    DTextBrowser   *labelLocationCountryCode;
    DTextBrowser   *labelLocationProvinceState;
    DTextBrowser   *labelLocationCity;
    DTextBrowser   *labelLocationLocation;

    // IPTC Contact info.

};

TemplateViewer::TemplateViewer(QWidget* parent=0)
              : RExpanderBox(parent), d(new TemplateViewerPriv)
{
    setFrameStyle(QFrame::NoFrame);

    KVBox *w1            = new KVBox(this);
    d->names             = new DTextLabelName(i18n("Names:"), w1);
    d->labelNames        = new DTextBrowser(QString(), w1);
    d->position          = new DTextLabelName(i18n("Position:"), w1);
    d->labelPosition     = new DTextBrowser(QString(), w1);
    d->credit            = new DTextLabelName(i18n("Credit:"), w1);
    d->labelCredit       = new DTextBrowser(QString(), w1);
    d->copyright         = new DTextLabelName(i18n("Copyright:"), w1);
    d->labelCopyright    = new DTextBrowser(QString(), w1);
    d->usages            = new DTextLabelName(i18n("Usages:"), w1);
    d->labelUsages       = new DTextBrowser(QString(), w1);
    d->source            = new DTextLabelName(i18n("Source:"), w1);
    d->labelSource       = new DTextBrowser(QString(), w1);
    d->instructions      = new DTextLabelName(i18n("Instructions:"), w1);
    d->labelInstructions = new DTextBrowser(QString(), w1);

    d->names->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->position->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->credit->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->copyright->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->credit->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->usages->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->source->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->instructions->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    addItem(w1, SmallIcon("flag-red"),
            i18n("Rights"), QString("Rights"), true);

    // ------------------------------------------------------------------

    KVBox *w2                     = new KVBox(this);
    d->locationCountry            = new DTextLabelName(i18n("Country:"), w2);
    d->labelLocationCountry       = new DTextBrowser(QString(), w2);
    d->locationCountryCode        = new DTextLabelName(i18n("Country Code:"), w2);
    d->labelLocationCountryCode   = new DTextBrowser(QString(), w2);
    d->locationProvinceState      = new DTextLabelName(i18n("Province State:"), w2);
    d->labelLocationProvinceState = new DTextBrowser(QString(), w2);
    d->locationCity               = new DTextLabelName(i18n("City:"), w2);
    d->labelLocationCity          = new DTextBrowser(QString(), w2);
    d->locationLocation           = new DTextLabelName(i18n("Sublocation:"), w2);
    d->labelLocationLocation      = new DTextBrowser(QString(), w2);

    d->locationCountry->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->locationCountryCode->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->locationProvinceState->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->locationCity->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->locationLocation->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    addItem(w2, SmallIcon("applications-internet"),
            i18n("Location"), QString("Location"), true);

    // ------------------------------------------------------------------

    KVBox *w3            = new KVBox(this);

    addItem(w3, SmallIcon("view-pim-contacts"),
            i18n("Contact"), QString("Contact"), true);

    addStretch();
}

TemplateViewer::~TemplateViewer()
{
    delete d;
}

void TemplateViewer::setTemplate(const Template& t)
{
    d->labelNames->setText(t.authors().join("\n"));
    d->labelPosition->setText(t.authorsPosition());
    d->labelCredit->setText(t.credit());
    d->labelCopyright->setText(t.copyright()["x-default"]);
    d->labelUsages->setText(t.rightUsageTerms()["x-default"]);
    d->labelSource->setText(t.source());
    d->labelInstructions->setText(t.instructions());

    d->labelLocationCountry->setText(t.locationInfo().country);
    d->labelLocationCountryCode->setText(t.locationInfo().countryCode);
    d->labelLocationProvinceState->setText(t.locationInfo().provinceState);
    d->labelLocationCity->setText(t.locationInfo().city);
    d->labelLocationLocation->setText(t.locationInfo().location);
}

}  // namespace Digikam
