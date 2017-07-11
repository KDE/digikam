/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : image editor printing interface.
 *
 * Copyright (C) 2009 by Angelo Naselli <anaselli at linux dot it>
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

#include "printoptionspage.h"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QToolButton>
#include <QPushButton>
#include <QApplication>
#include <QStyle>
#include <QHBoxLayout>
#include <QMessageBox>

// KDE includes

#include <kconfigdialogmanager.h>

// Local includes

#include "digikam_debug.h"
#include "printconfig.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"
#include "iccmanager.h"
#include "signalblocker.h"
#include "ui_printoptionspage.h"
#include "editorwindow.h"

namespace Digikam
{

static inline double unitToInches(PrintOptionsPage::Unit unit)
{
    if (unit == PrintOptionsPage::Inches)
    {
        return 1.0;
    }
    else if (unit == PrintOptionsPage::Centimeters)
    {
        return 1 / 2.54;
    }
    else   // Millimeters
    {
        return 1 / 25.4;
    }
}

// --------------------------------------------------------------------------

class PrintOptionsPage::Private : public Ui_PrintOptionsPage
{
public:

    Private() :
        mParent(0),
        mConfigDialogManager(0),
        colorManaged(0),
        cmPreferences(0),
        cmEnabled(false)
    {
    }

    QWidget*              mParent;
    QSize                 mImageSize;
    QButtonGroup          mScaleGroup;
    QButtonGroup          mPositionGroup;
    KConfigDialogManager* mConfigDialogManager;
    IccProfile            outputProfile;
    QCheckBox*            colorManaged;
    QPushButton*          cmPreferences;
    bool                  cmEnabled;

public:

    void initColorManagement()
    {
        QHBoxLayout* const hlay = new QHBoxLayout(cmbox);
        colorManaged            = new QCheckBox(i18n("Use Color Management for Printing"),
                                                cmbox);
        colorManaged->setChecked(false);
        cmPreferences           = new QPushButton(i18n("Settings..."), cmbox);
        QWidget* const space    = new QWidget(cmbox);
        hlay->addWidget(colorManaged);
        hlay->addWidget(cmPreferences);
        hlay->addWidget(space);
        hlay->setStretchFactor(space, 10);
        hlay->setSpacing(0);
    }

