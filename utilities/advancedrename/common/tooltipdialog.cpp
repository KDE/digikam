/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-28
 * Description : a dialog for showing the advancedrename tooltip
 *
 * Copyright (C) 2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "tooltipdialog.moc"

// Qt includes

#include <QTextBrowser>

// KDE includes

#include <klocale.h>

namespace Digikam
{

class TooltipDialogPriv
{
public:

    TooltipDialogPriv() :
        textBrowser(0)
    {}

    QTextBrowser* textBrowser;
};

TooltipDialog::TooltipDialog(QWidget* parent)
             : KDialog(parent), d(new TooltipDialogPriv)
{
    d->textBrowser = new QTextBrowser(this);
    d->textBrowser->setFrameStyle(QFrame::NoFrame);
    d->textBrowser->setOpenLinks(true);
    d->textBrowser->setOpenExternalLinks(true);
    setCaption(i18n("Information"));
    setButtons(KDialog::Close);
    setMainWidget(d->textBrowser);
}

TooltipDialog::~TooltipDialog()
{
    delete d;
}

void TooltipDialog::setTooltip(const QString& tooltip)
{
    clearTooltip();
    d->textBrowser->setHtml(tooltip);
}

void TooltipDialog::clearTooltip()
{
    d->textBrowser->clear();
}

} // namespace Digikam
