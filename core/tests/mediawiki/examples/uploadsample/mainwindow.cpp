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

#include <QFile>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      mediawiki(QUrl(QLatin1String("https://test.wikipedia.org/w/api.php")))
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    this->ui->comboBox->addItem(QString("Own work, multi-license with CC-BY-SA-3.0 and GFDL"),QString("{{self|cc-by-sa-3.0|GFDL|migration=redundant}}"));
    this->ui->comboBox->addItem(QString("Own work, multi-license with CC-BY-SA-3.0 and older"),QString("{{self|cc-by-sa-3.0,2.5,2.0,1.0}}"));
    this->ui->comboBox->addItem(QString("Creative Commons Attribution-Share Alike 3.0"),QString("{{self|cc-by-sa-3.0}}"));
    this->ui->comboBox->addItem(QString("Own work, Creative Commons Attribution 3.0"),QString("{{self|cc-by-3.0}}"));
    this->ui->comboBox->addItem(QString("Own work, release into public domain under the CC-Zero license"),QString("{{self|cc-zero}}"));
    this->ui->comboBox->addItem(QString("Author died more than 100 years ago"),QString("{{PD-old}}"));
    this->ui->comboBox->addItem(QString("Photo of a two-dimensional work whose author died more than 100 years ago"),QString("{{PD-art}}"));
    this->ui->comboBox->addItem(QString("First published in the United States before 1923"),QString("{{PD-US}}"));
    this->ui->comboBox->addItem(QString("Work of a U.S. government agency"),QString("{{PD-USGov}}"));
    this->ui->comboBox->addItem(QString("Simple typefaces, individual words or geometric shapes"),QString("{{PD-text}}"));
    this->ui->comboBox->addItem(QString("Logos with only simple typefaces, individual words or geometric shapes"),QString("{{PD-textlogo}}"));
}

void MainWindow::on_pushButton_clicked()
{
    this->ui->progressBar->setValue(0);
    Login* login = new Login(mediawiki, this->ui->mLoginEdit->text(), this->ui->mMdpEdit->text());
    connect(login, SIGNAL(result(KJob*)),
            this, SLOT(loginHandle(KJob*)));
    login->start();
}

void MainWindow::loginHandle(KJob* login)
{
    if(login->error() != 0)
    {
        QMessageBox popup;
        popup.setText("Wrong authentication.");
        popup.exec();
    }
    else
    {
        Upload* e1  = new Upload( mediawiki );
        QFile* file = new QFile(this->ui->lineEdit->text());
        file->open(QIODevice::ReadOnly);
        e1->setFile(file);
        e1->setFilename(this->ui->lineEdit_2->text());

        QString text("== {{int:filedesc}} == \n{{Information |Description=");
        text.append(this->ui->descriptionEdit->text());
        text.append("\n|Source=").append(this->ui->sourceEdit->text());
        text.append("\n|Date=").append(this->ui->dateEdit->text());
        text.append("\n|Author=").append(this->ui->authorEdit->text());
        text.append("\n|Permission=").append(this->ui->permissionEdit->text());
        text.append("\n|other_versions=").append(this->ui->versionsEdit->text());
        text.append("\n}}\n== {{int:license}} ==\n");
        text.append(this->ui->comboBox->itemData(this->ui->comboBox->currentIndex()).toString());

        e1->setText(text);
        connect(e1, SIGNAL(result(KJob*)),
                this, SLOT(uploadHandle(KJob*)));

        connect(e1,SIGNAL(processedSize(KJob*,qulonglong)),
                this, SLOT(processedUploadSize(KJob*,qulonglong)));

        connect(e1,SIGNAL(totalSize(KJob*,qulonglong)),
                this,SLOT(TotalUploadSize(KJob*,qulonglong)));
        e1->start();
    }
}

void MainWindow::uploadHandle(KJob* job)
{
    disconnect(this, SIGNAL(result(KJob*)),
               this, SLOT(uploadHandle(KJob*)));

    disconnect(this, SIGNAL(processedSize(KJob*,qulonglong)),
               this, SLOT(processedUploadSize(KJob*,qulonglong)));

    disconnect(this, SIGNAL(totalSize(KJob*,qulonglong)),
               this, SLOT(TotalUploadSize(KJob*,qulonglong)));

    QString errorMessage;
    if(job->error() == 0) errorMessage = "Image uploaded successfully.";
    else errorMessage = "Image upload failed.";
    QMessageBox popup;
    popup.setText(errorMessage);
    popup.exec();
}

void MainWindow::processedUploadSize(KJob* job, qulonglong size)
{
    Q_UNUSED(job)
    this->ui->progressBar->setValue(size);
}

void MainWindow::TotalUploadSize(KJob* job, qulonglong size)
{
    Q_UNUSED(job)
    this->ui->progressBar->setMaximum(size);
}

void MainWindow::on_parcourir_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, QLatin1String("Open Image"), "~", QLatin1String("Image Files (*.png *.jpg *.bmp *.jpeg *.gif)"));
    if(fileName != NULL)
    {
        QPixmap preview(fileName);
        QSize size(preview.size());
        size.scale(400,200,Qt::KeepAspectRatio);
        preview = preview.scaled(size, Qt::KeepAspectRatio,Qt::FastTransformation);

        this->ui->previewLabel->setPixmap(preview);
        this->ui->lineEdit->setText(fileName);
    }
}

void MainWindow::on_lineEdit_textChanged(QString text)
{
    this->ui->pushButton->setEnabled(!text.isEmpty() && !text.isNull());
}