    void initPositionFrame()
    {
        mPositionFrame->setStyleSheet(QLatin1String(
            "QFrame {"
            " background-color: palette(mid);"
            " border: 1px solid palette(dark);"
            "}"
            "QToolButton {"
            " border: none;"
            " background: palette(base);"
            "}"
            "QToolButton:hover {"
            " background: palette(alternate-base);"
            " border: 1px solid palette(highlight);"
            "}"
            "QToolButton:checked {"
            " background-color: palette(highlight);"
            "}"
        ));

        QGridLayout* const layout = new QGridLayout(mPositionFrame);
        layout->setContentsMargins(QMargins());
        layout->setSpacing(1);

        for (int row = 0; row < 3; ++row)
        {
            for (int col = 0; col < 3; ++col)
            {
                QToolButton* const button = new QToolButton(mPositionFrame);
                button->setFixedSize(40, 40);
                button->setCheckable(true);
                layout->addWidget(button, row, col);

                Qt::Alignment alignment;

                if (row == 0)
                {
                    alignment = Qt::AlignTop;
                }
                else if (row == 1)
                {
                    alignment = Qt::AlignVCenter;
                }
                else
                {
                    alignment = Qt::AlignBottom;
                }

                if (col == 0)
                {
                    alignment |= Qt::AlignLeft;
                }
                else if (col == 1)
                {
                    alignment |= Qt::AlignHCenter;
                }
                else
                {
                    alignment |= Qt::AlignRight;
                }

                mPositionGroup.addButton(button, int (alignment));
            }
        }
    }
};

PrintOptionsPage::PrintOptionsPage(QWidget* const parent,
                                   const QSize& imageSize)
    : QWidget(),
      d(new Private)
{
    d->setupUi(this);
    d->mParent              = parent;
    d->mImageSize           = imageSize;
    d->mConfigDialogManager = new KConfigDialogManager(this, PrintConfig::self());

    d->initPositionFrame();

    //color management
    d->initColorManagement();

    d->mScaleGroup.addButton(d->mNoScale,     NoScale);
    d->mScaleGroup.addButton(d->mScaleToPage, ScaleToPage);
    d->mScaleGroup.addButton(d->mScaleTo,     ScaleToCustomSize);

    connect(d->kcfg_PrintWidth, SIGNAL(valueChanged(double)),
            this, SLOT(adjustHeightToRatio()));

    connect(d->kcfg_PrintHeight, SIGNAL(valueChanged(double)),
            this, SLOT(adjustWidthToRatio()));

    connect(d->kcfg_PrintKeepRatio, SIGNAL(toggled(bool)),
            this, SLOT(adjustHeightToRatio()));

    connect(d->colorManaged, SIGNAL(toggled(bool)),
            this, SLOT(slotAlertSettings(bool)));

    connect(d->cmPreferences, SIGNAL(clicked()),
            this, SLOT(slotSetupDlg()));
}

PrintOptionsPage::~PrintOptionsPage()
{
    delete d;
}

bool PrintOptionsPage::colorManaged() const
{
    return d->colorManaged->isChecked();
}

bool PrintOptionsPage::autoRotation() const
{
    return d->kcfg_PrintAutoRotate->isChecked();
}

IccProfile PrintOptionsPage::outputProfile() const
{
    return d->outputProfile;
}

Qt::Alignment PrintOptionsPage::alignment() const
{
    int id = d->mPositionGroup.checkedId();
    qCWarning(DIGIKAM_GENERAL_LOG) << "alignment=" << id;

    return Qt::Alignment(id);
}

PrintOptionsPage::ScaleMode PrintOptionsPage::scaleMode() const
{
    return PrintOptionsPage::ScaleMode(d->mScaleGroup.checkedId());
}

bool PrintOptionsPage::enlargeSmallerImages() const
{
    return d->kcfg_PrintEnlargeSmallerImages->isChecked();
}

PrintOptionsPage::Unit PrintOptionsPage::scaleUnit() const
{
    return PrintOptionsPage::Unit(d->kcfg_PrintUnit->currentIndex());
}

double PrintOptionsPage::scaleWidth() const
{
    return d->kcfg_PrintWidth->value() * unitToInches(scaleUnit());
}

double PrintOptionsPage::scaleHeight() const
{
    return d->kcfg_PrintHeight->value() * unitToInches(scaleUnit());
}

void PrintOptionsPage::adjustWidthToRatio()
{
    if (!d->kcfg_PrintKeepRatio->isChecked())
    {
        return;
    }

    double width = d->mImageSize.width() * d->kcfg_PrintHeight->value() /
                   d->mImageSize.height();

    SignalBlocker blocker(d->kcfg_PrintWidth);
    d->kcfg_PrintWidth->setValue(width ? width : 1.);
}

void PrintOptionsPage::adjustHeightToRatio()
{
    if (!d->kcfg_PrintKeepRatio->isChecked())
    {
        return;
    }

    double height = d->mImageSize.height() * d->kcfg_PrintWidth->value() /
                    d->mImageSize.width();

    SignalBlocker blocker(d->kcfg_PrintHeight);
    d->kcfg_PrintHeight->setValue(height ? height : 1.0);
}

void PrintOptionsPage::loadConfig()
{
    QAbstractButton* button = d->mPositionGroup.button(PrintConfig::printPosition());

    if (button)
    {
        button->setChecked(true);
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown button for position group";
    }

    button = d->mScaleGroup.button(PrintConfig::printScaleMode());

    if (button)
    {
        button->setChecked(true);
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown button for scale group";
    }

    d->mConfigDialogManager->updateWidgets();

    if (d->kcfg_PrintKeepRatio->isChecked())
    {
        adjustHeightToRatio();
    }

    d->colorManaged->setChecked(PrintConfig::printColorManaged());
    ICCSettingsContainer settings = IccSettings::instance()->settings();
    d->outputProfile              = settings.defaultProofProfile;
    d->cmEnabled                  = settings.enableCM;
}

void PrintOptionsPage::saveConfig()
{
    int position        = d->mPositionGroup.checkedId();
    PrintConfig::setPrintPosition(position);

    ScaleMode scaleMode = ScaleMode(d->mScaleGroup.checkedId());
    PrintConfig::setPrintScaleMode(scaleMode);

    d->mConfigDialogManager->updateSettings();

    PrintConfig::self()->save();
}

void PrintOptionsPage::slotAlertSettings(bool t)
{
    if (t && !d->cmEnabled)
    {
        QString message = i18n("<p>Color Management is disabled.</p> "
                               "<p>You can enable it now by clicking "
                               "on the \"Settings\" button.</p>");
        QMessageBox::information(this, qApp->applicationName(), message);
        d->colorManaged->setChecked(!t);
    }
}

void PrintOptionsPage::slotSetupDlg()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(d->mParent);

    if (editor)
        editor->slotSetupICC();
}

} // namespace DigiKam
