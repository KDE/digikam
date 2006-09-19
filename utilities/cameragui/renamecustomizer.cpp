/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2004-09-19
 * Description : a options group to set renaming files
 *               operations during camera downloading
 *
 * Copyright 2004-2005 by Renchi Raju
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

#include <qdatetime.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kdialogbase.h>

// Local includes.

#include "renamecustomizer.h"

namespace Digikam
{

class RenameCustomizerPriv
{
public:

    RenameCustomizerPriv()
    {
        renameDefault         = 0;
        renameCustom          = 0;
        renameDefaultBox      = 0;
        renameCustomBox       = 0;
        renameDefaultCase     = 0;
        renameDefaultCaseType = 0;
        addDateTimeBox        = 0;
        addCameraNameBox      = 0;
        addSeqNumberBox       = 0;
        changedTimer          = 0;
        renameCustomPrefix    = 0;
        renameCustomSuffix   = 0;
        startIndexLabel       = 0;
        startIndexInput       = 0;
        focusedWidget         = 0;
    }

    QWidget      *focusedWidget;

    QString       cameraTitle;

    QRadioButton *renameDefault;
    QRadioButton *renameCustom;

    QGroupBox    *renameDefaultBox;
    QGroupBox    *renameCustomBox;
    
    QLabel       *renameDefaultCase;
    QLabel       *startIndexLabel;

    QComboBox    *renameDefaultCaseType;

    QCheckBox    *addDateTimeBox;
    QCheckBox    *addCameraNameBox;
    QCheckBox    *addSeqNumberBox;

    QTimer       *changedTimer;

    KLineEdit    *renameCustomPrefix;
    KLineEdit    *renameCustomSuffix;

    KIntNumInput *startIndexInput;
};

RenameCustomizer::RenameCustomizer(QWidget* parent, const QString& cameraTitle)
                : QButtonGroup(parent)
{
    d = new RenameCustomizerPriv;
    d->changedTimer = new QTimer();
    d->cameraTitle  = cameraTitle;

    setTitle(i18n("Renaming Options"));
    setRadioButtonExclusive(true);
    setColumnLayout(0, Qt::Vertical);
    QGridLayout* mainLayout = new QGridLayout(layout(), 3, 1);

    // ----------------------------------------------------------------

    d->renameDefault = new QRadioButton(i18n("Camera filenames"), this);
    QWhatsThis::add( d->renameDefault, i18n("<p>Toogle on this option to use camera "
                                            "provided image filenames without modifications."));
    mainLayout->addMultiCellWidget(d->renameDefault, 0, 0, 0, 1);

    d->renameDefaultBox = new QGroupBox( this );
    d->renameDefaultBox->setFrameStyle(QFrame::NoFrame|QFrame::Plain);
    d->renameDefaultBox->setInsideMargin(0);
    d->renameDefaultBox->setColumnLayout(0, Qt::Vertical);

    d->renameDefaultCase = new QLabel( i18n("Change case to:"), d->renameDefaultBox );
    d->renameDefaultCase->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );

    d->renameDefaultCaseType = new QComboBox( d->renameDefaultBox );
    d->renameDefaultCaseType->insertItem(i18n("Leave as Is"), 0);
    d->renameDefaultCaseType->insertItem(i18n("Upper"), 1);
    d->renameDefaultCaseType->insertItem(i18n("Lower"), 2);
    d->renameDefaultCaseType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    QWhatsThis::add( d->renameDefaultCaseType, i18n("<p>Set here the method to use to change case "
                                                    "of image filenames."));
                                           
    QHBoxLayout* boxLayout = new QHBoxLayout( d->renameDefaultBox->layout() );
    boxLayout->addSpacing( 10 );
    boxLayout->addWidget( d->renameDefaultCase );
    boxLayout->addWidget( d->renameDefaultCaseType );

    mainLayout->addMultiCellWidget(d->renameDefaultBox, 1, 1, 0, 1);

    // -------------------------------------------------------------

    d->renameCustom = new QRadioButton(i18n("Customize"), this);
    mainLayout->addMultiCellWidget(d->renameCustom, 2, 2, 0, 1);
    QWhatsThis::add( d->renameCustom, i18n("<p>Toogle on this option to customize image filenames "
                                           "during download."));

