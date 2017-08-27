/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of an application named HUpnpAvSimpleTestApp
 *  used for demonstrating how to use the Herqq UPnP A/V (HUPnPAv) library.
 *
 *  HUpnpAvSimpleTestApp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HUpnpAvSimpleTestApp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HUpnpAvSimpleTestApp. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QMainWindow>
#else
#include <QtWidgets/QMainWindow>
#endif

namespace Ui {
    class MainWindow;
}

//
// Main window for the test application.
//
class MediaServerIntroductionWindow :
    public QMainWindow
{
Q_OBJECT
Q_DISABLE_COPY(MediaServerIntroductionWindow)

public:

    explicit MediaServerIntroductionWindow(QWidget* parent = 0);
    virtual ~MediaServerIntroductionWindow();

protected:

    virtual void changeEvent(QEvent*);

private:

    Ui::MainWindow* m_ui;

private Q_SLOTS:


    void on_startMediaServer_clicked();
    void serverWindowClosed();
};

#endif // MAINWINDOW_H
