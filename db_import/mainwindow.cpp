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

// Qt includes

#include <qfiledialog.h>
#include <QSettings>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVector>
#include <QThread>
#include <QtConcurrentRun>
#include <QFutureWatcher>

// Local includes

#include "mainwindow.h"
#include "import.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QDir filePath;
    digikamConfigFileName = "/digikam4.db";

    ui->setupUi(this);

    #ifdef Q_WS_WIN
    digikamConfigFilePath = ".kde/share/config/digikamrc";
    ui->lineEdit->setText(MainWindow::getPictureProjetWindowsRootPath());
    ui->digikamDatabaseSqliteFilePath->setText(MainWindow::getDigiKamDatabaseFile());
    #elif defined(Q_WS_X11)
    #elif defined(Q_WS_MAC)
    #endif

    if (filePath.exists(MainWindow::getDigiKamDatabaseFile()) == TRUE)
    {
        ui->importButton->setEnabled(TRUE);
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (dir != "")
        {
        ui->lineEdit->setText(dir);
        }


    ui->importButton->setEnabled(TRUE);
}

void MainWindow::on_actionExit_activated()
{
    qApp->exit(0);
}

QString MainWindow::getMyDocumentsWindowsFolder()
{
    QSettings settings(QSettings::UserScope, "Microsoft", "Windows");
    settings.beginGroup("CurrentVersion/Explorer/Shell Folders");
    return settings.value("Personal").toString();
}

QString MainWindow::getPictureProjetWindowsRootPath()
{
    QSettings settings(QSettings::UserScope, "Nikon", "PictureProject");
    settings.beginGroup("MainFrame/Database");
    QDir dir = QDir::fromNativeSeparators(settings.value("RootPath").toString());
    return dir.absolutePath();
}

QString MainWindow::getDigiKamDatabaseFile()
{

        QString appDataPath , digikamConfigFileFullPath;

        #ifdef Q_WS_WIN

        QSettings settings(QSettings::UserScope, "Microsoft", "Windows");
        settings.beginGroup("CurrentVersion/Explorer/Shell Folders");
        appDataPath =  settings.value("AppData").toString();
        QSettings settings2(appDataPath + "\\" + digikamConfigFilePath , QSettings::IniFormat);
        settings2.beginGroup("Album Settings");
        digikamConfigFileFullPath = settings2.value("Database File Path").toString() + digikamConfigFileName;

        #elif defined(Q_WS_X11)
        #elif defined(Q_WS_MAC)
        #endif

        return digikamConfigFileFullPath;

}

void MainWindow::on_select_digikam_db_file_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select database"),
                                                    QDir::homePath(),
                                                    tr("SQLite (*.sqlite *.db)"));
    if (file != "")
        {
        ui->digikamDatabaseSqliteFilePath->setText(file);
        }
}

void MainWindow::on_actionAbout_activated()
{
    QMessageBox msgBox;
    msgBox.setText(tr("digiKam database import tool by Laurent Espitallier"));
    msgBox.setDetailedText("Hi there !");
    msgBox.exec();
}

void MainWindow::on_importButton_clicked()
{

    bool db_digikam_ok, db_npp_ok;
    import *dbImportInstance = new import;

    ui->importButton->setEnabled(FALSE);

    // Connexion a la base de donnees de digiKam
    db_digikam_ok = dbImportInstance->connect_db_digikam(ui->digikamDatabaseSqliteFilePath->text());

    // Connexion aux bases de donnees de PictureProject
    db_npp_ok = dbImportInstance->connect_db_npp(ui->lineEdit->text());

    if ((db_digikam_ok == TRUE) && (db_npp_ok == TRUE))
         {

         QFutureWatcher<void> watcher;
         QFuture<void> future = QtConcurrent::run(dbImportInstance,&import::NPP_import);
         QObject::connect(&watcher, SIGNAL(finished()), this, SLOT(handleFinished()));
         QObject::connect(dbImportInstance, SIGNAL(updateStatusBar(QString)), ui->current_action, SLOT(setText(QString)));
         watcher.setFuture(future);

         }
         else
         {
             QMessageBox::critical(this, tr("Error"), tr("Unable to open database") + ui->digikamDatabaseSqliteFilePath->text());
         }  

}

void MainWindow::handleFinished()
{
    qDebug() << "finished";
}

