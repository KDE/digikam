/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-19
 * Description : a options group to set renaming files
 *               operations during camera downloading
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "renamecustomizer.h"
#include "renamecustomizer.moc"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QDateTime>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QTimer>
#include <QTextStream>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kdialog.h>
#include <khbox.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kicon.h>

// Local includes

#include "dcursortracker.h"

namespace Digikam
{

class ManualRenameInputPriv
{
public:

    ManualRenameInputPriv()
    {
        parseStringLineEdit = 0;
        tooltipTracker      = 0;
        tooltipToggleButton = 0;
    }

    QToolButton* tooltipToggleButton;
    KLineEdit*   parseStringLineEdit;
    DTipTracker* tooltipTracker;
};

ManualRenameInput::ManualRenameInput(QWidget* parent)
                 : QWidget(parent), d(new ManualRenameInputPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QGridLayout* mainLayout = new QGridLayout(this);
    d->parseStringLineEdit  = new KLineEdit(this);
    d->tooltipToggleButton  = new QToolButton(this);
    d->tooltipToggleButton->setCheckable(true);
    d->tooltipToggleButton->setIcon(SmallIcon("dialog-information"));

    mainLayout->addWidget(d->parseStringLineEdit, 1, 1, 1, 1);
    mainLayout->addWidget(d->tooltipToggleButton, 1, 2, 1, 1);
    mainLayout->setColumnStretch(1, 10);
    setLayout(mainLayout);

    QString tooltip   = createToolTip();
    d->tooltipTracker = new DTipTracker(tooltip, d->parseStringLineEdit, Qt::AlignLeft);
    d->tooltipTracker->setEnable(false);
    d->tooltipTracker->setKeepOpen(true);

    QString additionalInfo = i18n("<p><i>Example:</i><br/>"
                                  "<b>new_$_###</b><br/>"
                                  "=> new_MyImageName_001.jpg"
                                  "</p>");
    d->parseStringLineEdit->setWhatsThis(tooltip + additionalInfo);
    d->parseStringLineEdit->setClearButtonShown(true);
    d->parseStringLineEdit->setCompletionMode(KGlobalSettings::CompletionAuto);

    connect(d->tooltipToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(slotToolTipButtonToggled(bool)));

    connect(d->parseStringLineEdit, SIGNAL(textChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));

    connect(d->tooltipTracker, SIGNAL(signalClicked()),
            this, SLOT(slotHideToolTipTracker()));
}

ManualRenameInput::~ManualRenameInput()
{
    // we need to delete it manually, because it has no parent
    delete d->tooltipTracker;
    delete d;
}

QString ManualRenameInput::text() const
{
    return d->parseStringLineEdit->text();
}

void ManualRenameInput::setText(const QString &text)
{
    d->parseStringLineEdit->setText(text);
}

void ManualRenameInput::setTrackerAlignment(Qt::Alignment alignment)
{
    d->tooltipTracker->setTrackerAlignment(alignment);
}

KLineEdit* ManualRenameInput::input() const
{
    return d->parseStringLineEdit;
}

void ManualRenameInput::slotHideToolTipTracker()
{
    d->tooltipToggleButton->setChecked(false);
    slotToolTipButtonToggled(false);
}

QString ManualRenameInput::parser(const QString &parse,
                                  const QString &fileName, const QString &cameraName,
                                  const QDateTime &dateTime, int index)
{
    QFileInfo fi(fileName);
    QString parsedString = parse;

    // parse sequence number token ----------------------------
    {
        QRegExp regExp("(#+)(\\{(\\d+)\\s*,\\s*(\\d+)\\})?");
        int pos = 0;
        while (pos > -1)
        {
            pos = regExp.indexIn(parsedString, pos);
            if (pos > -1)
            {
                int slength = 0;
                int start   = 0;
                int step    = 0;

                slength = regExp.cap(1).length();
                start   = regExp.cap(3).isEmpty() ? 1 : regExp.cap(3).toInt();
                step    = regExp.cap(4).isEmpty() ? 1 : regExp.cap(4).toInt();

                QString tmp;
                QTextStream seqStream(&tmp);
                seqStream.setFieldWidth(slength);
                seqStream.setFieldAlignment(QTextStream::AlignRight);
                seqStream.setPadChar('0');
                seqStream << start + ((index-1) * step);
                parsedString.replace(pos, regExp.matchedLength(), tmp);
            }
        }
    }
    // parse date time token ----------------------------------
    {
        QRegExp regExp("\\[date:(.*)\\]");
        regExp.setMinimal(true);
        int pos = 0;
        while (pos > -1)
        {
            pos  = regExp.indexIn(parsedString, pos);
            if (pos > -1)
            {
                QString tmp;
                tmp = dateTime.toString(regExp.cap(1));
                parsedString.replace(pos, regExp.matchedLength(), tmp);
            }
        }
    }
    // parse * token (first letter of each word upper case) ---
    {
        QRegExp regExp("\\*{1}");
        regExp.setMinimal(true);
        int pos = 0;
        while (pos > -1)
        {
            pos  = regExp.indexIn(parsedString, pos);
            if (pos > -1)
            {
                QString tmp = fileName.toLower();
                if( tmp[0].isLetter() )
                    tmp[0] = tmp[0].toUpper();

                for( int i = 0; i < tmp.length(); ++i )
                {
                    if( tmp[i+1].isLetter() && !tmp[i].isLetter() &&
                            tmp[i] != '\'' && tmp[i] != '?' && tmp[i] != '`' )
                    {
                        tmp[i+1] = tmp[i+1].toUpper();
                    }
                }
                parsedString.replace(pos, regExp.matchedLength(), tmp);
            }
        }
    }
    // parse simple / remaining tokens ------------------------
    {
        parsedString.replace("[cam]", cameraName);
        parsedString.replace("$",     fi.baseName());
        parsedString.replace("&",     fi.baseName().toUpper());
        parsedString.replace("%",     fi.baseName().toLower());
    }

    return parsedString;
}

QString ManualRenameInput::parse(const QString &fileName, const QString &cameraName,
                                 const QDateTime &dateTime, int index) const
{
    QString parseString = d->parseStringLineEdit->text();
    return (parser(parseString, fileName, cameraName, dateTime, index));
}

QString ManualRenameInput::createToolTip()
{
    typedef QPair<QString, QString> token;
    QList<token> tokenList;

    tokenList << token(QString("$"),              i18n("filename (original)"))
              << token(QString("&"),              i18n("filename (upper case)"))
              << token(QString("%"),              i18n("filename (lower case)"))
              << token(QString("*"),              i18n("filename (first letter of each word upper case)"))
              << token(QString("#"),              i18n("sequence number"))
              << token(QString("#{start,step}"),  i18n("sequence number (custom start + step)"))
              << token(QString("[cam]"),          i18n("camera name"))
              << token(QString("[date:format]"),  i18n("date and time of the file"));

    QString tooltip;
    tooltip += QString("<table>");

    foreach (const token &token, tokenList)
    {
        tooltip += QString("<tr><td><b>%1</b></td><td>:</td><td>%2</td></tr>").arg(token.first)
                                                                              .arg(token.second);
    }

    tooltip += QString("</table>");
    return tooltip;
}

void ManualRenameInput::slotToolTipButtonToggled(bool checked)
{
    d->tooltipTracker->setVisible(checked);
    slotUpdateTrackerPos();
}

void ManualRenameInput::slotUpdateTrackerPos()
{
    d->tooltipTracker->refresh();
}


// ------------------------------------------------------------------------------


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
        addCameraNameBox      = 0;
        addDateTimeBox        = 0;
        addOrigNameBox        = 0;
        addSeqNumberBox       = 0;
        buttonGroup           = 0;
        changedTimer          = 0;
        dateTimeButton        = 0;
        dateTimeFormat        = 0;
        dateTimeLabel         = 0;
        focusedWidget         = 0;
        manualRenameInput     = 0;
        renameCustom          = 0;
        renameCustomBox       = 0;
        renameCustomPrefix    = 0;
        renameCustomSuffix    = 0;
        renameDefault         = 0;
        renameDefaultBox      = 0;
        renameDefaultCase     = 0;
        renameDefaultCaseType = 0;
        renameManual          = 0;
        startIndexInput       = 0;
        startIndexLabel       = 0;
}

    QButtonGroup        *buttonGroup;

    QCheckBox           *addCameraNameBox;
    QCheckBox           *addDateTimeBox;
    QCheckBox           *addOrigNameBox;
    QCheckBox           *addSeqNumberBox;

    QLabel              *dateTimeLabel;
    QLabel              *renameDefaultCase;
    QLabel              *startIndexLabel;

    QPushButton         *dateTimeButton;

    QRadioButton        *renameCustom;
    QRadioButton        *renameDefault;
    QRadioButton        *renameManual;

    QString              cameraTitle;
    QString              dateTimeFormatString;

    QTimer              *changedTimer;

    QWidget             *focusedWidget;
    QWidget             *renameCustomBox;
    QWidget             *renameDefaultBox;

    KComboBox           *dateTimeFormat;
    KComboBox           *renameDefaultCaseType;

    KIntNumInput        *startIndexInput;
    KLineEdit           *renameCustomPrefix;
    KLineEdit           *renameCustomSuffix;

    ManualRenameInput   *manualRenameInput;
};

