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

#ifndef SEARCH_TEXT_BAR_H
#define SEARCH_TEXT_BAR_H

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class SearchTextBarPriv;

class DIGIKAM_EXPORT SearchTextBar : public QWidget
{
Q_OBJECT

public:

    SearchTextBar(QWidget *parent=0);
    ~SearchTextBar();

signals:
    
    void signalTextChanged(const QString&);    

public slots:

    void slotSearchResult(bool);

private :

    SearchTextBarPriv* d;    
};

}  // namespace Digikam

#endif /* SEARCH_TEXT_BAR_H */
