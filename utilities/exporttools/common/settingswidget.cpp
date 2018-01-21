/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-28
 * Description : Common widgets shared by export tools
 *
 * Copyright (C) 2013 by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2015 by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "settingswidget.h"

// Qt includes

#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QComboBox>
#include <QScrollArea>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class SettingsWidget::Private
{
public:

    Private(QWidget* const widget, const QString& toolName)
    {
        m_toolName           = toolName;
        mainLayout           = new QHBoxLayout(widget);
        m_imgList            = new DImagesList(widget);
        settingsScrollArea   = new QScrollArea(widget);
        m_settingsBox        = new QWidget(settingsScrollArea);
        m_settingsBoxLayout  = new QVBoxLayout(m_settingsBox);
        m_headerLbl          = new QLabel(widget);
        m_accountBox         = new QGroupBox(i18n("Account"),m_settingsBox);
        m_accountBoxLayout   = new QGridLayout(m_accountBox);
        m_userNameDisplayLbl = new QLabel(m_accountBox);
        m_changeUserBtn      = new QPushButton(m_accountBox);
        m_albBox             = new QGroupBox(i18n("Album"),m_settingsBox);
        m_albumsBoxLayout    = new QGridLayout(m_albBox);
        m_albumsCoB          = new QComboBox(m_albBox);
        m_newAlbumBtn        = new QPushButton(m_accountBox);
        m_reloadAlbumsBtn    = new QPushButton(m_accountBox);
        m_sizeBox            = new QGroupBox(i18n("Max Dimension"), m_settingsBox);
        m_sizeBoxLayout      = new QVBoxLayout(m_sizeBox);
        m_dlDimensionCoB     = new QComboBox(m_sizeBox);
        m_optionsBox         = new QGroupBox(i18n("Options"),m_settingsBox);
        m_optionsBoxLayout   = new QGridLayout(m_optionsBox);
        m_originalChB        = new QCheckBox(m_optionsBox);
        m_resizeChB          = new QCheckBox(m_optionsBox);
        m_dimensionSpB       = new QSpinBox(m_optionsBox);
        m_imageQualitySpB    = new QSpinBox(m_optionsBox);
        m_progressBar        = new DProgressWdg(m_settingsBox);

    }

    DImagesList*                   m_imgList;
    QString                        m_toolName;

    QLabel*                        m_headerLbl;
    QLabel*                        m_userNameDisplayLbl;
    QPushButton*                   m_changeUserBtn;
    QComboBox*                     m_dlDimensionCoB;
    QScrollArea*                   settingsScrollArea;

    QComboBox*                     m_albumsCoB;
    QPushButton*                   m_newAlbumBtn;
    QPushButton*                   m_reloadAlbumsBtn;

    QCheckBox*                     m_originalChB;
    QCheckBox*                     m_resizeChB;
    QSpinBox*                      m_dimensionSpB;
    QSpinBox*                      m_imageQualitySpB;

    QHBoxLayout*                   mainLayout;

    QWidget*                       m_settingsBox;
    QVBoxLayout*                   m_settingsBoxLayout;

    QGroupBox*                     m_albBox;
    QGridLayout*                   m_albumsBoxLayout;

    QGroupBox*                     m_optionsBox;
    QGridLayout*                   m_optionsBoxLayout;

    QGroupBox*                     m_sizeBox;
    QVBoxLayout*                   m_sizeBoxLayout;

    QGroupBox*                     m_accountBox;
    QGridLayout*                   m_accountBoxLayout;

    DProgressWdg*                  m_progressBar;
};

