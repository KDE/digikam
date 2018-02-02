/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-01
 * Description : new album creation dialog for remote web service.
 *
 * Copyright (C) 2010 by Jens Mueller <tschenser at gmx dot de>
 * Copyright (C) 2015 by Shourya Singh Gupta <shouryasgupta at gmail dot com>
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

#ifndef WS_NEW_ALBUM_DIALOG_H
#define WS_NEW_ALBUM_DIALOG_H

// Qt includes

#include <QRadioButton>
#include <QDateTimeEdit>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCloseEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT WSNewAlbumDialog : public QDialog
{
    Q_OBJECT

public:

    explicit WSNewAlbumDialog(QWidget* const parent, const QString& toolName);
    ~WSNewAlbumDialog();

    void hideDateTime();
    void hideDesc();
    void hideLocation();

    QWidget*          getMainWidget()   const;
    QGroupBox*        getAlbumBox()     const;

    QLineEdit*        getTitleEdit()    const;
    QTextEdit*        getDescEdit()     const;
    QLineEdit*        getLocEdit()      const;
    QDateTimeEdit*    getDateTimeEdit() const;
    QDialogButtonBox* getButtonBox()    const;

    void addToMainLayout(QWidget* const widget);

private Q_SLOTS:

    void slotTextChanged(const QString& text);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // WS_NEW_ALBUM_DIALOG_H
