/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : a presentation tool.
 *
 * Copyright (C) 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PRESENTATION_DLG_H
#define PRESENTATION_DLG_H

// Local includes

#include <QDialog>

namespace Digikam
{

class PresentationContainer;

class PresentationDlg : public QDialog
{
    Q_OBJECT

public:

    explicit PresentationDlg(QWidget* const parent, PresentationContainer* const sharedData);
    ~PresentationDlg();

Q_SIGNALS:

    void buttonStartClicked();

private:

    void readSettings();
    void saveSettings();
    void closeEvent(QCloseEvent* e);

private Q_SLOTS:

    void slotStartClicked();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PRESENTATION_DLG_H
