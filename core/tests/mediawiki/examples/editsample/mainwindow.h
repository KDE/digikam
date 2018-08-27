/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * Copyright (C) 2011      by Hormiere Guillaume <hormiere dot guillaume at gmail dot com>
 * Copyright (C) 2011      by Manuel Campomanes <campomanes dot manuel at gmail dot com>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes

#include <QMainWindow>
#include <QMessageBox>

// Local includes

#include "mediawiki_edit.h"
#include "mediawiki_iface.h"
#include "mediawiki_login.h"
#include "mediawiki_queryinfo.h"
#include "mediawiki_queryrevision.h"

namespace Ui
{
    class MainWindow;
}

using namespace mediawiki;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget* const parent = 0);
    ~MainWindow();

private Q_SLOTS:

    void on_plainTextEdit_textChanged();
    void on_mPageEdit_textChanged(QString);
    void on_mWikiEdit_textChanged(QString);
    void on_pushButton1_clicked();
    void on_pushButton2_clicked();
    void loginHandle(KJob* login);
    void revisionHandle(const QList<Revision>& revisions);
    void editError(KJob* job);
    void revisionError(KJob* job);

private:

    Ui::MainWindow* ui;
    Iface*      mediawiki;
};

#endif // MAINWINDOW_H
