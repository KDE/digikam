/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-24
 * Description : a dialog to display processed messages in background
 *
 * Copyright (C) 2009-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "doutputdlg.h"

// Qt includes

#include <QLabel>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class DOutputDlg::Private
{
public:

    Private()
    {
        debugView = 0;
        buttonBox = 0;
    }

    QTextBrowser*     debugView;
    QDialogButtonBox* buttonBox;
};

DOutputDlg::DOutputDlg(QWidget* const parent, const QString& caption,
                       const QString& messages, const QString& header)
    : QDialog(parent),
      d(new Private)
{
    setModal(true);
    setWindowTitle(caption);

    // Create dialog buttons
    d->buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    d->buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    QPushButton* const copyButton = new QPushButton(QIcon::fromTheme(QString::fromLatin1("edit-copy")), i18n("Copy to Clip&board"));
    d->buttonBox->addButton(copyButton, QDialogButtonBox::ActionRole);

    // Create dialog contents
    QLabel* const lHeader = new QLabel(this);
    d->debugView = new QTextBrowser(this);
    d->debugView->append(messages);
    lHeader->setText(header);

    // Put contained widgets together in a vertical layout
    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(lHeader);
    mainLayout->addWidget(d->debugView);
    mainLayout->addWidget(d->buttonBox);

    connect(d->buttonBox, &QDialogButtonBox::accepted,
            this, &DOutputDlg::accept);

    connect(copyButton, &QPushButton::clicked,
            this, &DOutputDlg::slotCopyToCliboard);

    resize(600, 400);
}

DOutputDlg::~DOutputDlg()
{
    delete d;
}

void DOutputDlg::slotCopyToCliboard()
{
    d->debugView->selectAll();
    d->debugView->copy();
    d->debugView->setPlainText(d->debugView->toPlainText());
}

}  // namespace Digikam
