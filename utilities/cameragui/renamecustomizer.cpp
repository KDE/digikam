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
#include <QGridLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QTimer>

// KDE includes

#include <kcombobox.h>
#include <kconfig.h>
#include <klocale.h>

// Local includes

#include "dcursortracker.h"
#include "parser.h"
#include "manualrenamewidget.h"

using namespace Digikam::ManualRename;
namespace Digikam
{

class RenameCustomizerPriv
{
public:

    RenameCustomizerPriv()
    {
        buttonGroup           = 0;
        changedTimer          = 0;
        focusedWidget         = 0;
        manualRenameInput     = 0;
        renameDefault         = 0;
        renameDefaultBox      = 0;
        renameDefaultCase     = 0;
        renameDefaultCaseType = 0;
        renameCustom          = 0;
        startIndex            = 1;
}

    int                 startIndex;

    QButtonGroup*       buttonGroup;

    QLabel*             renameDefaultCase;

    QRadioButton*       renameDefault;
    QRadioButton*       renameCustom;

    QString             cameraTitle;
    QString             dateTimeFormatString;

    QTimer*             changedTimer;

    QWidget*            focusedWidget;
    QWidget*            renameDefaultBox;

    KComboBox*          renameDefaultCaseType;

    ManualRenameWidget* manualRenameInput;
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

    d->renameCustom      = new QRadioButton(i18nc("Custom Image Renaming", "Customize"), this);
    d->manualRenameInput = new ManualRenameWidget(this);
    d->manualRenameInput->setTrackerAlignment(Qt::AlignRight);
    d->manualRenameInput->setParserInputStyle(ManualRenameWidget::BigButtons);
    d->buttonGroup->addButton(d->renameCustom, 2);

    mainLayout->addWidget(d->renameDefault,     0, 0, 1, 2);
    mainLayout->addWidget(d->renameDefaultBox,  1, 0, 1, 2);
    mainLayout->addWidget(d->renameCustom,      4, 0, 1, 2);
    mainLayout->addWidget(d->manualRenameInput, 5, 0, 1, 2);
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

    connect(d->manualRenameInput, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

    // --------------------------------------------------------

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
    return d->startIndex;
}

void RenameCustomizer::setStartIndex(int startIndex)
{
    d->startIndex = startIndex;
}

QString RenameCustomizer::newName(const QString& fileName, const QDateTime& dateTime,
                                  int index, const QString &extension) const
{

    if (d->renameDefault->isChecked())
        return QString();

    QString name;
    QString cameraName = QString("%1").arg(d->cameraTitle.simplified().remove(' '));

    if (d->renameCustom->isChecked())
    {
        ParseInformation info;
        info.filePath   = fileName;
        info.cameraName = cameraName;
        info.datetime   = dateTime;
        info.index      = index;

        name = d->manualRenameInput->parse(info);
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

    d->renameDefaultBox->setEnabled( btn == d->renameDefault );
    d->manualRenameInput->setEnabled( btn == d->renameCustom );
    d->manualRenameInput->slotHideToolTipTracker();
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

    switch(def)
    {
        case 0:
        {
            d->renameDefault->setChecked(true);
            d->renameDefaultBox->setEnabled(true);
            d->renameCustom->setChecked(false);
            d->manualRenameInput->setEnabled(false);
            break;
        }
        case 1:
        {
            d->renameDefault->setChecked(false);
            d->renameDefaultBox->setEnabled(false);
            d->renameCustom->setChecked(false);
            d->manualRenameInput->setEnabled(false);
            break;
        }
        default:
        {
            d->renameDefault->setChecked(false);
            d->renameDefaultBox->setEnabled(false);
            d->renameCustom->setChecked(true);
            d->manualRenameInput->setEnabled(true);
            break;
        }
    }

    d->renameDefaultCaseType->setCurrentIndex(chcaseT);
    d->manualRenameInput->setText(manualRename);
    slotRenameOptionsChanged();
}

void RenameCustomizer::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group("Camera Settings");
    group.writeEntry("Rename Method",        d->buttonGroup->checkedId());
    group.writeEntry("Case Type",            d->renameDefaultCaseType->currentIndex());
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
