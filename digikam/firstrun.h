/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-25
 * Description : a widget to use in first run dialog
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAMFIRSTFIRSTRUNWIDGET_H
#define DIGIKAMFIRSTFIRSTRUNWIDGET_H

// Qt includes.

#include <qwidget.h>

// Local includes.

#include "digikam_export.h"

class QLabel;

class KURLRequester;

namespace Digikam 
{

class DIGIKAM_EXPORT FirstRunWidget : public QWidget
{
    Q_OBJECT

public:
    
    FirstRunWidget( QWidget* parent = 0 );
    ~FirstRunWidget();

public:
    
    QLabel        *m_pixLabel;
    KURLRequester *m_path;
    
protected slots:
    
    virtual void languageChange();
    
private:

    QLabel *m_textLabel1;
    QLabel *m_textLabel2;
};

}  // namespace Digikam

#endif // DIGIKAMFIRSTFIRSTRUNWIDGET_H
