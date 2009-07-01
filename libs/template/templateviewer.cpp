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
#include "templateviewer.moc"

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
        names             = 0;
        position          = 0;
        credit            = 0;
        copyright         = 0;
        usages            = 0;
        source            = 0;
        labelNames        = 0;
        labelPosition     = 0;
        labelCredit       = 0;
        labelCopyright    = 0;
        labelUsages       = 0;
        labelSource       = 0;
        labelInstructions = 0;
    }

    DTextLabelName *names;
    DTextLabelName *position;
    DTextLabelName *credit;
    DTextLabelName *copyright;
    DTextLabelName *usages;
    DTextLabelName *source;

    DTextBrowser   *labelPosition;
    DTextBrowser   *labelCredit;
    DTextBrowser   *labelCopyright;
    DTextBrowser   *labelUsages;
    DTextBrowser   *labelSource;
    DTextBrowser   *labelNames;
    DTextBrowser   *labelInstructions;
};

TemplateViewer::TemplateViewer(QWidget* parent=0)
              : RExpanderBox(parent), d(new TemplateViewerPriv)
{
    setFrameStyle(QFrame::NoFrame);

    KVBox *w1        = new KVBox(this);
    d->names         = new DTextLabelName(i18n("Names:"), w1);
    d->labelNames    = new DTextBrowser(QString(), w1);
    d->position      = new DTextLabelName(i18n("Position:"), w1);
    d->labelPosition = new DTextBrowser(QString(), w1);

    d->names->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->position->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    addItem(w1, SmallIcon("user-identity"),
            i18n("Authors"), QString("Authors"), true);

    // ------------------------------------------------------------------

    KVBox *w2         = new KVBox(this);
    d->credit         = new DTextLabelName(i18n("Credit:"), w2);
    d->labelCredit    = new DTextBrowser(QString(), w2);
    d->copyright      = new DTextLabelName(i18n("Copyright:"), w2);
    d->labelCopyright = new DTextBrowser(QString(), w2);
    d->usages         = new DTextLabelName(i18n("Usages:"), w2);
    d->labelUsages    = new DTextBrowser(QString(), w2);
    d->source         = new DTextLabelName(i18n("Source:"), w2);
    d->labelSource    = new DTextBrowser(QString(), w2);

    d->credit->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->copyright->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->credit->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->usages->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->source->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    addItem(w2, SmallIcon("flag-red"),
            i18n("Intellectual Property"), QString("CopyRight"), true);

    // ------------------------------------------------------------------

    KVBox *w3            = new KVBox(this);
    d->labelInstructions = new DTextBrowser(QString(), w3);

    addItem(w3, SmallIcon("view-pim-journal"),
            i18n("Instructions"), QString("Instructions"), true);

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
}

}  // namespace Digikam
