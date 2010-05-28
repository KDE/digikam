/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-28
 * Description : a dialog for showing the advancedrename tooltip
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include <QTextEdit>

// KDE includes

#include <klocale.h>

// Local includes

#include "parser.h"

namespace Digikam
{

class TooltipDialogPriv
{
public:

    TooltipDialogPriv() :
        textEdit(0)
    {}

    QTextEdit* textEdit;
};

TooltipDialog::TooltipDialog(QWidget* parent)
             : KDialog(parent), d(new TooltipDialogPriv)
{
    d->textEdit = new QTextEdit(this);
    d->textEdit->setFrameStyle(QFrame::NoFrame);
    setCaption(i18n("Information"));
    setButtons(KDialog::Close);
    setMainWidget(d->textEdit);
}

TooltipDialog::~TooltipDialog()
{
    delete d;
}

void TooltipDialog::setTooltip(const QString& tooltip)
{
    clearTooltip();
    d->textEdit->setHtml(tooltip);
    d->textEdit->setReadOnly(true);
}

void TooltipDialog::clearTooltip()
{
    d->textEdit->clear();
}

} // namespace Digikam