SettingsWidget::SettingsWidget(QWidget* const parent, const QString& toolName)
    : QWidget(parent),
      d(new Private(this, toolName))
{
    setObjectName(d->m_toolName + QString::fromLatin1(" Widget"));

    //----------------------------------------------------------

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->m_imgList->setControlButtonsPlacement(DImagesList::ControlButtonsBelow);
    d->m_imgList->setAllowRAW(true);
    d->m_imgList->listView()->setWhatsThis(i18n("This is the list of images to upload to your %1 account.", d->m_toolName));

    d->settingsScrollArea->setMinimumSize(400,500);
    d->settingsScrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->settingsScrollArea->setWidget(d->m_settingsBox);
    d->settingsScrollArea->setWidgetResizable(true);
    d->settingsScrollArea->setFrameShadow(QFrame::Plain);

    d->m_headerLbl->setWhatsThis(i18n("This is a clickable link to open %1 in a browser.", d->m_toolName));
    d->m_headerLbl->setOpenExternalLinks(true);
    d->m_headerLbl->setFocusPolicy(Qt::NoFocus);

    //------------------------------------------------------------

    d->m_accountBox->setWhatsThis(i18n("This is the %1 account that is currently logged in.", d->m_toolName));

    QLabel* const userNameLbl = new QLabel(i18nc("account settings","Name:"), d->m_accountBox);
    d->m_changeUserBtn->setText(i18n("Change Account"));
    d->m_changeUserBtn->setIcon(QIcon::fromTheme(QString::fromLatin1("system-switch-user")).pixmap(16));
    d->m_changeUserBtn->setToolTip(i18n("Change %1 account for transfer", d->m_toolName));

    d->m_accountBoxLayout->addWidget(userNameLbl,             0, 0, 1, 2);
    d->m_accountBoxLayout->addWidget(d->m_userNameDisplayLbl, 0, 2, 1, 2);
    d->m_accountBoxLayout->addWidget(d->m_changeUserBtn,      1, 0, 1, 4);
    d->m_accountBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    d->m_accountBoxLayout->setSpacing(spacing);

    //-------------------------------------------------------------

    d->m_albBox->setWhatsThis(i18n("This is the %1 folder to/from which selected photos will be uploaded/downloaded.", d->m_toolName));

    QLabel* const albLbl = new QLabel(i18n("Album:"), d->m_albBox);

    d->m_albumsCoB->setEditable(false);

    d->m_newAlbumBtn->setText(i18n("New Album"));
    d->m_newAlbumBtn->setIcon(QIcon::fromTheme(QString::fromLatin1("list-add")).pixmap(16));
    d->m_newAlbumBtn->setToolTip(i18n("Create new %1 folder", d->m_toolName));

    d->m_reloadAlbumsBtn->setText(i18nc("album list","Reload"));
    d->m_reloadAlbumsBtn->setIcon(QIcon::fromTheme(QString::fromLatin1("view-refresh")).pixmap(16));
    d->m_reloadAlbumsBtn->setToolTip(i18n("Reload album list"));

    d->m_albumsBoxLayout->addWidget(albLbl,               0, 0, 1, 1);
    d->m_albumsBoxLayout->addWidget(d->m_albumsCoB,       0, 1, 1, 4);
    d->m_albumsBoxLayout->addWidget(d->m_newAlbumBtn,     1, 3, 1, 1);
    d->m_albumsBoxLayout->addWidget(d->m_reloadAlbumsBtn, 1, 4, 1, 1);

    //----------------------------------------------------------

    d->m_sizeBox->setWhatsThis(i18n("This is the maximum dimension of the images. Images larger than this will be scaled down."));
    d->m_dlDimensionCoB->addItem(i18n("Original Size"), QString::fromLatin1("d"));
    d->m_dlDimensionCoB->addItem(i18n("1600 px"), QString::fromLatin1("1600"));
    d->m_dlDimensionCoB->addItem(i18n("1440 px"), QString::fromLatin1("1440"));
    d->m_dlDimensionCoB->addItem(i18n("1280 px"), QString::fromLatin1("1280"));
    d->m_dlDimensionCoB->addItem(i18n("1152 px"), QString::fromLatin1("1152"));
    d->m_dlDimensionCoB->addItem(i18n("1024 px"), QString::fromLatin1("1024"));
    d->m_dlDimensionCoB->setCurrentIndex(0);
    d->m_sizeBoxLayout->addWidget(d->m_dlDimensionCoB);

    //-----------------------------------------------------------

    d->m_optionsBox->setWhatsThis(i18n("These are the options that would be applied to photos before upload."));

    d->m_originalChB->setText(i18n("Upload original image file"));
    d->m_originalChB->setChecked(false);
    d->m_originalChB->hide();

    d->m_resizeChB->setText(i18n("Resize photos before uploading"));
    d->m_resizeChB->setChecked(false);

    d->m_dimensionSpB->setMinimum(0);
    d->m_dimensionSpB->setMaximum(5000);
    d->m_dimensionSpB->setSingleStep(10);
    d->m_dimensionSpB->setValue(1600);
    d->m_dimensionSpB->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    d->m_dimensionSpB->setEnabled(false);

    QLabel* const dimensionLbl = new QLabel(i18n("Maximum Dimension:"), d->m_optionsBox);

    d->m_imageQualitySpB->setMinimum(0);
    d->m_imageQualitySpB->setMaximum(100);
    d->m_imageQualitySpB->setSingleStep(1);
    d->m_imageQualitySpB->setValue(90);
    d->m_imageQualitySpB->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    QLabel* const imageQualityLbl = new QLabel(i18n("JPEG Quality:"), d->m_optionsBox);

    d->m_optionsBoxLayout->addWidget(d->m_originalChB,     0, 0, 1, 5);
    d->m_optionsBoxLayout->addWidget(d->m_resizeChB,       1, 0, 1, 5);
    d->m_optionsBoxLayout->addWidget(imageQualityLbl,      2, 1, 1, 1);
    d->m_optionsBoxLayout->addWidget(d->m_imageQualitySpB, 2, 2, 1, 1);
    d->m_optionsBoxLayout->addWidget(dimensionLbl,         3, 1, 1, 1);
    d->m_optionsBoxLayout->addWidget(d->m_dimensionSpB,    3, 2, 1, 1);
    d->m_optionsBoxLayout->setRowStretch(4, 10);
    d->m_optionsBoxLayout->setSpacing(spacing);
    d->m_optionsBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);

    d->m_progressBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    d->m_progressBar->hide();

    //------------------------------------------------------

    d->m_settingsBoxLayout->addWidget(d->m_headerLbl);
    d->m_settingsBoxLayout->addWidget(d->m_accountBox);
    d->m_settingsBoxLayout->addWidget(d->m_albBox);
    d->m_settingsBoxLayout->addWidget(d->m_sizeBox);
    d->m_settingsBoxLayout->addWidget(d->m_optionsBox);
    d->m_settingsBoxLayout->addWidget(d->m_progressBar);
    d->m_settingsBoxLayout->setSpacing(spacing);
    d->m_settingsBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);

    //--------------------------------------------------------

    d->mainLayout->addWidget(d->m_imgList);
    d->mainLayout->addWidget(d->settingsScrollArea);
    d->mainLayout->setContentsMargins(QMargins());
    d->mainLayout->setSpacing(spacing);

    //-------------------------------------------------------

    connect(d->m_originalChB, SIGNAL(toggled(bool)),
            this, SLOT(slotResizeChecked()));

    connect(d->m_resizeChB, SIGNAL(toggled(bool)),
            this, SLOT(slotResizeChecked()));
}

