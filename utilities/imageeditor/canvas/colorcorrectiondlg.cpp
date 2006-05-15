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

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>

// Local includes.

#include "dimg.h"
#include "icctransform.h"
#include "iccprofileinfodlg.h"
#include "colorcorrectiondlg.h"

namespace Digikam
{

ColorCorrectionDlg::ColorCorrectionDlg(QWidget* parent, DImg *preview, 
                                       IccTransform *iccTrans, const QString& msg,
                                       const QString& file)
                  : KDialogBase(parent, 0, true, QString::null,
                                Help|User1|User2|Ok|Cancel, Ok, true)

{
    m_iccTrans = iccTrans;
    m_parent   = parent;
    setHelp("iccprofile.anchor", "digikam");
    setButtonText(Ok,     i18n("Apply"));
    setButtonText(Cancel, i18n("Do Nothing"));
    setButtonText(User1,  i18n("Current Profile Info..."));
    setButtonText(User2,  i18n("Embedded Profile Info..."));

    QFileInfo fi(file);
    setCaption(fi.fileName());
    showButton(User2, !m_iccTrans->embeddedProfile().isEmpty());
    
    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QGridLayout* grid = new QGridLayout(page, 4, 1, 0, KDialog::spacingHint());
        
    QLabel *originalTitle   = new QLabel(i18n("Original Picture:"), page);
    QLabel *previewOriginal = new QLabel(page);
    QLabel *targetTitle     = new QLabel(i18n("Corrected Picture:"), page);
    QLabel *previewTarget   = new QLabel(page);
    QLabel *logo            = new QLabel(page);
    QLabel *message         = new QLabel(msg, page);
    
    previewOriginal->setPixmap(preview->convertToPixmap());
    previewTarget->setPixmap(preview->convertToPixmap(m_iccTrans));
    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIcon::NoGroup, 128, KIcon::DefaultState, 0, true));    
    
    grid->addMultiCellWidget(originalTitle, 0, 0, 0, 0);
    grid->addMultiCellWidget(previewOriginal, 1, 2, 0, 0);
    grid->addMultiCellWidget(targetTitle, 3, 3, 0, 0);
    grid->addMultiCellWidget(previewTarget, 4, 4, 0, 0);
    grid->addMultiCellWidget(logo, 1, 1, 1, 1);
    grid->addMultiCellWidget(message, 2, 4, 1, 1);
}

ColorCorrectionDlg::~ColorCorrectionDlg()
{
}

void ColorCorrectionDlg::slotUser1()
{
    if (m_iccTrans->outputProfile().isEmpty())
        return;

    ICCProfileInfoDlg infoDlg(m_parent, QString::null, m_iccTrans->outputProfile());
    infoDlg.exec();
}

void ColorCorrectionDlg::slotUser2()
{
    if (m_iccTrans->embeddedProfile().isEmpty())
        return;

    ICCProfileInfoDlg infoDlg(m_parent, QString::null, m_iccTrans->embeddedProfile());
    infoDlg.exec();
}

}  // NameSpace Digikam

