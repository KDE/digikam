/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-28
 * Description : a dialog for showing the advancedrename tooltip
 *
 * Copyright (C) 2010-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "tooltipdialog.h"

// Qt includes

#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "tooltipcreator.h"

namespace Digikam
{

class TooltipDialog::Private
{
public:

    Private() :
        buttons(0),
        textBrowser(0)
    {
    }

    QDialogButtonBox* buttons;
    QTextBrowser*     textBrowser;
};

TooltipDialog::TooltipDialog(QWidget* const parent)
    : QDialog(parent), d(new Private)
{
    setWindowTitle(i18n("Information"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    d->buttons->button(QDialogButtonBox::Close)->setDefault(true);

    d->textBrowser = new QTextBrowser(this);
    d->textBrowser->setFrameStyle(QFrame::NoFrame);
    d->textBrowser->setOpenLinks(true);
    d->textBrowser->setOpenExternalLinks(true);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->textBrowser);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    connect(d->buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(accept()));
}

TooltipDialog::~TooltipDialog()
{
    delete d;
}

void TooltipDialog::setTooltip(const QString& tooltip)
{
    clearTooltip();

    // set image resources
    d->textBrowser->document()->addResource(QTextDocument::ImageResource,
                                            QUrl::fromLocalFile(TooltipCreator::getInstance().getInfoIconResourceName()),
                                            TooltipCreator::getInstance().getInfoIcon());
    d->textBrowser->setHtml(tooltip);
}

void TooltipDialog::clearTooltip()
{
    d->textBrowser->clear();
}

} // namespace Digikam
