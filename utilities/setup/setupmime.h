/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2003-05-03
 * Description : mime types setup tab.
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

#ifndef SETUPMIME_H
#define SETUPMIME_H

// Qt includes.

#include <qwidget.h>

class QLineEdit;

namespace Digikam
{

class SetupMime : public QWidget
{
    Q_OBJECT
    
public:

    SetupMime(QWidget* parent = 0);
    ~SetupMime();

    void applySettings();

private:

    void readSettings();

    QLineEdit *m_imageFileFilterEdit;
    QLineEdit *m_movieFileFilterEdit;
    QLineEdit *m_audioFileFilterEdit;
    QLineEdit *m_rawFileFilterEdit;
};

}  // namespace Digikam

#endif // SETUPMIME_H 
