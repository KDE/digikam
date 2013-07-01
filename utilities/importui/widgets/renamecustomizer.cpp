/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-19
 * Description : a options group to set renaming files
 *               operations during camera downloading
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Andi Clemens <andi dot clemens at gmail dot com>
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

class RenameCustomizer::Private
{
public:

    Private() :
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

RenameCustomizer::RenameCustomizer(QWidget* const parent, const QString& cameraTitle)
    : QWidget(parent), d(new Private)
{
    d->changedTimer = new QTimer(this);
    d->cameraTitle  = cameraTitle;
    d->buttonGroup  = new QButtonGroup(this);
    d->buttonGroup->setExclusive(true);

    setAttribute(Qt::WA_DeleteOnClose);

    QGridLayout* mainLayout  = new QGridLayout(this);

    // ----------------------------------------------------------------

    d->renameDefault         = new QRadioButton(i18n("Camera filenames"), this);
    d->buttonGroup->addButton(d->renameDefault, 0);
    d->renameDefault->setWhatsThis(i18n("Turn on this option to use the camera "
                                        "provided image filenames without modifications."));

    d->renameDefaultBox      = new QWidget(this);
    QHBoxLayout* boxLayout1  = new QHBoxLayout(d->renameDefaultBox);

    d->renameDefaultCase     = new QLabel(i18n("Change case to:"), d->renameDefaultBox);
    d->renameDefaultCase->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    d->renameDefaultCaseType = new KComboBox(d->renameDefaultBox);
    d->renameDefaultCaseType->insertItem(NONE,  i18nc("Leave filename as it is", "Leave as-is"));
    d->renameDefaultCaseType->insertItem(UPPER, i18nc("Filename to uppercase",   "Upper"));
    d->renameDefaultCaseType->insertItem(LOWER, i18nc("Filename to lowercase",   "Lower"));
    d->renameDefaultCaseType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    d->renameDefaultCaseType->setWhatsThis(i18n("Set the method to use to change the case "
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

    connect(d->renameDefaultCaseType, SIGNAL(activated(QString)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->changedTimer, SIGNAL(timeout()),
            this, SIGNAL(signalChanged()));

    connect(d->advancedRenameWidget, SIGNAL(signalTextChanged(QString)),
            this, SLOT(slotCustomRenameChanged()));

    // --------------------------------------------------------

    readSettings();
}

RenameCustomizer::~RenameCustomizer()
{
    saveSettings();
    delete d->advancedRenameManager;
    delete d;
}

void RenameCustomizer::setUseDefault(bool val)
{
    if (val)
    {
        d->renameDefault->setChecked(true);
    }
    else
    {
        d->renameCustom->setChecked(true);
    }
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

    QString result = fileName.trimmed();

    if (d->renameDefault->isChecked())
    {
        switch (changeCase())
        {
        case UPPER: return result.toUpper(); break;
        case LOWER: return result.toLower(); break;
        default: return result;
        }
    }

    return d->advancedRenameManager->newName(result);
}

RenameCustomizer::Case RenameCustomizer::changeCase() const
{
    return static_cast<Case>(d->renameDefaultCaseType->currentIndex());
}

void RenameCustomizer::setChangeCase(RenameCustomizer::Case val)
{
    d->renameDefaultCaseType->setCurrentIndex(val);
}

void RenameCustomizer::slotRadioButtonClicked(int id)
{
    QRadioButton* btn = dynamic_cast<QRadioButton*>(d->buttonGroup->button(id));

    if (!btn)
    {
        return;
    }

    btn->setChecked(true);
    d->renameDefaultBox->setEnabled(btn == d->renameDefault);
    d->advancedRenameWidget->setEnabled(btn == d->renameCustom);
    slotRenameOptionsChanged();
}

void RenameCustomizer::slotRenameOptionsChanged()
{
    d->focusedWidget = focusWidget();

    d->changedTimer->setSingleShot(true);
    d->changedTimer->start(500);
}

void RenameCustomizer::slotCustomRenameChanged()
{
    d->advancedRenameManager->parseFiles();
    slotRenameOptionsChanged();
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
