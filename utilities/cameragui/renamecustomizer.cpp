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
#include <qpushbutton.h>
#include <qtimer.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kdialogbase.h>
#if KDE_IS_VERSION(3,2,0)
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

// Local includes.

#include "renamecustomizer.h"
#include "renamecustomizer.moc"

namespace Digikam
{

class RenameCustomizerPriv
{
public:

    enum DateFormatOptions
    {
        DigikamStandard = 0,
        IsoDateFormat,
        TextDateFormat,
        LocalDateFormat,
        Advanced
    };

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
        renameCustomSuffix    = 0;
        startIndexLabel       = 0;
        startIndexInput       = 0;
        focusedWidget         = 0;
        dateTimeButton        = 0;
        dateTimeLabel         = 0;
        dateTimeFormat        = 0;
}

    QWidget      *focusedWidget;

    QString       cameraTitle;

    QRadioButton *renameDefault;
    QRadioButton *renameCustom;

    QGroupBox    *renameDefaultBox;
    QGroupBox    *renameCustomBox;
    
    QLabel       *renameDefaultCase;
    QLabel       *startIndexLabel;
    QLabel       *dateTimeLabel;

    QComboBox    *renameDefaultCaseType;
    QComboBox    *dateTimeFormat;

    QCheckBox    *addDateTimeBox;
    QCheckBox    *addCameraNameBox;
    QCheckBox    *addSeqNumberBox;

    QPushButton  *dateTimeButton;
    QString       dateTimeFormatString;

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

    setFrameStyle( QFrame::NoFrame );
    setRadioButtonExclusive(true);
    setColumnLayout(0, Qt::Vertical);
    QGridLayout* mainLayout = new QGridLayout(layout(), 4, 1);

    // ----------------------------------------------------------------

