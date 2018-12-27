/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QFile>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      MediaWiki(QUrl(QLatin1String("https://test.wikipedia.org/w/api.php")))
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
    this->ui->comboBox->addItem(QLatin1String("Own work, multi-license with CC-BY-SA-3.0 and GFDL"),                        QLatin1String("{{self|cc-by-sa-3.0|GFDL|migration=redundant}}"));
    this->ui->comboBox->addItem(QLatin1String("Own work, multi-license with CC-BY-SA-3.0 and older"),                       QLatin1String("{{self|cc-by-sa-3.0,2.5,2.0,1.0}}"));
    this->ui->comboBox->addItem(QLatin1String("Creative Commons Attribution-Share Alike 3.0"),                              QLatin1String("{{self|cc-by-sa-3.0}}"));
    this->ui->comboBox->addItem(QLatin1String("Own work, Creative Commons Attribution 3.0"),                                QLatin1String("{{self|cc-by-3.0}}"));
    this->ui->comboBox->addItem(QLatin1String("Own work, release into public domain under the CC-Zero license"),            QLatin1String("{{self|cc-zero}}"));
    this->ui->comboBox->addItem(QLatin1String("Author died more than 100 years ago"),                                       QLatin1String("{{PD-old}}"));
    this->ui->comboBox->addItem(QLatin1String("Photo of a two-dimensional work whose author died more than 100 years ago"), QLatin1String("{{PD-art}}"));
    this->ui->comboBox->addItem(QLatin1String("First published in the United States before 1923"),                          QLatin1String("{{PD-US}}"));
    this->ui->comboBox->addItem(QLatin1String("Work of a U.S. government agency"),                                          QLatin1String("{{PD-USGov}}"));
    this->ui->comboBox->addItem(QLatin1String("Simple typefaces, individual words or geometric shapes"),                    QLatin1String("{{PD-text}}"));
    this->ui->comboBox->addItem(QLatin1String("Logos with only simple typefaces, individual words or geometric shapes"),    QLatin1String("{{PD-textlogo}}"));
}

void MainWindow::on_pushButton_clicked()
{
    this->ui->progressBar->setValue(0);
    Login* const login = new Login(MediaWiki, this->ui->mLoginEdit->text(), this->ui->mMdpEdit->text());

    connect(login, SIGNAL(result(KJob*)),
            this, SLOT(loginHandle(KJob*)));

    login->start();
}

void MainWindow::loginHandle(KJob* login)
{
    if (login->error() != 0)
    {
        QMessageBox popup;
        popup.setText(QLatin1String("Wrong authentication."));
        popup.exec();
    }
    else
    {
        Upload* const e1  = new Upload( MediaWiki );
        QFile* const file = new QFile(this->ui->lineEdit->text());
        file->open(QIODevice::ReadOnly);
        e1->setFile(file);
        e1->setFilename(this->ui->lineEdit_2->text());

        QString text = QLatin1String("== {{int:filedesc}} == \n{{Information |Description=");
        text.append(this->ui->descriptionEdit->text());
        text.append(QLatin1String("\n|Source=")).append(this->ui->sourceEdit->text());
        text.append(QLatin1String("\n|Date=")).append(this->ui->dateEdit->text());
        text.append(QLatin1String("\n|Author=")).append(this->ui->authorEdit->text());
        text.append(QLatin1String("\n|Permission=")).append(this->ui->permissionEdit->text());
        text.append(QLatin1String("\n|other_versions=")).append(this->ui->versionsEdit->text());
        text.append(QLatin1String("\n}}\n== {{int:license}} ==\n"));
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

    if (job->error() == 0)
        errorMessage = QLatin1String("Image uploaded successfully.");
    else
        errorMessage = QLatin1String("Image upload failed.");

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
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    QLatin1String("Open Image"),
                                                    QLatin1String("~"),
                                                    QLatin1String("Image Files (*.png *.jpg *.bmp *.jpeg *.gif)"));

    if (!fileName.isEmpty())
    {
        QPixmap preview(fileName);
        QSize size(preview.size());
        size.scale(400, 200, Qt::KeepAspectRatio);
        preview = preview.scaled(size, Qt::KeepAspectRatio, Qt::FastTransformation);

        this->ui->previewLabel->setPixmap(preview);
        this->ui->lineEdit->setText(fileName);
    }
}

void MainWindow::on_lineEdit_textChanged(QString text)
{
    this->ui->pushButton->setEnabled(!text.isEmpty() && !text.isNull());
}
