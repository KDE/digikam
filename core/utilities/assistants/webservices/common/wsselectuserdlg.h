/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-16-05
 * Description : a dialog to select user for Web Service tools
 *
 * Copyright (C) 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WS_SELECT_USER_DLG_H
#define WS_SELECT_USER_DLG_H

// Qt includes

#include <QString>
#include <QDialog>

namespace Digikam
{

class WSSelectUserDlg : public QDialog
{
    Q_OBJECT

public:

    explicit WSSelectUserDlg(QWidget* const parent, const QString& serviceName);
    ~WSSelectUserDlg();

    void    reactivate();
    QString getUserName() const;

protected Q_SLOTS:

    void slotOkClicked();
    void slotNewAccountClicked();

private:
    
    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // WS_SELECT_USER_DLG_H
