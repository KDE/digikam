/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-15
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2010      by Jens Mueller <tschenser at gmx dot de>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "gsreplacedlg.h"

// Qt includes

#include <QIcon>
#include <QTimer>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QPushButton>
#include <QMimeDatabase>
#include <QDialogButtonBox>
#include <QNetworkAccessManager>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dworkingpixmap.h"
#include "wstoolutils.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class ReplaceDialog::Private
{
public:

    explicit Private()
    {
        progressPix     = DWorkingPixmap();
        bAdd            = 0;
        bAddAll         = 0;
        bReplace        = 0;
        bReplaceAll     = 0;
        iface           = 0;
        lbSrc           = 0;
        lbDest          = 0;
        netMngr         = 0;
        progressCount   = 0;
        progressTimer   = 0;
        result          = -1;
        thumbLoadThread = ThumbnailLoadThread::defaultThread();
    }

    QPushButton*           bAdd;
    QPushButton*           bAddAll;
    QPushButton*           bReplace;
    QPushButton*           bReplaceAll;
    QUrl                   src;
    QUrl                   dest;
    DInfoInterface*        iface;
    QLabel*                lbSrc;
    QLabel*                lbDest;
    QByteArray             buffer;
    QNetworkAccessManager* netMngr;
    QPixmap                mimePix;
    DWorkingPixmap         progressPix;
    ThumbnailLoadThread*   thumbLoadThread;
    int                    progressCount;
    QTimer*                progressTimer;
    int                    result;
};

ReplaceDialog::ReplaceDialog(QWidget* const parent,
                             const QString& _caption,
                             DInfoInterface* const iface,
                             const QUrl& src,
                             const QUrl& dest)
    : QDialog(parent),
      d(new Private)
{
    setObjectName(QString::fromLatin1("ReplaceDialog"));

    d->src   = src;
    d->dest  = dest;
    d->iface = iface;

    setWindowTitle(_caption);

    QDialogButtonBox* const buttonBox = new QDialogButtonBox();
    buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(cancelPressed()));

    d->bAdd = new QPushButton(buttonBox);
    d->bAdd->setText(i18n("Add As New"));
    d->bAdd->setToolTip(i18n("Item will be added alongside the linked version."));

    connect(d->bAdd, SIGNAL(clicked()),
            this, SLOT(addPressed()));

    d->bAddAll = new QPushButton(buttonBox);
    d->bAddAll->setText(i18n("Add All"));
    d->bAddAll->setToolTip(i18n("Items will be added alongside the linked version. You will not be prompted again."));

    connect(d->bAddAll, SIGNAL(clicked()),
            this, SLOT(addAllPressed()));

    d->bReplace = new QPushButton(buttonBox);
    d->bReplace->setText(i18n("Replace"));
    d->bReplace->setToolTip(i18n("Item will be replacing the linked version."));

    connect(d->bReplace, SIGNAL(clicked()),
            this, SLOT(replacePressed()));

    d->bReplaceAll = new QPushButton(buttonBox);
    d->bReplaceAll->setText(i18n("Replace All"));
    d->bReplaceAll->setToolTip(i18n("Items will be replacing the linked version. You will not be prompted again."));

    connect(d->bReplaceAll, SIGNAL(clicked()),
            this, SLOT(replaceAllPressed()));

    buttonBox->addButton(d->bAdd,        QDialogButtonBox::AcceptRole);
    buttonBox->addButton(d->bAddAll,     QDialogButtonBox::AcceptRole);
    buttonBox->addButton(d->bReplace,    QDialogButtonBox::AcceptRole);
    buttonBox->addButton(d->bReplaceAll, QDialogButtonBox::AcceptRole);

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    QVBoxLayout* const pLayout    = new QVBoxLayout(this);
    pLayout->addStrut(360);                          // makes dlg at least that wide

    QGridLayout* const gridLayout = new QGridLayout();
    pLayout->addLayout(gridLayout);

    QLabel* const lb1             = new QLabel(this);
    lb1->setText(i18n("A linked item already exists."));
    lb1->setAlignment(Qt::AlignHCenter);
    gridLayout->addWidget(lb1, 0, 0, 1, 3);

    QMimeDatabase db;
    QString icon = db.mimeTypeForUrl(d->dest).iconName();
    d->mimePix = QIcon::fromTheme(icon).pixmap(48);

    d->lbDest  = new QLabel(this);
    d->lbDest->setPixmap(d->mimePix);
    d->lbDest->setAlignment(Qt::AlignCenter);
    gridLayout->addWidget(d->lbDest, 1, 0, 1, 1);

    d->lbSrc   = new QLabel(this);
    icon = db.mimeTypeForUrl(d->src).iconName();
    d->lbSrc->setPixmap(QIcon::fromTheme(icon).pixmap(48));
    d->lbSrc->setAlignment(Qt::AlignCenter);
    gridLayout->addWidget(d->lbSrc, 1, 2, 1, 1);

    QLabel* const lb2 = new QLabel(this);
    lb2->setText(i18n("Destination"));
    lb2->setAlignment(Qt::AlignHCenter);
    gridLayout->addWidget(lb2, 2, 0, 1, 1);

    QLabel* const lb3 = new QLabel(this);
    lb3->setText(i18n("Source"));
    lb3->setAlignment(Qt::AlignHCenter);
    gridLayout->addWidget(lb3, 2, 2, 1, 1);

    QHBoxLayout* const layout2 = new QHBoxLayout();
    pLayout->addLayout(layout2);

    QFrame* const hline = new QFrame(this);
    hline->setLineWidth(1);
    hline->setMidLineWidth(0);
    hline->setFrameShape(QFrame::HLine);
    hline->setFrameShadow(QFrame::Sunken);
    hline->setMinimumSize(0, 2);
    hline->updateGeometry();
    pLayout->addWidget(hline);

    QHBoxLayout* const layout = new QHBoxLayout();
    pLayout->addLayout(layout);

    layout->addStretch(1);
    layout->addWidget(buttonBox);

    d->progressTimer = new QTimer(this);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));

    d->progressTimer->start(300);

    // get source thumbnail
    if (d->src.isValid())
    {
        connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnail(LoadingDescription,QPixmap)));

        d->thumbLoadThread->find(ThumbnailIdentifier(d->src.toLocalFile()));
    }

    // get dest thumbnail
    d->buffer.resize(0);

    if (d->dest.isValid())
    {
        d->netMngr = new QNetworkAccessManager(this);

        connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(slotFinished(QNetworkReply*)));

        QNetworkRequest netRequest(d->dest);
        netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                             QLatin1String("application/x-www-form-urlencoded"));

        d->netMngr->get(netRequest);
    }

    resize(sizeHint());
}

