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

#include <qlayout.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qtimer.h>

#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>

#include "renamecustomizer.h"

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

    m_renameCustom = new QRadioButton(i18n("Customize names"), this);
    mainLayout->addMultiCellWidget(m_renameCustom, 1, 1, 0, 1);

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
                                        QSizePolicy::Minimum ), 2, 0);
    mainLayout->addWidget(m_renameCustomBox, 2, 1);

    // -- setup connections -------------------------------------------------

    connect(this, SIGNAL(clicked(int)),
            SLOT(slotRadioButtonClicked(int)));
    connect(m_renameCustomPrefix, SIGNAL(textChanged(const QString&)),
            SLOT(slotPrefixChanged(const QString&)));
    connect(m_renameCustomExif, SIGNAL(toggled(bool)),
            SLOT(slotExifChanged(bool)));
    connect(m_renameCustomSeq, SIGNAL(toggled(bool)),
            SLOT(slotSeqChanged(bool)));

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

void RenameCustomizer::slotRadioButtonClicked(int)
{
    QRadioButton* btn = dynamic_cast<QRadioButton*>(selected());
    if (!btn)
        return;

    m_renameCustomBox->setEnabled( btn != m_renameDefault );
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

void RenameCustomizer::readSettings()
{
    KConfig* config = kapp->config();

    bool    def;
    bool    exif;
    bool    seq;
    QString prefix;
    
    config->setGroup("Camera Settings");
    def     = config->readBoolEntry("Rename Use Default", true);
    exif    = config->readBoolEntry("Rename Add Exif", true);
    seq     = config->readBoolEntry("Rename Add Sequence", true);
    prefix  = config->readEntry("Rename Prefix", i18n("photo"));

    if (def)
    {
        m_renameDefault->setChecked(true);
        m_renameCustom->setChecked(false);
        m_renameCustomBox->setEnabled(false);
    }
    else
    {
        m_renameDefault->setChecked(false);
        m_renameCustom->setChecked(true);
        m_renameCustomBox->setEnabled(true);
    }

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
    config->writeEntry("Rename Prefix",
                       m_renameCustomPrefix->text());
    config->sync();
}

#include "renamecustomizer.moc"
