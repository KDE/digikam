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

#ifndef SEARCH_TEXT_BAR_H
#define SEARCH_TEXT_BAR_H

// KDE includes.

#include <klineedit.h>
#include <klocale.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT SearchTextBar : public KLineEdit
{
Q_OBJECT

public:

    SearchTextBar(QWidget *parent=0, const QString& msg=i18n("Search..."));
    ~SearchTextBar();

public slots:

    void slotSearchResult(bool);

private slots:

    void slotTextChanged(const QString&);
};

}  // namespace Digikam

#endif /* SEARCH_TEXT_BAR_H */