SettingsWidget::~SettingsWidget()
{
    delete d;
}

DImagesList* SettingsWidget::imagesList() const
{
    return d->m_imgList;
}

void SettingsWidget::slotResizeChecked()
{
    d->m_resizeChB->setEnabled(!d->m_originalChB->isChecked());
    d->m_imageQualitySpB->setEnabled(!d->m_originalChB->isChecked());
    d->m_dimensionSpB->setEnabled(d->m_resizeChB->isChecked() && !d->m_originalChB->isChecked());
}

DProgressWdg* SettingsWidget::progressBar() const
{
    return d->m_progressBar;
}

void SettingsWidget::addWidgetToSettingsBox(QWidget* const widget)
{
    d->m_settingsBoxLayout->addWidget(widget);
    d->m_settingsBoxLayout->removeWidget(d->m_progressBar); // NOTE: This is important because progress bar always has to be at the end of settings box layout. So we remove it and then add it back.
    d->m_settingsBoxLayout->addWidget(d->m_progressBar);
}

void SettingsWidget::replaceImageList(QWidget* const imgList)
{
    d->m_imgList->hide();
    d->mainLayout->removeWidget(d->m_imgList);
    d->mainLayout->insertWidget(0, imgList);
}

QWidget* SettingsWidget::getSettingsBox() const
{
    return d->m_settingsBox;
}

QVBoxLayout* SettingsWidget::getSettingsBoxLayout() const
{
    return d->m_settingsBoxLayout;
}

QGroupBox* SettingsWidget::getAlbumBox() const
{
    return d->m_albBox;
}

QGridLayout* SettingsWidget::getAlbumBoxLayout() const
{
    return d->m_albumsBoxLayout;
}

QGroupBox* SettingsWidget::getOptionsBox() const
{
    return d->m_optionsBox;
}

QGridLayout* SettingsWidget::getOptionsBoxLayout() const
{
    return d->m_optionsBoxLayout;
}

QGroupBox* SettingsWidget::getSizeBox() const
{
    return d->m_sizeBox;
}

QVBoxLayout* SettingsWidget::getSizeBoxLayout() const
{
    return d->m_sizeBoxLayout;
}

QGroupBox* SettingsWidget::getAccountBox() const
{
    return d->m_accountBox;
}

QGridLayout* SettingsWidget::getAccountBoxLayout() const
{
    return d->m_accountBoxLayout;
}

QLabel* SettingsWidget::getHeaderLbl() const
{
    return d->m_headerLbl;
}

QLabel* SettingsWidget::getUserNameLabel() const
{
    return d->m_userNameDisplayLbl;
}

QPushButton* SettingsWidget::getChangeUserBtn() const
{
    return d->m_changeUserBtn;
}

QComboBox* SettingsWidget::getDimensionCoB() const
{
    return d->m_dlDimensionCoB;
}

QPushButton* SettingsWidget::getNewAlbmBtn() const
{
    return d->m_newAlbumBtn;
}

QPushButton* SettingsWidget::getReloadBtn() const
{
    return d->m_reloadAlbumsBtn;
}

QCheckBox* SettingsWidget::getOriginalCheckBox() const
{
    return d->m_originalChB;
}

QCheckBox* SettingsWidget::getResizeCheckBox() const
{
    return d->m_resizeChB;
}

QSpinBox* SettingsWidget::getDimensionSpB() const
{
    return d->m_dimensionSpB;
}

QSpinBox* SettingsWidget::getImgQualitySpB() const
{
    return d->m_imageQualitySpB;
}

QComboBox* SettingsWidget::getAlbumsCoB() const
{
    return d->m_albumsCoB;
}

} // namespace Digikam
