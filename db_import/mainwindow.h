/*

This file is part of digikam database import tool.

    digikam database import tool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    digikam database import tool is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with digikam database import tool.  If not, see <http://www.gnu.org/licenses/>

*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString digikamConfigFilePath;
    QString digikamConfigFileName;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    QString getMyDocumentsWindowsFolder();
    QString getPictureProjetWindowsRootPath();
    QString getDigiKamDatabaseFile();

private slots:
    void on_importButton_clicked();
    void on_actionAbout_activated();
    void on_select_digikam_db_file_clicked();
    void on_actionExit_activated();
    void on_pushButton_2_clicked();
    void handleFinished();
};

#endif // MAINWINDOW_H
