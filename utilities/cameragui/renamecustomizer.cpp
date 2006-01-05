/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-19
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qlayout.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qtimer.h>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>

// Local includes.

#include "renamecustomizer.h"

namespace Digikam
{

RenameCustomizer::RenameCustomizer(QWidget* parent)
    : QButtonGroup(parent)
{
    // -- setup view --------------------------------------------------
    
    setTitle(i18n("Renaming Options"));
    setRadioButtonExclusive(true);
    
    setColumnLayout(0, Qt::Vertical);
    layout()->setSpacing(5);
    layout()->setMargin(10);

    QGridLayout* mainLayout = new QGridLayout(layout());

    m_renameDefault = new QRadioButton(i18n("Use camera provided names"), this);
    mainLayout->addMultiCellWidget(m_renameDefault, 0, 0, 0, 1);

    m_renameDefaultBox = new QGroupBox( this );
    m_renameDefaultBox->setFrameStyle(QFrame::NoFrame|QFrame::Plain);
    m_renameDefaultBox->setColumnLayout(0, Qt::Vertical);

    m_renameDefaultCase = new QLabel( i18n("Change case to"), m_renameDefaultBox );
    m_renameDefaultCase->setIndent( 10 );
    m_renameDefaultCase->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );

    m_renameDefaultCaseType = new QComboBox( m_renameDefaultBox );
    m_renameDefaultCaseType->insertItem(i18n("Leave as Is"), 0);
    m_renameDefaultCaseType->insertItem(i18n("Upper"), 1);
    m_renameDefaultCaseType->insertItem(i18n("Lower"), 2);
    m_renameDefaultCaseType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QHBoxLayout* boxLayout =  new QHBoxLayout( m_renameDefaultBox->layout() );
    boxLayout->setSpacing(5);
    boxLayout->addWidget( m_renameDefaultCase );
    boxLayout->addWidget( m_renameDefaultCaseType );
    boxLayout->addItem(new QSpacerItem(20, 10, QSizePolicy::Expanding,
                                       QSizePolicy::Minimum ));

    mainLayout->addMultiCellWidget(m_renameDefaultBox, 1, 1, 0, 1);

    
    m_renameCustom = new QRadioButton(i18n("Customize names"), this);
    mainLayout->addMultiCellWidget(m_renameCustom, 2, 2, 0, 1);

    m_renameCustomBox = new QGroupBox(this);
    m_renameCustomBox->setFrameStyle(QFrame::NoFrame|QFrame::Plain);
    m_renameCustomBox->setColumnLayout(0, Qt::Vertical);
    QGridLayout* renameCustomBoxLayout = new QGridLayout(m_renameCustomBox->layout());
    renameCustomBoxLayout->setMargin(0);
    renameCustomBoxLayout->setSpacing(5);

    QLabel* prefixLabel  = new QLabel(i18n("Prefix:"), m_renameCustomBox);
    renameCustomBoxLayout->addWidget(prefixLabel, 0, 0);

    m_renameCustomPrefix = new QLineEdit(m_renameCustomBox);
    renameCustomBoxLayout->addWidget(m_renameCustomPrefix, 0, 1);

    m_renameCustomExif = new QCheckBox(i18n("Add camera provided date and time"),
                                       m_renameCustomBox);
    renameCustomBoxLayout->addMultiCellWidget(m_renameCustomExif, 1, 1, 0, 1);

    m_renameCustomSeq  = new QCheckBox(i18n("Add sequence number"),
                                       m_renameCustomBox);
    renameCustomBoxLayout->addMultiCellWidget(m_renameCustomSeq, 2, 2, 0, 1);

    mainLayout->addItem(new QSpacerItem(20, 10, QSizePolicy::Minimum,
                                        QSizePolicy::Minimum ), 3, 0);
    mainLayout->addWidget(m_renameCustomBox, 3, 1);

    // -- setup connections -------------------------------------------------

