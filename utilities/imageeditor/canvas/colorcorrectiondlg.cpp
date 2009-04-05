/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-05-15
 * Description : a dialog to see preview ICC color correction
 *               before to apply color profile.
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "colorcorrectiondlg.h"
#include "colorcorrectiondlg.moc"

// Qt includes

#include <QLabel>
#include <QFrame>
#include <QString>
#include <QFileInfo>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"
#include "icctransform.h"
#include "iccprofileinfodlg.h"

namespace Digikam
{

ColorCorrectionDlg::ColorCorrectionDlg(QWidget* parent, DImg *preview,
                                       IccTransform *iccTrans, const QString& file)
                  : KDialog(parent)
{
    m_iccTrans = iccTrans;
    m_parent   = parent;

    setButtons(Help|Ok|Apply|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("iccprofile.anchor", "digikam");
    setButtonText(Ok,        i18n("Convert"));
    setButtonToolTip(Ok,     i18n("Apply the default color workspace profile to the image."));
    setButtonText(Cancel,    i18n("Do Nothing"));
    setButtonToolTip(Cancel, i18n("Do not change the image"));
    setButtonText(Apply,     i18n("Assign"));
    setButtonToolTip(Apply,  i18n("Only embed the color workspace profile in the image, "
                                  "do not change the image."));

    QFileInfo fi(file);
    setCaption(fi.fileName());

    QWidget *page     = new QWidget(this);
    QGridLayout* grid = new QGridLayout(page);
    setMainWidget(page);

    QLabel *originalTitle         = new QLabel(i18n("Original Image:"), page);
    QLabel *previewOriginal       = new QLabel(page);
    QLabel *targetTitle           = new QLabel(i18n("Corrected Image:"), page);
    QLabel *previewTarget         = new QLabel(page);
    QLabel *logo                  = new QLabel(page);
    QLabel *message               = new QLabel(page);
    QLabel *currentProfileTitle   = new QLabel(i18n("Current workspace color profile:"), page);
    QLabel *currentProfileDesc    = new QLabel(QString("<b>%1</b>")
                                               .arg(m_iccTrans->getOutpoutProfileDescriptor()), page);
    QPushButton *currentProfInfo  = new QPushButton(i18n("Info..."), page);
    QLabel *embeddedProfileTitle  = new QLabel(i18n("Embedded color profile:"), page);
    QLabel *embeddedProfileDesc   = new QLabel(QString("<b>%1</b>")
                                               .arg(m_iccTrans->getEmbeddedProfileDescriptor()), page);
    QPushButton *embeddedProfInfo = new QPushButton(i18n("Info..."), page);
    KSeparator *line              = new KSeparator(Qt::Horizontal, page);

    if (m_iccTrans->embeddedProfile().isEmpty())
    {
        message->setText(i18n("<p>This image has not been assigned a color profile.</p>"
                              "<p>Do you want to convert it to your workspace color profile?</p>"));

        line->hide();
        embeddedProfileTitle->hide();
        embeddedProfileDesc->hide();
        embeddedProfInfo->hide();
    }
    else
    {
        message->setText(i18n("<p>This image has been assigned to a color profile that does not "
                              "match your default workspace color profile.</p>"
                              "<p>Do you want to convert it to your workspace color profile?</p>"));
    }

    originalTitle->setWordWrap(true);
    targetTitle->setWordWrap(true);
    message->setWordWrap(true);
    currentProfileTitle->setWordWrap(true);
    currentProfileDesc->setWordWrap(true);
    embeddedProfileTitle->setWordWrap(true);
    embeddedProfileDesc->setWordWrap(true);

    previewOriginal->setPixmap(preview->convertToPixmap());
    previewTarget->setPixmap(preview->convertToPixmap(m_iccTrans));
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                            .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QHBoxLayout *hlay1 = new QHBoxLayout();
    hlay1->setSpacing( KDialog::spacingHint() );
    hlay1->addWidget(currentProfInfo);
    hlay1->addStretch();

    QHBoxLayout *hlay2 = new QHBoxLayout();
    hlay2->setSpacing( KDialog::spacingHint() );
    hlay2->addWidget(embeddedProfInfo);
    hlay2->addStretch();

    QVBoxLayout *vlay = new QVBoxLayout();
    vlay->setSpacing( KDialog::spacingHint() );
    vlay->addWidget(logo);
    vlay->addWidget(message);
    vlay->addWidget(new KSeparator (Qt::Horizontal, page));
    vlay->addWidget(currentProfileTitle);
    vlay->addWidget(currentProfileDesc);
    vlay->addLayout(hlay1);
    vlay->addWidget(line);
    vlay->addWidget(embeddedProfileTitle);
    vlay->addWidget(embeddedProfileDesc);
    vlay->addLayout(hlay2);
    vlay->addStretch();

    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());
    grid->addWidget(originalTitle,      0, 0, 1, 1);
    grid->addWidget(previewOriginal,    1, 0, 1, 1);
    grid->addWidget(targetTitle,        2, 0, 1, 1);
    grid->addWidget(previewTarget,      3, 0, 1, 1);
    grid->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                      QSizePolicy::Minimum, QSizePolicy::Expanding), 0, 1, 3, 1);
    grid->addLayout(vlay, 0, 2, 4, 1);

    // --------------------------------------------------------------------

    connect(currentProfInfo, SIGNAL(clicked()),
            this, SLOT(slotCurrentProfInfo()) );

    connect(embeddedProfInfo, SIGNAL(clicked()),
            this, SLOT(slotEmbeddedProfInfo()) );

    connect(this, SIGNAL(applyClicked()),
            this, SLOT(slotApplyClicked()));
}

ColorCorrectionDlg::~ColorCorrectionDlg()
{
}

void ColorCorrectionDlg::slotCurrentProfInfo()
{
    if (m_iccTrans->outputProfile().isEmpty())
        return;

    ICCProfileInfoDlg infoDlg(m_parent, QString(), m_iccTrans->outputProfile());
    infoDlg.exec();
}

void ColorCorrectionDlg::slotEmbeddedProfInfo()
{
    if (m_iccTrans->embeddedProfile().isEmpty())
        return;

    ICCProfileInfoDlg infoDlg(m_parent, QString(), m_iccTrans->embeddedProfile());
    infoDlg.exec();
}

void ColorCorrectionDlg::slotApplyClicked()
{
    done(-1);
}

}  // namespace Digikam