    d->renameCustomBox = new QGroupBox(this);
    d->renameCustomBox->setFrameStyle(QFrame::NoFrame|QFrame::Plain);
    d->renameCustomBox->setInsideMargin(0);
    d->renameCustomBox->setColumnLayout(0, Qt::Vertical);

    QGridLayout* renameCustomBoxLayout = new QGridLayout(d->renameCustomBox->layout(), 
                                                         5, 2, KDialogBase::spacingHint());
    renameCustomBoxLayout->setColSpacing( 0, 10 );

    QLabel* prefixLabel = new QLabel(i18n("Prefix:"), d->renameCustomBox);
    renameCustomBoxLayout->addMultiCellWidget(prefixLabel, 0, 0, 1, 1);
    d->renameCustomPrefix = new KLineEdit(d->renameCustomBox);
    d->focusedWidget = d->renameCustomPrefix;
    renameCustomBoxLayout->addMultiCellWidget(d->renameCustomPrefix, 0, 0, 2, 2);
    QWhatsThis::add( d->renameCustomPrefix, i18n("<p>Set the prefix which will be prepended to "
                                                 "image filenames."));

    QLabel* suffixLabel = new QLabel(i18n("Suffix:"), d->renameCustomBox);
    renameCustomBoxLayout->addMultiCellWidget(suffixLabel, 1, 1, 1, 1);
    d->renameCustomSuffix = new KLineEdit(d->renameCustomBox);
    renameCustomBoxLayout->addMultiCellWidget(d->renameCustomSuffix, 1, 1, 2, 2);
    QWhatsThis::add( d->renameCustomSuffix, i18n("<p>Set the suffix which will be added to "
                                                  "image filenames."));

    d->addDateTimeBox = new QCheckBox( i18n("Add Date && Time"), d->renameCustomBox );
    renameCustomBoxLayout->addMultiCellWidget(d->addDateTimeBox, 2, 2, 1, 2);
    QWhatsThis::add( d->addDateTimeBox, i18n("<p>Set this option to add the camera provided date and time."));

    d->addCameraNameBox = new QCheckBox( i18n("Add Camera Name"), d->renameCustomBox );
    renameCustomBoxLayout->addMultiCellWidget(d->addCameraNameBox, 3, 3, 1, 2);
    QWhatsThis::add( d->addCameraNameBox, i18n("<p>Set this option to add the camera name."));

    d->addSeqNumberBox = new QCheckBox( i18n("Add Sequence Number"), d->renameCustomBox );
    renameCustomBoxLayout->addMultiCellWidget(d->addSeqNumberBox, 4, 4, 1, 2);
    QWhatsThis::add( d->addSeqNumberBox, i18n("<p>Set this option to add a sequence number starting with the index set below."));

    d->startIndexLabel = new QLabel( i18n("Start Index:"), d->renameCustomBox );
    d->startIndexInput = new KIntNumInput(1, d->renameCustomBox);
    d->startIndexInput->setRange(1, 900000, 1, false);
    QWhatsThis::add( d->startIndexInput, i18n("<p>Set here the start index value used to rename picture "
                                              "files with a sequence number."));

    renameCustomBoxLayout->addMultiCellWidget(d->startIndexLabel, 5, 5, 1, 1);
    renameCustomBoxLayout->addMultiCellWidget(d->startIndexInput, 5, 5, 2, 2);

    mainLayout->addMultiCellWidget(d->renameCustomBox, 3, 3, 0, 1);

    // -- setup connections -------------------------------------------------

    connect(this, SIGNAL(clicked(int)),
            this, SLOT(slotRadioButtonClicked(int)));
            
