/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-11-12
 * Description : a common login dialog for export tools
 *
 * Copyright (C) 2007-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2011      by Roman Tsisyk <roman at tsisyk dot com>
 * Copyright (C) 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>

// Local includes

#include "digikam_export.h"

class QLabel;
class QLineEdit;

namespace Digikam
{

class DIGIKAM_EXPORT LoginDialog : public QDialog
{
    Q_OBJECT

public:

    explicit LoginDialog(QWidget* const parent, const QString& prompt,
                         const QString& header=QString(), const QString& passwd=QString());
    ~LoginDialog();

    QString login()    const;
    QString password() const;

    void setLogin(const QString&);
    void setPassword(const QString&);

protected Q_SLOTS:

    void slotAccept();

protected:

    QLabel*    m_headerLabel;
    QLineEdit* m_loginEdit;
    QLineEdit* m_passwordEdit;
};

} // namespace Digikam

#endif // LOGIN_DIALOG_H
