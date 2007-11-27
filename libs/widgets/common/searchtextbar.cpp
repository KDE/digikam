/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-25
 * Description : a bar used to search a string.
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
#include <QString>

// KDE includes.

#include <klocale.h>

// Local includes

#include "searchtextbar.h"
#include "searchtextbar.moc"

namespace Digikam
{

SearchTextBar::SearchTextBar(QWidget *parent)
             : KLineEdit(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setClearButtonShown(true);
    setClickMessage(i18n("Search..."));
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTextChanged(const QString&)));
}

SearchTextBar::~SearchTextBar()
{
}

void SearchTextBar::slotTextChanged(const QString& text)
{
    if (text.isEmpty())
        setPalette(QPalette());
}

void SearchTextBar::slotSearchResult(bool match)
{
    if (text().isEmpty())
    {
        setPalette(QPalette());
        return;
    }

    QPalette pal = palette();
    pal.setColor(QPalette::Active, QColorGroup::Base,
                 match ?  QColor(200, 255, 200) :
                 QColor(255, 200, 200));
    setPalette(pal);
}

}  // namespace Digikam
