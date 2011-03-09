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
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2011 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "renamecustomizer.moc"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QTimer>

// KDE includes

#include <kcombobox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <klocale.h>

// Local includes

#include "dcursortracker.h"
#include "parsesettings.h"
#include "parser.h"
#include "advancedrenamewidget.h"
#include "importrenameparser.h"
#include "advancedrenamemanager.h"

namespace Digikam
{

class RenameCustomizer::RenameCustomizerPriv
{
public:

    RenameCustomizerPriv() :
        startIndex(1),
        buttonGroup(0),
        renameDefaultCase(0),
        renameDefault(0),
        renameCustom(0),
        changedTimer(0),
        focusedWidget(0),
        renameDefaultBox(0),
        renameDefaultCaseType(0),
        advancedRenameWidget(0),
        advancedRenameManager(0)
    {
    }

    int                    startIndex;

    QButtonGroup*          buttonGroup;

    QLabel*                renameDefaultCase;

    QRadioButton*          renameDefault;
    QRadioButton*          renameCustom;

    QString                cameraTitle;

    QTimer*                changedTimer;

    QWidget*               focusedWidget;
    QWidget*               renameDefaultBox;

    KComboBox*             renameDefaultCaseType;

    AdvancedRenameWidget*  advancedRenameWidget;
    AdvancedRenameManager* advancedRenameManager;
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
    d->buttonGroup->addButton(d->renameDefault, 0);
    d->renameDefault->setWhatsThis(i18n("Turn on this option to use the camera "
                                        "provided image filenames without modifications."));

    d->renameDefaultBox     = new QWidget(this);
    QHBoxLayout* boxLayout1 = new QHBoxLayout(d->renameDefaultBox);

    d->renameDefaultCase = new QLabel(i18n("Change case to:"), d->renameDefaultBox);
    d->renameDefaultCase->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    d->renameDefaultCaseType = new KComboBox(d->renameDefaultBox);
    d->renameDefaultCaseType->insertItem(0, i18nc("Leave filename as it is", "Leave as-is"));
    d->renameDefaultCaseType->insertItem(1, i18nc("Filename to uppercase",   "Upper"));
    d->renameDefaultCaseType->insertItem(2, i18nc("Filename to lowercase",   "Lower"));
    d->renameDefaultCaseType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    d->renameDefaultCaseType->setWhatsThis( i18n("Set the method to use to change the case "
                                                 "of the image filenames."));

    boxLayout1->setMargin(KDialog::spacingHint());
    boxLayout1->setSpacing(KDialog::spacingHint());
    boxLayout1->addSpacing(10);
    boxLayout1->addWidget(d->renameDefaultCase);
    boxLayout1->addWidget(d->renameDefaultCaseType);

    // ----------------------------------------------------------------------

    d->renameCustom          = new QRadioButton(i18nc("Custom Image Renaming", "Customize"), this);

    d->advancedRenameWidget  = new AdvancedRenameWidget(this);
    d->advancedRenameManager = new AdvancedRenameManager();
    d->advancedRenameManager->setParserType(AdvancedRenameManager::ImportParser);
    d->advancedRenameManager->setWidget(d->advancedRenameWidget);

    d->buttonGroup->addButton(d->renameCustom, 2);

    mainLayout->addWidget(d->renameDefault,        0, 0, 1, 2);
    mainLayout->addWidget(d->renameDefaultBox,     1, 0, 1, 2);
    mainLayout->addWidget(d->renameCustom,         4, 0, 1, 2);
    mainLayout->addWidget(d->advancedRenameWidget, 5, 0, 1, 2);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setSpacing(KDialog::spacingHint());

    // -- setup connections -------------------------------------------------

    connect(d->buttonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotRadioButtonClicked(int)));

    connect(d->renameDefaultCaseType, SIGNAL(activated(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->changedTimer, SIGNAL(timeout()),
            this, SIGNAL(signalChanged()));

    connect(d->advancedRenameWidget, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

    // --------------------------------------------------------

    readSettings();
}

RenameCustomizer::~RenameCustomizer()
{
    saveSettings();
    delete d->advancedRenameManager;
    delete d;
}

bool RenameCustomizer::useDefault() const
{
    return d->renameDefault->isChecked();
}

int RenameCustomizer::startIndex() const
{
    return d->startIndex;
}

void RenameCustomizer::setStartIndex(int startIndex)
{
    d->startIndex = startIndex;
    d->advancedRenameManager->setStartIndex(startIndex);
}

void RenameCustomizer::reset()
{
    d->advancedRenameManager->reset();
}

AdvancedRenameManager* RenameCustomizer::renameManager() const
{
    return d->advancedRenameManager;
}

QString RenameCustomizer::newName(const QString& fileName, const QDateTime& dateTime) const
{
    Q_UNUSED(dateTime)

    if (d->renameDefault->isChecked())
    {
        return QString();
    }

    return d->advancedRenameManager->newName(fileName);
}

//QString RenameCustomizer::newName(const QString& fileName, const QDateTime& dateTime) const
//{
//
//    if (d->renameDefault->isChecked())
//        return QString();
//
//    QString name;
//    QString cameraName = QString("%1").arg(d->cameraTitle.simplified().remove(' '));
//
//    if (d->renameCustom->isChecked())
//    {
//        ParseSettings settings;
//        settings.fileUrl    = fileName;
//        settings.cameraName = cameraName;
//        settings.dateTime   = dateTime;
//
//        name = d->advancedRenameWidget->parse(settings);
//    }
//
//    return name;
//}

RenameCustomizer::Case RenameCustomizer::changeCase() const
{
    RenameCustomizer::Case type = NONE;

    if (d->renameDefaultCaseType->currentIndex() == 1)
    {
        type=UPPER;
    }

    if (d->renameDefaultCaseType->currentIndex() == 2)
    {
        type=LOWER;
    }

    return type;
}

void RenameCustomizer::slotRadioButtonClicked(int id)
{
    QRadioButton* btn = dynamic_cast<QRadioButton*>(d->buttonGroup->button(id));

    if (!btn)
    {
        return;
    }

    btn->setChecked(true);
    d->renameDefaultBox->setEnabled( btn == d->renameDefault );
    d->advancedRenameWidget->setEnabled( btn == d->renameCustom );
    slotRenameOptionsChanged();
}

void RenameCustomizer::slotRenameOptionsChanged()
{
    d->focusedWidget = focusWidget();

    d->changedTimer->setSingleShot(true);
    d->changedTimer->start(500);
}

void RenameCustomizer::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group   = config->group("Camera Settings");
    int def              = group.readEntry("Rename Method",        0);
    int chcaseT          = group.readEntry("Case Type",            (int)NONE);
    QString manualRename = group.readEntry("Manual Rename String", QString());

    slotRadioButtonClicked(def);

    d->renameDefaultCaseType->setCurrentIndex(chcaseT);
    d->advancedRenameWidget->setParseString(manualRename);
}

void RenameCustomizer::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group("Camera Settings");
    group.writeEntry("Rename Method",        d->buttonGroup->checkedId());
    group.writeEntry("Case Type",            d->renameDefaultCaseType->currentIndex());
    group.writeEntry("Manual Rename String", d->advancedRenameWidget->parseString());
    config->sync();
}

void RenameCustomizer::restoreFocus()
{
    d->focusedWidget->setFocus();
}

}  // namespace Digikam
