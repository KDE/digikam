/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-05-15
 * Description : a dialog to see preview ICC color correction 
 *               before to apply color profile.
 * 
 * Copyright 2006 by Gilles Caulier
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

// Qt includes.

#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qpushbutton.h>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kseparator.h>

// Local includes.

#include "dimg.h"
#include "icctransform.h"
#include "iccprofileinfodlg.h"
#include "colorcorrectiondlg.h"

namespace Digikam
{

ColorCorrectionDlg::ColorCorrectionDlg(QWidget* parent, DImg *preview, 
                                       IccTransform *iccTrans, const QString& file)
                  : KDialogBase(parent, "", true, QString::null,
                                Help|Ok|Apply|Cancel, Ok, true)

{
    m_iccTrans = iccTrans;
    m_parent   = parent;
    setHelp("iccprofile.anchor", "digikam");
    setButtonText(Ok,     i18n("Apply"));
    setButtonTip(Ok,      i18n("Apply the default color workspace profile to the image"));
    setButtonText(Cancel, i18n("Do Nothing"));
    setButtonTip(Cancel,  i18n("Do not change the image"));
    setButtonText(Apply,  i18n("Embed only"));
    setButtonTip(Apply,   i18n("Embed only the color workspace profile to the image without changing the image"));

    QFileInfo fi(file);
    setCaption(fi.fileName());
    
    QWidget *page     = new QWidget(this);
    QGridLayout* grid = new QGridLayout(page, 3, 2, 0, KDialog::spacingHint());
        
    QLabel *originalTitle         = new QLabel(i18n("Original Picture:"), page);
    QLabel *previewOriginal       = new QLabel(page);
    QLabel *targetTitle           = new QLabel(i18n("Corrected Picture:"), page);
    QLabel *previewTarget         = new QLabel(page);
    QLabel *logo                  = new QLabel(page);
    QLabel *message               = new QLabel(page);
    QLabel *currentProfileTitle   = new QLabel(i18n("Current workspace color profile:"), page);
    QLabel *currentProfileDesc    = new QLabel(QString("<b>%1</b>").arg(m_iccTrans->getOutpoutProfileDescriptor()), page);
    QPushButton *currentProfInfo  = new QPushButton(i18n("Info..."), page);
    QLabel *embeddedProfileTitle  = new QLabel(i18n("Embedded color profile:"), page);
    QLabel *embeddedProfileDesc   = new QLabel(QString("<b>%1</b>").arg(m_iccTrans->getEmbeddedProfileDescriptor()), page);
    QPushButton *embeddedProfInfo = new QPushButton(i18n("Info..."), page);
    KSeparator *line              = new KSeparator (Horizontal, page);
    
    if (m_iccTrans->embeddedProfile().isEmpty())
    {
        message->setText(i18n("<p>This picture has not assigned any color profile.</p>"
                              "<p>Do you want to convert it to your workspace color profile?</p>"));
                              
        line->hide();
        embeddedProfileTitle->hide();
        embeddedProfileDesc->hide();
        embeddedProfInfo->hide();
    }
    else
    {
        message->setText(i18n("<p>This picture has assigned a color profile that does not "
                              "match with your default workspace color profile.</p>"
                              "<p>Do you want to convert it to your workspace color profile?</p>"));
    }
    
    previewOriginal->setPixmap(preview->convertToPixmap());
    previewTarget->setPixmap(preview->convertToPixmap(m_iccTrans));
    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIcon::NoGroup, 128, KIcon::DefaultState, 0, true));    
    
    grid->addMultiCellWidget(originalTitle, 0, 0, 0, 0);
    grid->addMultiCellWidget(previewOriginal, 1, 1, 0, 0);
    grid->addMultiCellWidget(targetTitle, 2, 2, 0, 0);
    grid->addMultiCellWidget(previewTarget, 3, 3, 0, 0);
    
    QVBoxLayout *vlay = new QVBoxLayout( KDialog::spacingHint() );
    vlay->addWidget(logo);
    vlay->addWidget(message);
    
    vlay->addWidget(new KSeparator (Horizontal, page));
    vlay->addWidget(currentProfileTitle);
    vlay->addWidget(currentProfileDesc);
    
    QHBoxLayout *hlay1 = new QHBoxLayout( KDialog::spacingHint() );
    hlay1->addWidget(currentProfInfo);
    hlay1->addStretch();
    vlay->addLayout(hlay1);
    
    vlay->addWidget(line);
    vlay->addWidget(embeddedProfileTitle);
    vlay->addWidget(embeddedProfileDesc);    
    
    QHBoxLayout *hlay2 = new QHBoxLayout( KDialog::spacingHint() );
    hlay2->addWidget(embeddedProfInfo);
    hlay2->addStretch();
    vlay->addLayout(hlay2);
    vlay->addStretch();
    
    grid->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                       QSizePolicy::Minimum, QSizePolicy::Expanding), 0, 3, 1, 1);
    grid->addMultiCellLayout(vlay, 0, 3, 2, 2);
    
    setMainWidget(page);
    
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

    ICCProfileInfoDlg infoDlg(m_parent, QString::null, m_iccTrans->outputProfile());
    infoDlg.exec();
}

void ColorCorrectionDlg::slotEmbeddedProfInfo()
{
    if (m_iccTrans->embeddedProfile().isEmpty())
        return;

    ICCProfileInfoDlg infoDlg(m_parent, QString::null, m_iccTrans->embeddedProfile());
    infoDlg.exec();
}

void ColorCorrectionDlg::slotApplyClicked()
{
    kdDebug() << "colorcorrectiondlg: Apply pressed" << endl;
    done(-1);
}

}  // NameSpace Digikam

#include "colorcorrectiondlg.moc"
