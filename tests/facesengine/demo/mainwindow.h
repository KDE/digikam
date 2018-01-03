/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : GUI test program for FacesEngine
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Alex Jironkin <alexjironkin at gmail dot com>
 * Copyright (C)      2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget* const parent = 0);
    ~MainWindow();

public Q_SLOTS:

    void slotOpenImage();
    void slotDetectFaces();
    void slotUpdateDatabase();
    void slotRecognise();
    void slotUpdateAccuracy();
    void slotUpdateSensitivity();

protected:

    void changeEvent(QEvent* e);

private:

    void clearScene();

private:

    class Private;
    Private* const d;
};

#endif // MAINWINDOW_H