    connect(d->renameCustomPrefix, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->renameCustomSuffix, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->addDateTimeBox, SIGNAL(toggled(bool)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->addCameraNameBox, SIGNAL(toggled(bool)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->addSeqNumberBox, SIGNAL(toggled(bool)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->renameDefaultCaseType, SIGNAL(activated(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->startIndexInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->changedTimer, SIGNAL(timeout()),
            this, SIGNAL(signalChanged()));

    // -- initial values ---------------------------------------------------

    readSettings();
}

RenameCustomizer::~RenameCustomizer()
{
    delete d->changedTimer;
    saveSettings();
    delete d;
}

bool RenameCustomizer::useDefault() const
{
    return d->renameDefault->isChecked();
}

int RenameCustomizer::startIndex() const
{
    return d->startIndexInput->value();
}

QString RenameCustomizer::newName(const QDateTime &dateTime, int index, const QString &extension) const
{
    if (d->renameDefault->isChecked())
        return QString();
    else
    {
        QString name(d->renameCustomPrefix->text());

        // use the "T" as a delimiter between date and time
        QString date = dateTime.toString("yyyyMMddThhmmss");

        // it seems that QString::number does not support padding with zeros
        QString seq;
        seq.sprintf("-%06d", index);

        if (d->addDateTimeBox->isChecked())
            name += date;

        if (d->addSeqNumberBox->isChecked())
            name += seq;

        if (d->addCameraNameBox->isChecked())
            name += QString("-%1").arg(d->cameraTitle.simplifyWhiteSpace().replace(" ", ""));

        name += d->renameCustomSuffix->text();
        name += extension;

        return name;
    }
}

RenameCustomizer::Case RenameCustomizer::changeCase() const
{
    RenameCustomizer::Case type = NONE;

    if (d->renameDefaultCaseType->currentItem() == 1)
        type=UPPER;
    if (d->renameDefaultCaseType->currentItem() == 2)
        type=LOWER;

    return type;
}

void RenameCustomizer::slotRadioButtonClicked(int)
{
    QRadioButton* btn = dynamic_cast<QRadioButton*>(selected());
    if (!btn)
        return;

    d->renameCustomBox->setEnabled( btn != d->renameDefault );
    d->renameDefaultBox->setEnabled( btn == d->renameDefault );
    slotRenameOptionsChanged();
}

void RenameCustomizer::slotRenameOptionsChanged()
{
    d->focusedWidget = focusWidget();

    if (d->addSeqNumberBox->isChecked())
    {
        d->startIndexInput->setEnabled(true);
        d->startIndexLabel->setEnabled(true);
    }
    else
    {
        d->startIndexInput->setEnabled(false);
        d->startIndexLabel->setEnabled(false);
    }

    d->changedTimer->start(500, true);
}

void RenameCustomizer::readSettings()
{
    KConfig* config = kapp->config();
    
    config->setGroup("Camera Settings");
    bool def         = config->readBoolEntry("Rename Use Default", true);
    bool addSeqNumb  = config->readBoolEntry("Add Sequence Number", true);
    bool adddateTime = config->readBoolEntry("Add Date Time", false);
    bool addCamName  = config->readBoolEntry("Add Camera Name", false);
    int chcaseT      = config->readNumEntry("Case Type", NONE);
    QString prefix   = config->readEntry("Rename Prefix", i18n("photo"));
    QString suffix   = config->readEntry("Rename Postfix", QString());
    int startIndex   = config->readNumEntry("Rename Start Index", 1);

    if (def)
    {
        d->renameDefault->setChecked(true);
        d->renameCustom->setChecked(false);
        d->renameCustomBox->setEnabled(false);
        d->renameDefaultBox->setEnabled(true);
    }
    else
    {
        d->renameDefault->setChecked(false);
        d->renameCustom->setChecked(true);
        d->renameCustomBox->setEnabled(true);
        d->renameDefaultBox->setEnabled(false);
    }

    d->addDateTimeBox->setChecked(adddateTime);
    d->addCameraNameBox->setChecked(addCamName);
    d->addSeqNumberBox->setChecked(addSeqNumb);
    d->renameDefaultCaseType->setCurrentItem(chcaseT);
    d->renameCustomPrefix->setText(prefix);
    d->renameCustomSuffix->setText(suffix);
    d->startIndexInput->setValue(startIndex);
    slotRenameOptionsChanged();
}

void RenameCustomizer::saveSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("Camera Settings");
    config->writeEntry("Rename Use Default", d->renameDefault->isChecked());
    config->writeEntry("Add Camera Name", d->addCameraNameBox->isChecked());
    config->writeEntry("Add Date Time", d->addDateTimeBox->isChecked());
    config->writeEntry("Add Sequence Number", d->addSeqNumberBox->isChecked());
    config->writeEntry("Case Type", d->renameDefaultCaseType->currentItem());
    config->writeEntry("Rename Prefix", d->renameCustomPrefix->text());
    config->writeEntry("Rename Suffix", d->renameCustomSuffix->text());
    config->writeEntry("Rename Start Index", d->startIndexInput->value());
    config->sync();
}

void RenameCustomizer::restoreFocus()
{
    d->focusedWidget->setFocus();
}

}  // namespace Digikam

#include "renamecustomizer.moc"