    connect(this, SIGNAL(clicked(int)),
            SLOT(slotRadioButtonClicked(int)));
    connect(m_renameCustomPrefix, SIGNAL(textChanged(const QString&)),
            SLOT(slotPrefixChanged(const QString&)));
    connect(m_renameCustomExif, SIGNAL(toggled(bool)),
            SLOT(slotExifChanged(bool)));
    connect(m_renameCustomSeq, SIGNAL(toggled(bool)),
            SLOT(slotSeqChanged(bool)));
    connect(m_renameDefaultCaseType, SIGNAL(activated(const QString&)),
            SLOT(slotCaseTypeChanged(const QString&)));

    // -- changed timer ----------------------------------------------------

    m_changedTimer = new QTimer();
    connect(m_changedTimer, SIGNAL(timeout()),
            this, SIGNAL(signalChanged()));

    // -- initial values ---------------------------------------------------

    readSettings();
}

RenameCustomizer::~RenameCustomizer()
{
    delete m_changedTimer;
    saveSettings();
}

bool RenameCustomizer::useDefault() const
{
    return m_renameDefault->isChecked();    
}

QString RenameCustomizer::nameTemplate() const
{
    if (m_renameDefault->isChecked())
        return QString();
    else
    {
        QString templ(m_renameCustomPrefix->text());

        if (m_renameCustomExif->isChecked())
            templ += "%Y%m%d-%H:%M:%S";

        if (m_renameCustomSeq->isChecked())
            templ += "-%%04d";

        return templ;
    }
}

RenameCustomizer::Case RenameCustomizer::changeCase() const
{
    RenameCustomizer::Case type = NONE;

    if (m_renameDefaultCaseType->currentItem() == 1)
        type=UPPER;
    if (m_renameDefaultCaseType->currentItem() == 2)
        type=LOWER;

    return type;
}


void RenameCustomizer::slotRadioButtonClicked(int)
{
    QRadioButton* btn = dynamic_cast<QRadioButton*>(selected());
    if (!btn)
        return;

    m_renameCustomBox->setEnabled( btn != m_renameDefault );
    m_renameDefaultBox->setEnabled( btn == m_renameDefault );
    m_changedTimer->start(500, true);
}

void RenameCustomizer::slotPrefixChanged(const QString&)
{
    m_changedTimer->start(500, true);
}

void RenameCustomizer::slotExifChanged(bool)
{
    m_changedTimer->start(500, true);
}

void RenameCustomizer::slotSeqChanged(bool)
{
    m_changedTimer->start(500, true);
}

void RenameCustomizer::slotCaseTypeChanged(const QString&)
{
    m_changedTimer->start(500, true);
}

void RenameCustomizer::readSettings()
{
    KConfig* config = kapp->config();

    bool    def;
    bool    exif;
    bool    seq;
    int     chcaseT;
    QString prefix;
    
    config->setGroup("Camera Settings");
    def     = config->readBoolEntry("Rename Use Default", true);
    exif    = config->readBoolEntry("Rename Add Exif", true);
    seq     = config->readBoolEntry("Rename Add Sequence", true);
    chcaseT = config->readNumEntry("Case Type", NONE);
    prefix  = config->readEntry("Rename Prefix", i18n("photo"));

    if (def)
    {
        m_renameDefault->setChecked(true);
        m_renameCustom->setChecked(false);
        m_renameCustomBox->setEnabled(false);
        m_renameDefaultBox->setEnabled(true);
    }
    else
    {
        m_renameDefault->setChecked(false);
        m_renameCustom->setChecked(true);
        m_renameCustomBox->setEnabled(true);
        m_renameDefaultBox->setEnabled(false);
    }

    m_renameDefaultCaseType->setCurrentItem(chcaseT);
    m_renameCustomPrefix->setText(prefix);
    m_renameCustomExif->setChecked(exif);
    m_renameCustomSeq->setChecked(seq);
}

void RenameCustomizer::saveSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("Camera Settings");
    config->writeEntry("Rename Use Default",
                       m_renameDefault->isChecked());
    config->writeEntry("Rename Add Exif",
                       m_renameCustomExif->isChecked());
    config->writeEntry("Rename Add Sequence",
                       m_renameCustomSeq->isChecked());
    config->writeEntry("Case Type",
                       m_renameDefaultCaseType->currentItem());
    config->writeEntry("Rename Prefix",
                       m_renameCustomPrefix->text());
    config->sync();
}

}  // namespace Digikam

#include "renamecustomizer.moc"
