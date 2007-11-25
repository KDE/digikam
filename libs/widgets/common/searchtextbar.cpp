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

#include <qcolor.h>
#include <qpalette.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

// KDE includes.

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdialogbase.h>

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
        clearButton = 0;
    }

    QLabel      *searchLabel;

    QToolButton *clearButton;

    KLineEdit   *searchEdit;
};

SearchTextBar::SearchTextBar(QWidget *parent)
                 : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new SearchTextBarPriv;
    setFocusPolicy(QWidget::NoFocus);

    QHBoxLayout *hlay = new QHBoxLayout(this);

    d->clearButton = new QToolButton(this);
    d->clearButton->setAutoRaise(true);
    d->clearButton->setIconSet(kapp->iconLoader()->loadIcon("locationbar_erase",
                               KIcon::Toolbar, KIcon::SizeSmall));

    d->searchLabel = new QLabel(i18n("Search:"), this);
    d->searchEdit  = new KLineEdit(this);
    d->searchEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    hlay->setSpacing(KDialog::spacingHint());
    hlay->setMargin(0);
    hlay->addWidget(d->clearButton);
    hlay->addWidget(d->searchLabel);
    hlay->addWidget(d->searchEdit);

    connect(d->clearButton, SIGNAL(clicked()),
            d->searchEdit, SLOT(clear()));

    connect(d->searchEdit, SIGNAL(textChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));
}

SearchTextBar::~SearchTextBar()
{
    delete d;
}

void SearchTextBar::slotSearchResult(bool match)
{
    if (d->searchEdit->text().isEmpty())
    {
        d->searchEdit->unsetPalette();
        return;
    }

    QPalette pal = d->searchEdit->palette();
    pal.setColor(QPalette::Active, QColorGroup::Base,
                 match ?  QColor(200, 255, 200) :
                 QColor(255, 200, 200));
    d->searchEdit->setPalette(pal);
}

}  // namespace Digikam
