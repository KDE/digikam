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

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* const parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      mediawiki(0)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//Load page
void MainWindow::on_pushButton2_clicked()
{
    mediawiki = new MediaWiki(QUrl(this->ui->mWikiEdit->text()));
    QueryRevision* const queryrevision(new QueryRevision(*mediawiki));
    queryrevision->setPageName(this->ui->mPageEdit->text());
    queryrevision->setProperties(QueryRevision::Content);
    queryrevision->setExpandTemplates(true);
    queryrevision->setLimit(1);

    connect(queryrevision, SIGNAL(revision(QList<Revision>)),
            this, SLOT(revisionHandle(QList<Revision>)));

    connect(queryrevision, SIGNAL(result(KJob*)),
            this, SLOT(revisionError(KJob*)));

    queryrevision->start();
}

void MainWindow::revisionHandle(const QList<Revision>& revisions)
{
    if (revisions.isEmpty())
    {
        QMessageBox popup;
        popup.setText(QLatin1String("This page doesn't exist."));
        popup.exec();
        return;
    }

    this->ui->plainTextEdit->setPlainText(revisions[0].content());
}

//Send page
void MainWindow::on_pushButton1_clicked()
{
    Login* const login = new Login(*mediawiki, this->ui->mLoginEdit->text(), this->ui->mMdpEdit->text());

    connect(login, SIGNAL(result(KJob*)),
            this, SLOT(loginHandle(KJob*)));

    login->start();
}

void MainWindow::loginHandle(KJob* login)
{
    if (login->error()!= 0)
    {
        QMessageBox popup;
        popup.setText(QLatin1String("Wrong authentication."));
        popup.exec();
    }
    else
    {
        Edit* const job = new Edit(*mediawiki, NULL);
        job->setPageName(this->ui->mPageEdit->text());
        job->setText(this->ui->plainTextEdit->toPlainText());

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(editError(KJob*)));

        job->start();
    }
}

void MainWindow::editError(KJob* job)
{
    QString errorMessage;

    if (job->error() == 0)
        errorMessage = QLatin1String("The Wiki page modified successfully.");
    else
        errorMessage = QLatin1String("The Wiki page can't be modified.");

    QMessageBox popup;
    popup.setText(errorMessage);
    popup.exec();
}

void MainWindow::revisionError(KJob* job)
{
    if(job->error() != 0)
    {
        QMessageBox popup;
        popup.setText(QLatin1String("The Wiki page can't be loaded."));
        popup.exec();
    }
}

void MainWindow::on_mPageEdit_textChanged(QString text)
{
    this->ui->pushButton2->setEnabled(!text.isEmpty() && !text.isNull() && !this->ui->mWikiEdit->text().isEmpty());
}

void MainWindow::on_mWikiEdit_textChanged(QString text)
{
    this->ui->pushButton2->setEnabled(!text.isEmpty() && !text.isNull() && !this->ui->mPageEdit->text().isEmpty());
}

void MainWindow::on_plainTextEdit_textChanged()
{
    QString text = this->ui->plainTextEdit->toPlainText();
    this->ui->pushButton1->setEnabled(!text.isEmpty() && !text.isNull());
}