    d->renameDefault = new QRadioButton(i18n("Camera filenames"), this);
    QWhatsThis::add( d->renameDefault, i18n("<p>Toggle on this option to use camera "
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
                                           
    QHBoxLayout* boxLayout1 = new QHBoxLayout( d->renameDefaultBox->layout() );
    boxLayout1->addSpacing( 10 );
    boxLayout1->addWidget( d->renameDefaultCase );
    boxLayout1->addWidget( d->renameDefaultCaseType );

    mainLayout->addMultiCellWidget(d->renameDefaultBox, 1, 1, 0, 1);

    // -------------------------------------------------------------

    d->renameCustom = new QRadioButton(i18n("Customize"), this);
    mainLayout->addMultiCellWidget(d->renameCustom, 2, 2, 0, 1);
    QWhatsThis::add( d->renameCustom, i18n("<p>Toggle on this option to customize image filenames "
                                           "during download."));

    d->renameCustomBox = new QGroupBox(this);
    d->renameCustomBox->setFrameStyle(QFrame::NoFrame|QFrame::Plain);
    d->renameCustomBox->setInsideMargin(0);
    d->renameCustomBox->setColumnLayout(0, Qt::Vertical);

    QGridLayout* renameCustomBoxLayout = new QGridLayout(d->renameCustomBox->layout(), 
                                                         6, 2, KDialogBase::spacingHint());
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

    QWidget *dateTimeWidget = new QWidget(d->renameCustomBox);
    d->dateTimeLabel    = new QLabel(i18n("Date format:"), dateTimeWidget);
    d->dateTimeFormat   = new QComboBox(dateTimeWidget);
    d->dateTimeFormat->insertItem(i18n("Standard"),       RenameCustomizerPriv::DigikamStandard);
    d->dateTimeFormat->insertItem(i18n("ISO"),            RenameCustomizerPriv::IsoDateFormat);
    d->dateTimeFormat->insertItem(i18n("Full Text"),      RenameCustomizerPriv::TextDateFormat);
    d->dateTimeFormat->insertItem(i18n("Local Settings"), RenameCustomizerPriv::LocalDateFormat);
    d->dateTimeFormat->insertItem(i18n("Advanced..."),    RenameCustomizerPriv::Advanced);
    QWhatsThis::add( d->dateTimeFormat, i18n("<p>Select here your preferred date format used to "
                    "create new albums. The options available are:</p>"
                    "<p><b>Standard</b>: the date format that has been used as a standard by digiKam. "
                    "E.g.: <i>20060824T142618</i></p>"
                    "<p/><b>ISO</b>: the date format is in accordance with ISO 8601 "
                    "(YYYY-MM-DD). E.g.: <i>2006-08-24T14:26:18</i></p>"
                    "<p><b>Full Text</b>: the date format is in a user-readable string. "
                    "E.g.: <i>Thu Aug 24 14:26:18 2006</i></p>"
                    "<p><b>Local Settings</b>: the date format depending on KDE control panel settings.</p>"
                    "<p><b>Advanced:</b> allows to specify a custom date format.</p>"));
    d->dateTimeButton = new QPushButton(SmallIcon("configure"), QString(), dateTimeWidget);
    QSizePolicy policy = d->dateTimeButton->sizePolicy();
    policy.setHorData(QSizePolicy::Maximum);
    d->dateTimeButton->setSizePolicy(policy);
    QHBoxLayout *boxLayout2 = new QHBoxLayout(dateTimeWidget);
    boxLayout2->addWidget(d->dateTimeLabel);
    boxLayout2->addWidget(d->dateTimeFormat);
    boxLayout2->addWidget(d->dateTimeButton);
    renameCustomBoxLayout->addMultiCellWidget(dateTimeWidget, 3, 3, 1, 2);

    d->addCameraNameBox = new QCheckBox( i18n("Add Camera Name"), d->renameCustomBox );
    renameCustomBoxLayout->addMultiCellWidget(d->addCameraNameBox, 4, 4, 1, 2);
    QWhatsThis::add( d->addCameraNameBox, i18n("<p>Set this option to add the camera name."));

    d->addSeqNumberBox = new QCheckBox( i18n("Add Sequence Number"), d->renameCustomBox );
    renameCustomBoxLayout->addMultiCellWidget(d->addSeqNumberBox, 5, 5, 1, 2);
    QWhatsThis::add( d->addSeqNumberBox, i18n("<p>Set this option to add a sequence number "
                                              "starting with the index set below."));

    d->startIndexLabel = new QLabel( i18n("Start Index:"), d->renameCustomBox );
    d->startIndexInput = new KIntNumInput(1, d->renameCustomBox);
    d->startIndexInput->setRange(1, 900000, 1, false);
    QWhatsThis::add( d->startIndexInput, i18n("<p>Set here the start index value used to rename picture "
                                              "files with a sequence number."));

    renameCustomBoxLayout->addMultiCellWidget(d->startIndexLabel, 6, 6, 1, 1);
    renameCustomBoxLayout->addMultiCellWidget(d->startIndexInput, 6, 6, 2, 2);

    mainLayout->addMultiCellWidget(d->renameCustomBox, 3, 3, 0, 1);
    mainLayout->setRowStretch(4, 10);

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

    connect(d->dateTimeButton, SIGNAL(clicked()),
            this, SLOT(slotDateTimeButtonClicked()));

    connect(d->dateTimeFormat, SIGNAL(activated(int)),
            this, SLOT(slotDateTimeFormatChanged(int)));

    connect(d->addDateTimeBox, SIGNAL(toggled(bool)),
            this, SLOT(slotDateTimeBoxToggled(bool)));

    // -- initial values ---------------------------------------------------

    readSettings();

    // signal to this not yet connected when readSettings is called? Don't know
    slotDateTimeBoxToggled(d->addDateTimeBox->isChecked());
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
        QString date;
        switch (d->dateTimeFormat->currentItem())
        {
            case RenameCustomizerPriv::DigikamStandard:
                date = dateTime.toString("yyyyMMddThhmmss");
                break;
            case RenameCustomizerPriv::TextDateFormat:
                date = dateTime.toString(Qt::TextDate);
                break;
            case RenameCustomizerPriv::LocalDateFormat:
                date = dateTime.toString(Qt::LocalDate);
                break;
            case RenameCustomizerPriv::IsoDateFormat:
                date = dateTime.toString(Qt::ISODate);
                break;
            case RenameCustomizerPriv::Advanced:
                date = dateTime.toString(d->dateTimeFormatString);
                break;
         }

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

void RenameCustomizer::slotDateTimeBoxToggled(bool on)
{
    d->dateTimeLabel->setEnabled(on);
    d->dateTimeFormat->setEnabled(on);
    d->dateTimeButton->setEnabled(on
            && d->dateTimeFormat->currentItem() == RenameCustomizerPriv::Advanced);
    slotRenameOptionsChanged();
}

void RenameCustomizer::slotDateTimeFormatChanged(int index)
{
    if (index == RenameCustomizerPriv::Advanced)
    {
        d->dateTimeButton->setEnabled(true);
        //d->dateTimeButton->show();
        //slotDateTimeButtonClicked();
    }
    else
    {
        d->dateTimeButton->setEnabled(false);
        //d->dateTimeButton->hide();
    }
    slotRenameOptionsChanged();
}

void RenameCustomizer::slotDateTimeButtonClicked()
{
    bool ok;
    QString message = i18n("<qt><p>Enter the format for date and time.</p>"
                           "<p>Use <i>dd</i> for the day, "
                           "<i>MM</i> for the month, "
                           "<i>yyyy</i> for the year, "
                           "<i>hh</i> for the hour, "
                           "<i>mm</i> for the minute, "
                           "<i>ss</i> for the second.</p>"
                           "<p>Examples: <i>yyyyMMddThhmmss</i> "
                           "for 20060824T142418,<br>"
                           "<i>yyyy-MM-dd hh:mm:ss</i> "
                           "for 2006-08-24 14:24:18.</p></qt>");

#if KDE_IS_VERSION(3,2,0)
    QString newFormat = KInputDialog::getText(i18n("Change Date and Time Format"),
                                              message,
                                              d->dateTimeFormatString, &ok, this);
#else
    QString newFormat = KLineEditDlg::getText(i18n("Change Date and Time Format"),
                                              message,
                                              d->dateTimeFormatString, &ok, this);
#endif

    if (!ok)
        return;

    d->dateTimeFormatString = newFormat;
    slotRenameOptionsChanged();
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
    int dateTime     = config->readNumEntry("Date Time Format", RenameCustomizerPriv::IsoDateFormat);
    QString format   = config->readEntry("Date Time Format String", "yyyyMMddThhmmss");

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
    d->dateTimeFormat->setCurrentItem(dateTime);
    d->dateTimeFormatString = format;
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
    config->writeEntry("Date Time Format", d->dateTimeFormat->currentItem());
    config->writeEntry("Date Time Format String", d->dateTimeFormatString);
    config->sync();
}

void RenameCustomizer::restoreFocus()
{
    d->focusedWidget->setFocus();
}

}  // namespace Digikam

