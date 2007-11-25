/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-25
 * Description : a bar used to search a text somewhere.
 * 
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QColor>
#include <QPalette>
#include <QLabel>
#include <QLayout>

// KDE includes.

#include <kapplication.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdialog.h>

// Local includes

#include "searchtextbar.h"
#include "searchtextbar.moc"

namespace Digikam
{

class SearchTextBarPriv
{
public:

    SearchTextBarPriv()
    {
        searchEdit  = 0;
        searchLabel = 0;
    }

    QLabel      *searchLabel;

    KLineEdit   *searchEdit;
};

SearchTextBar::SearchTextBar(QWidget *parent)
             : QWidget(parent)
{
    d = new SearchTextBarPriv;
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *hlay = new QHBoxLayout(this);

    d->searchLabel = new QLabel(i18n("Search:"), this);
    d->searchEdit  = new KLineEdit(this);
    d->searchEdit->setClearButtonShown(true);
    d->searchEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    hlay->setSpacing(KDialog::spacingHint());
    hlay->setMargin(0);
    hlay->addWidget(d->searchLabel);
    hlay->addWidget(d->searchEdit);

    connect(d->searchEdit, SIGNAL(textChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));
}

SearchTextBar::~SearchTextBar()
{
    delete d;
}

void SearchTextBar::setText(const QString& text)
{
    d->searchEdit->setText(text);
}

QString SearchTextBar::text() const
{
    return d->searchEdit->text();
}

void SearchTextBar::slotSearchResult(bool match)
{
    if (d->searchEdit->text().isEmpty())
    {
        d->searchEdit->setPalette(QPalette());
        return;
    }

    QPalette pal = d->searchEdit->palette();
    pal.setColor(QPalette::Active, QColorGroup::Base,
                 match ?  QColor(200, 255, 200) :
                 QColor(255, 200, 200));
    d->searchEdit->setPalette(pal);
}

}  // namespace Digikam
