/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-19
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

#include <qlayout.h>
#include <qradiobutton.h>
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
        renameCustomOptions   = 0;
        changedTimer          = 0;
        renameCustomPrefix    = 0;
    }

    QRadioButton *renameDefault;
    QRadioButton *renameCustom;

    QGroupBox    *renameDefaultBox;
    QGroupBox    *renameCustomBox;
    
    QLabel       *renameDefaultCase;

    QComboBox    *renameDefaultCaseType;

    QComboBox    *renameCustomOptions;

    QTimer       *changedTimer;

    KLineEdit    *renameCustomPrefix;
};

RenameCustomizer::RenameCustomizer(QWidget* parent)
                : QButtonGroup(parent)
{
    d = new RenameCustomizerPriv;
    d->changedTimer = new QTimer();
    
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
                                         1, 2, KDialogBase::spacingHint());
    renameCustomBoxLayout->setColSpacing( 0, 10 );
    QLabel* prefixLabel = new QLabel(i18n("Prefix:"), d->renameCustomBox);
    renameCustomBoxLayout->addMultiCellWidget(prefixLabel, 0, 0, 1, 1);

    d->renameCustomPrefix = new KLineEdit(d->renameCustomBox);
    renameCustomBoxLayout->addMultiCellWidget(d->renameCustomPrefix, 0, 0, 2, 2);
    QWhatsThis::add( d->renameCustomPrefix, i18n("<p>Set here the string to use like a prefix of "
                                                 "image filenames."));
                                                 
    QLabel *renameOptionsLabel = new QLabel( i18n("Add:"), d->renameCustomBox );
    renameOptionsLabel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
    renameCustomBoxLayout->addMultiCellWidget(renameOptionsLabel, 1, 1, 1, 1);

    d->renameCustomOptions = new QComboBox( d->renameCustomBox );
    d->renameCustomOptions->insertItem(i18n("Date and time"), 0);
    d->renameCustomOptions->insertItem(i18n("Sequence number"), 1);
    d->renameCustomOptions->insertItem(i18n("Time stamp & number"), 2);
    renameCustomBoxLayout->addMultiCellWidget(d->renameCustomOptions, 1, 1, 2, 2);
    QWhatsThis::add( d->renameCustomOptions, i18n("<p>Set here the information to add to filename:<p>"
                     "<b>Date and time</b>: add the camera provided date and time.<p>"
                     "<b>Sequence number</b>: add a sequence number.<p>"
                     "<b>Time stamp & number</b>: add both camera provided date and time and "
                     "a sequence number."));

    mainLayout->addMultiCellWidget(d->renameCustomBox, 3, 3, 0, 1);

    // -- setup connections -------------------------------------------------

    connect(this, SIGNAL(clicked(int)),
            this, SLOT(slotRadioButtonClicked(int)));
            
    connect(d->renameCustomPrefix, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotPrefixChanged(const QString&)));
            
    connect(d->renameCustomOptions, SIGNAL(activated(const QString&)),
            this, SLOT(slotCustomOptionsChanged(const QString&)));
            
    connect(d->renameDefaultCaseType, SIGNAL(activated(const QString&)),
            this, SLOT(slotCaseTypeChanged(const QString&)));

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

QString RenameCustomizer::nameTemplate() const
{
    if (d->renameDefault->isChecked())
        return QString();
    else
    {
        QString templ(d->renameCustomPrefix->text());

        switch (d->renameCustomOptions->currentItem())
        {
        case ADDDATETIME:
            templ += "%Y%m%dT%H%M%S"; 
            break;
        case ADDSEQNUMB:
            templ += "-%%04d";
            break;
        case ADDBOTH:
            templ += "%Y%m%dT%H%M%S"; 
            templ += "-%%04d";
            break;
        }

        return templ;
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
    d->changedTimer->start(500, true);
}

void RenameCustomizer::slotPrefixChanged(const QString&)
{
    d->changedTimer->start(500, true);
}

void RenameCustomizer::slotCustomOptionsChanged(const QString&)
{
    d->changedTimer->start(500, true);
}

void RenameCustomizer::slotCaseTypeChanged(const QString&)
{
    d->changedTimer->start(500, true);
}

void RenameCustomizer::readSettings()
{
    KConfig* config = kapp->config();
    
    config->setGroup("Camera Settings");
    bool def       = config->readBoolEntry("Rename Use Default", true);
    int option     = config->readNumEntry("Rename Add Option", ADDSEQNUMB);
    int chcaseT    = config->readNumEntry("Case Type", NONE);
    QString prefix = config->readEntry("Rename Prefix", i18n("photo"));

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

    d->renameDefaultCaseType->setCurrentItem(chcaseT);
    d->renameCustomPrefix->setText(prefix);
    d->renameCustomOptions->setCurrentItem(option);
}

void RenameCustomizer::saveSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("Camera Settings");
    config->writeEntry("Rename Use Default", d->renameDefault->isChecked());
    config->writeEntry("Rename Add Option", d->renameCustomOptions->currentItem());
    config->writeEntry("Case Type", d->renameDefaultCaseType->currentItem());
    config->writeEntry("Rename Prefix", d->renameCustomPrefix->text());
    config->sync();
}

}  // namespace Digikam

#include "renamecustomizer.moc"