void ReplaceDialog::slotFinished(QNetworkReply* reply)
{
    d->progressTimer->stop();

    if (reply->error() != QNetworkReply::NoError)
    {
        reply->deleteLater();
        return;
    }

    d->buffer.append(reply->readAll());

    if (!d->buffer.isEmpty())
    {
        QPixmap pxm;
        pxm.loadFromData(d->buffer);
        d->lbDest->setPixmap(pxm.scaled(200, 200, Qt::KeepAspectRatio, Qt::FastTransformation));
    }

    reply->deleteLater();
}

void ReplaceDialog::slotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (QUrl::fromLocalFile(desc.filePath) == d->src)
    {
        d->lbSrc->setPixmap(pix.scaled(200, 200, Qt::KeepAspectRatio, Qt::FastTransformation));
    }
}

ReplaceDialog::~ReplaceDialog()
{
    delete d;
}

void ReplaceDialog::cancelPressed()
{
    close();
    d->result = PWR_CANCEL;
}

void ReplaceDialog::addPressed()
{
    close();
    d->result = PWR_ADD;
}

void ReplaceDialog::addAllPressed()
{
    close();
    d->result = PWR_ADD_ALL;
}

void ReplaceDialog::replacePressed()
{
    close();
    d->result = PWR_REPLACE;
}

void ReplaceDialog::replaceAllPressed()
{
    close();
    d->result = PWR_REPLACE_ALL;
}

QPixmap ReplaceDialog::setProgressAnimation(const QPixmap& thumb, const QPixmap& pix)
{
    QPixmap overlay = thumb;
    QPixmap mask(overlay.size());
    mask.fill(QColor(128, 128, 128, 192));
    QPainter p(&overlay);
    p.drawPixmap(0, 0, mask);
    p.drawPixmap((overlay.width()/2) - (pix.width()/2), (overlay.height()/2) - (pix.height()/2), pix);
    return overlay;
}

void ReplaceDialog::slotProgressTimerDone()
{
    d->lbDest->setPixmap(setProgressAnimation(d->mimePix, d->progressPix.frameAt(d->progressCount)));
    d->progressCount++;

    if (d->progressCount == 8)
        d->progressCount = 0;

    d->progressTimer->start(300);
}

int ReplaceDialog::getResult()
{
    return d->result;
}

} // namespace Digikam