RenameCustomizer::RenameCustomizer(QWidget* parent, const QString& cameraTitle)
                : QWidget(parent), d(new RenameCustomizerPriv)
{
    d->changedTimer = new QTimer(this);
    d->cameraTitle  = cameraTitle;
    d->buttonGroup  = new QButtonGroup(this);
    d->buttonGroup->setExclusive(true);

    setAttribute(Qt::WA_DeleteOnClose);

    QGridLayout* mainLayout = new QGridLayout(this);

    // ----------------------------------------------------------------

    d->renameDefault = new QRadioButton(i18n("Camera filenames"), this);
    d->buttonGroup->addButton(d->renameDefault);
    d->renameDefault->setWhatsThis(i18n("Turn on this option to use the camera "
                                        "provided image filenames without modifications."));

    d->renameDefaultBox     = new QWidget(this);
    QHBoxLayout* boxLayout1 = new QHBoxLayout(d->renameDefaultBox);

    d->renameDefaultCase = new QLabel(i18n("Change case to:"), d->renameDefaultBox);
    d->renameDefaultCase->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    d->renameDefaultCaseType = new KComboBox(d->renameDefaultBox);
    d->renameDefaultCaseType->insertItem(0, i18n("Leave as-is"));
    d->renameDefaultCaseType->insertItem(1, i18n("Upper"));
    d->renameDefaultCaseType->insertItem(2, i18n("Lower"));
    d->renameDefaultCaseType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    d->renameDefaultCaseType->setWhatsThis( i18n("Set the method to use to change the case "
                                                 "of the image filenames."));

    boxLayout1->setMargin(KDialog::spacingHint());
    boxLayout1->setSpacing(KDialog::spacingHint());
    boxLayout1->addSpacing(10);
    boxLayout1->addWidget(d->renameDefaultCase);
    boxLayout1->addWidget(d->renameDefaultCaseType);

    // -------------------------------------------------------------

    d->renameCustom = new QRadioButton(i18n("Customize"), this);
    d->buttonGroup->addButton(d->renameCustom);
    d->renameCustom->setWhatsThis(i18n("Turn on this option to customize the image filenames "
                                       "during download."));

    d->renameCustomBox                 = new QWidget(this);
    QGridLayout* renameCustomBoxLayout = new QGridLayout(d->renameCustomBox);

    QLabel* prefixLabel   = new QLabel(i18n("Prefix:"), d->renameCustomBox);
    d->renameCustomPrefix = new KLineEdit(d->renameCustomBox);
    d->focusedWidget = d->renameCustomPrefix;
    d->renameCustomPrefix->setWhatsThis( i18n("Set the prefix which will be added to "
                                              "the image filenames."));

    QLabel* suffixLabel   = new QLabel(i18n("Suffix:"), d->renameCustomBox);
    d->renameCustomSuffix = new KLineEdit(d->renameCustomBox);
    d->renameCustomSuffix->setWhatsThis(i18n("Set the suffix which will be added to "
                                             "the image filenames."));

    d->addOrigNameBox = new QCheckBox(i18n("Add Original &Filename"), d->renameCustomBox);
    d->addOrigNameBox->setWhatsThis(i18n("Set this option to add the original filename."));

    d->addDateTimeBox = new QCheckBox(i18n("Add Date && Time"), d->renameCustomBox);
    d->addDateTimeBox->setWhatsThis( i18n("Set this option to add the camera-provided date and time."));

    QWidget *dateTimeWidget = new QWidget(d->renameCustomBox);
    QHBoxLayout *boxLayout2 = new QHBoxLayout(dateTimeWidget);
    d->dateTimeLabel        = new QLabel(i18n("Date format:"), dateTimeWidget);
    d->dateTimeFormat       = new KComboBox(dateTimeWidget);
    d->dateTimeFormat->insertItem(RenameCustomizerPriv::DigikamStandard, i18n("Standard"));
    d->dateTimeFormat->insertItem(RenameCustomizerPriv::IsoDateFormat,   i18n("ISO"));
    d->dateTimeFormat->insertItem(RenameCustomizerPriv::TextDateFormat,  i18n("Full Text"));
    d->dateTimeFormat->insertItem(RenameCustomizerPriv::LocalDateFormat, i18n("Local Settings"));
    d->dateTimeFormat->insertItem(RenameCustomizerPriv::Advanced,        i18n("Advanced..."));
    d->dateTimeFormat->setWhatsThis( i18n("<p>Select your preferred date format for "
                    "creating the new albums. The options available are:</p>"
                    "<p><b>Standard</b>: the date format that has been used as a standard by digiKam. "
                    "E.g.: <i>20060824T142618</i></p>"
                    "<p><b>ISO</b>: the date format according to ISO 8601 "
                    "(YYYY-MM-DD). E.g.: <i>2006-08-24T14:26:18</i></p>"
                    "<p><b>Full Text</b>: the date format is a user-readable string. "
                    "E.g.: <i>Thu Aug 24 14:26:18 2006</i></p>"
                    "<p><b>Local Settings</b>: the date format depending on the KDE control panel settings.</p>"
                    "<p><b>Advanced:</b> allows the user to specify a custom date format.</p>"));

    d->dateTimeButton = new QPushButton(SmallIcon("configure"), QString(), dateTimeWidget);
    QSizePolicy policy = d->dateTimeButton->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Maximum);
    d->dateTimeButton->setSizePolicy(policy);

    boxLayout2->addWidget(d->dateTimeLabel);
    boxLayout2->addWidget(d->dateTimeFormat);
    boxLayout2->addWidget(d->dateTimeButton);
    boxLayout2->setMargin(KDialog::spacingHint());
    boxLayout2->setSpacing(KDialog::spacingHint());

    d->addCameraNameBox = new QCheckBox(i18n("Add Camera Name"), d->renameCustomBox);
    d->addCameraNameBox->setWhatsThis(i18n("Set this option to add the camera name."));

    d->addSeqNumberBox = new QCheckBox(i18n("Add Sequence Number"), d->renameCustomBox);
    d->addSeqNumberBox->setWhatsThis(i18n("Set this option to add a sequence number "
                                          "starting with the index set below."));

    d->startIndexLabel = new QLabel(i18n("Start Index:"), d->renameCustomBox);
    d->startIndexInput = new KIntNumInput(1, d->renameCustomBox);
    d->startIndexInput->setRange(1, 900000, 1);
    d->startIndexInput->setSliderEnabled(false);
    d->startIndexInput->setWhatsThis(i18n("Set the starting index value used to rename "
                                          "files with a sequence number."));

    // -----------------------------------------------------------------------------

    renameCustomBoxLayout->addWidget(prefixLabel,           0, 1, 1, 1);
    renameCustomBoxLayout->addWidget(d->renameCustomPrefix, 0, 2, 1, 1);
    renameCustomBoxLayout->addWidget(suffixLabel,           1, 1, 1, 1);
    renameCustomBoxLayout->addWidget(d->renameCustomSuffix, 1, 2, 1, 1);
    renameCustomBoxLayout->addWidget(d->addOrigNameBox,     2, 1, 1, 2);
    renameCustomBoxLayout->addWidget(d->addDateTimeBox,     3, 1, 1, 2);
    renameCustomBoxLayout->addWidget(dateTimeWidget,        4, 1, 1, 2);
    renameCustomBoxLayout->addWidget(d->addCameraNameBox,   5, 1, 1, 2);
    renameCustomBoxLayout->addWidget(d->addSeqNumberBox,    6, 1, 1, 2);
    renameCustomBoxLayout->addWidget(d->startIndexLabel,    7, 1, 1, 1);
    renameCustomBoxLayout->addWidget(d->startIndexInput,    7, 2, 1, 1);
    renameCustomBoxLayout->setColumnMinimumWidth(0, 10);
    renameCustomBoxLayout->setMargin(KDialog::spacingHint());
    renameCustomBoxLayout->setSpacing(KDialog::spacingHint());

    // ----------------------------------------------------------------------

    d->renameManual      = new QRadioButton(i18n("Manual"), this);
    d->manualRenameInput = new ManualRenameInput;
    d->manualRenameInput->setTrackerAlignment(Qt::AlignRight);
    d->buttonGroup->addButton(d->renameManual);

    mainLayout->addWidget(d->renameDefault,     0, 0, 1, 2);
    mainLayout->addWidget(d->renameDefaultBox,  1, 0, 1, 2);
    mainLayout->addWidget(d->renameCustom,      2, 0, 1, 2);
    mainLayout->addWidget(d->renameCustomBox,   3, 0, 1, 2);
    mainLayout->addWidget(d->renameManual,      4, 0, 1, 2);
    mainLayout->addWidget(d->manualRenameInput, 5, 0, 1, 2);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setSpacing(KDialog::spacingHint());

    // -- setup connections -------------------------------------------------

    connect(d->buttonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotRadioButtonClicked(int)));

    connect(d->renameCustomPrefix, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->renameCustomSuffix, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->addOrigNameBox, SIGNAL(toggled(bool)),
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

    connect(d->manualRenameInput, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

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

QString RenameCustomizer::newName(const QString &fileName, const QDateTime &dateTime,
                                  int index, const QString &extension) const
{

    if (d->renameDefault->isChecked())
        return QString();

    QString name;
    QString cameraName = QString("%1").arg(d->cameraTitle.simplified().remove(' '));

    if (d->renameCustom->isChecked())
    {
        name = d->renameCustomPrefix->text();

        // use the "T" as a delimiter between date and time
        QString date;
        switch (d->dateTimeFormat->currentIndex())
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

        if (d->addOrigNameBox->isChecked())
        {
            QFileInfo fi(fileName);
            name += fi.baseName();
        }

        if (d->addDateTimeBox->isChecked())
            name += date;

        if (d->addSeqNumberBox->isChecked())
            name += seq;

        if (d->addCameraNameBox->isChecked())
        {
            if (!name.isEmpty())
                name += QString("-");
            name += cameraName;
        }

        name += d->renameCustomSuffix->text();
        name += extension;
    }
    else if (d->renameManual->isChecked())
    {
        name =  d->manualRenameInput->parse(fileName, cameraName, dateTime, index);
        name += extension;
    }

    return name;
}

RenameCustomizer::Case RenameCustomizer::changeCase() const
{
    RenameCustomizer::Case type = NONE;

    if (d->renameDefaultCaseType->currentIndex() == 1)
        type=UPPER;
    if (d->renameDefaultCaseType->currentIndex() == 2)
        type=LOWER;

    return type;
}

void RenameCustomizer::slotRadioButtonClicked(int)
{
    QRadioButton* btn = dynamic_cast<QRadioButton*>(d->buttonGroup->checkedButton());
    if (!btn)
        return;

    d->renameCustomBox->setEnabled( btn == d->renameCustom );
    d->renameDefaultBox->setEnabled( btn == d->renameDefault );
    d->manualRenameInput->setEnabled( btn == d->renameManual );
    d->manualRenameInput->slotHideToolTipTracker();
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

    d->changedTimer->setSingleShot(true);
    d->changedTimer->start(500);
}

void RenameCustomizer::slotDateTimeBoxToggled(bool on)
{
    d->dateTimeLabel->setEnabled(on);
    d->dateTimeFormat->setEnabled(on);
    d->dateTimeButton->setEnabled(on
            && d->dateTimeFormat->currentIndex() == RenameCustomizerPriv::Advanced);
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
    QString message = i18n("<p>Enter the format for date and time.</p>"
                           "<p>Use <i>dd</i> for the day, "
                           "<i>MM</i> for the month, "
                           "<i>yyyy</i> for the year, "
                           "<i>hh</i> for the hour, "
                           "<i>mm</i> for the minute, "
                           "<i>ss</i> for the second.</p>"
                           "<p>Examples: <i>yyyyMMddThhmmss</i> "
                           "for 20060824T142418,<br/>"
                           "<i>yyyy-MM-dd hh:mm:ss</i> "
                           "for 2006-08-24 14:24:18.</p>");

    QString newFormat = KInputDialog::getText(i18n("Change Date and Time Format"),
                                              message,
                                              d->dateTimeFormatString, &ok, this);
    if (!ok)
        return;

    d->dateTimeFormatString = newFormat;
    slotRenameOptionsChanged();
}

void RenameCustomizer::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group("Camera Settings");
    bool def             = group.readEntry("Rename Use Default", true);
    bool addSeqNumb      = group.readEntry("Add Sequence Number", true);
    bool adddateTime     = group.readEntry("Add Date Time", false);
    bool addCamName      = group.readEntry("Add Camera Name", false);
    int chcaseT          = group.readEntry("Case Type", (int)NONE);
    QString prefix       = group.readEntry("Rename Prefix", i18n("photo"));
    QString suffix       = group.readEntry("Rename Postfix", QString());
    int startIndex       = group.readEntry("Rename Start Index", 1);
    int dateTime         = group.readEntry("Date Time Format", (int)RenameCustomizerPriv::IsoDateFormat);
    QString format       = group.readEntry("Date Time Format String", QString("yyyyMMddThhmmss"));
    QString manualRename = group.readEntry("Manual Rename String", QString());

    if (def)
    {
        d->renameDefault->setChecked(true);
        d->renameDefaultBox->setEnabled(true);
        d->renameCustom->setChecked(false);
        d->renameCustomBox->setEnabled(false);
        d->renameManual->setChecked(false);
        d->manualRenameInput->setEnabled(false);
    }
    else
    {
        d->renameDefault->setChecked(false);
        d->renameDefaultBox->setEnabled(false);
        d->renameCustom->setChecked(true);
        d->renameCustomBox->setEnabled(true);
        d->renameManual->setChecked(true);
        d->manualRenameInput->setEnabled(true);
    }

    d->addDateTimeBox->setChecked(adddateTime);
    d->addCameraNameBox->setChecked(addCamName);
    d->addSeqNumberBox->setChecked(addSeqNumb);
    d->renameDefaultCaseType->setCurrentIndex(chcaseT);
    d->renameCustomPrefix->setText(prefix);
    d->renameCustomSuffix->setText(suffix);
    d->startIndexInput->setValue(startIndex);
    d->dateTimeFormat->setCurrentIndex(dateTime);
    d->dateTimeFormatString = format;
    d->manualRenameInput->setText(manualRename);
    slotRenameOptionsChanged();
}

void RenameCustomizer::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group("Camera Settings");
    group.writeEntry("Rename Use Default", d->renameDefault->isChecked());
    group.writeEntry("Add Camera Name", d->addCameraNameBox->isChecked());
    group.writeEntry("Add Date Time", d->addDateTimeBox->isChecked());
    group.writeEntry("Add Sequence Number", d->addSeqNumberBox->isChecked());
    group.writeEntry("Case Type", d->renameDefaultCaseType->currentIndex());
    group.writeEntry("Rename Prefix", d->renameCustomPrefix->text());
    group.writeEntry("Rename Suffix", d->renameCustomSuffix->text());
    group.writeEntry("Rename Start Index", d->startIndexInput->value());
    group.writeEntry("Date Time Format", d->dateTimeFormat->currentIndex());
    group.writeEntry("Date Time Format String", d->dateTimeFormatString);
    group.writeEntry("Manual Rename String", d->manualRenameInput->text());
    config->sync();
}

void RenameCustomizer::restoreFocus()
{
    d->focusedWidget->setFocus();
}

void RenameCustomizer::slotUpdateTrackerPos()
{
    d->manualRenameInput->slotUpdateTrackerPos();
}

void RenameCustomizer::slotHideToolTipTracker()
{
    d->manualRenameInput->slotHideToolTipTracker();
}

}  // namespace Digikam
