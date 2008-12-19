/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-14
 * Description : main digiKam theme designer window
 *
 * Copyright (C) 2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "mainwindow.h"
#include "mainwindow.moc"

// Qt includes.

#include <QCheckBox>
#include <QDateTime>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QTextStream>
#include <QVBoxLayout>

// KDE includes.

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kcolordialog.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kuser.h>

// Local includes.

#include "albumsettings.h"
#include "folderview.h"
#include "folderitem.h"
#include "themediconview.h"
#include "imagepropertiestab.h"
#include "themeengine.h"
#include "theme.h"

namespace Digikam
{

MainWindow::MainWindow()
          : QWidget(0)
{
    setWindowTitle(i18n("digiKam Theme Designer"));
    setAttribute(Qt::WA_DeleteOnClose);

    AlbumSettings::instance();
    AlbumSettings::instance()->readSettings();

    // Initialize theme engine ------------------------------------

    ThemeEngine::instance()->scanThemes();
    m_theme = new Theme(*(ThemeEngine::instance()->getCurrentTheme()));

    // Actual views ------------------------------------------------

    QGridLayout* layout = new QGridLayout(this);

    QSplitter* splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);
    splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    m_folderView = new FolderView(splitter);
    m_iconView   = new ThemedIconView(splitter);
    m_propView   = new ImagePropertiesTab(splitter);

    // Property Editor ---------------------------------------------

    QGroupBox *groupBox = new QGroupBox(this);
    QVBoxLayout* vlay   = new QVBoxLayout(groupBox);

    QLabel* label1  = new QLabel(i18n("Property: "), groupBox);
    m_propertyCombo = new KComboBox(groupBox);

    m_bevelLabel = new QLabel(i18n("Bevel: "), groupBox);
    m_bevelCombo = new KComboBox(groupBox);

    m_gradientLabel = new QLabel(i18n("Gradient: "), groupBox);
    m_gradientCombo = new KComboBox(groupBox);

    m_begColorLabel = new QLabel(i18n("Start Color: "), groupBox);
    m_begColorBtn   = new KColorButton(groupBox);

    m_endColorLabel = new QLabel(i18n("End Color: "), groupBox);
    m_endColorBtn   = new KColorButton(groupBox);

    m_addBorderCheck = new QCheckBox(i18n("Add Border"), groupBox);

    m_borderColorLabel = new QLabel(i18n("Border Color: "), groupBox);
    m_borderColorBtn   = new KColorButton(groupBox);

    vlay->setAlignment(Qt::AlignTop);
    vlay->setSpacing(5);
    vlay->setMargin(5);
    vlay->addWidget(label1);
    vlay->addWidget(m_propertyCombo);
    vlay->addWidget(m_bevelLabel);
    vlay->addWidget(m_bevelCombo);
    vlay->addWidget(m_gradientLabel);
    vlay->addWidget(m_gradientCombo);
    vlay->addWidget(m_begColorLabel);
    vlay->addWidget(m_begColorBtn);
    vlay->addWidget(m_endColorLabel);
    vlay->addWidget(m_endColorBtn);
    vlay->addWidget( m_addBorderCheck );
    vlay->addWidget(m_borderColorLabel);
    vlay->addWidget(m_borderColorBtn);
    vlay->addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // -------------------------------------------------------------

    m_propertyCombo->insertItem(BASE, "Base");
    m_propertyCombo->insertItem(REGULARTEXT, "Regular Text");
    m_propertyCombo->insertItem(SELECTEDTEXT, "Selected Text");
    m_propertyCombo->insertItem(REGULARSPECIALTEXT, "Special Regular Text");
    m_propertyCombo->insertItem(SELECTEDSPECIALTEXT, "Special Selected Text");
    m_propertyCombo->insertItem(BANNER, "Banner");
    m_propertyCombo->insertItem(THUMBNAILREGULAR, "Thumbnail Regular");
    m_propertyCombo->insertItem(THUMBNAILSELECTED, "Thumbnail Selected");
    m_propertyCombo->insertItem(LISTVIEWREGULAR, "ListView Regular");
    m_propertyCombo->insertItem(LISTVIEWSELECTED, "ListView Selected");

    m_bevelCombo->insertItem(FLAT, "Flat");
    m_bevelCombo->insertItem(RAISED, "Raised");
    m_bevelCombo->insertItem(SUNKEN, "Sunken");

    m_gradientCombo->insertItem(SOLID, "Solid");
    m_gradientCombo->insertItem(HORIZONTAL, "Horizontal");
    m_gradientCombo->insertItem(VERTICAL, "Vertical");
    m_gradientCombo->insertItem(DIAGONAL, "Diagonal");

    m_bevelMap[FLAT]   = Theme::FLAT;
    m_bevelMap[RAISED] = Theme::RAISED;
    m_bevelMap[SUNKEN] = Theme::SUNKEN;

    m_gradientMap[SOLID]      = Theme::SOLID;
    m_gradientMap[HORIZONTAL] = Theme::HORIZONTAL;
    m_gradientMap[VERTICAL]   = Theme::VERTICAL;
    m_gradientMap[DIAGONAL]   = Theme::DIAGONAL;

    m_bevelReverseMap[Theme::FLAT]   = FLAT;
    m_bevelReverseMap[Theme::RAISED] = RAISED;
    m_bevelReverseMap[Theme::SUNKEN] = SUNKEN;

    m_gradientReverseMap[Theme::SOLID]      = SOLID;
    m_gradientReverseMap[Theme::HORIZONTAL] = HORIZONTAL;
    m_gradientReverseMap[Theme::VERTICAL]   = VERTICAL;
    m_gradientReverseMap[Theme::DIAGONAL]   = DIAGONAL;

    m_begColorBtn->setColor(Qt::black);
    m_endColorBtn->setColor(Qt::black);
    m_borderColorBtn->setColor(Qt::black);

    // Bottom button bar -------------------------------------------------------

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    QPushButton* loadButton = new QPushButton(this);
    loadButton->setText(i18n("&Load"));

    QPushButton* saveButton = new QPushButton(this);
    saveButton->setText(i18n("&Save"));

    QPushButton* closeButton = new QPushButton(this);
    closeButton->setText(i18n("&Close"));

    buttonLayout->setMargin(5);
    buttonLayout->setSpacing(5);
    buttonLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(closeButton);

    // ------------------------------------------------------------------------

    layout->setMargin(5);
    layout->setSpacing(5);
    layout->addWidget(splitter,     0, 0, 1, 1);
    layout->addWidget(groupBox,     0, 1, 1, 1);
    layout->addLayout(buttonLayout, 1, 0, 1, 2);

    // ------------------------------------------------------------------------

    connect(m_propertyCombo, SIGNAL(activated(int)),
            this, SLOT(slotPropertyChanged()));

    connect(m_bevelCombo, SIGNAL(activated(int)),
            this, SLOT(slotUpdateTheme()));

    connect(m_gradientCombo, SIGNAL(activated(int)),
            this, SLOT(slotUpdateTheme()));

    connect(m_begColorBtn, SIGNAL(changed(const QColor&)),
            this, SLOT(slotUpdateTheme()));

    connect(m_endColorBtn, SIGNAL(changed(const QColor&)),
            this, SLOT(slotUpdateTheme()));

    connect(m_addBorderCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdateTheme()));

    connect(m_borderColorBtn, SIGNAL(changed(const QColor&)),
            this, SLOT(slotUpdateTheme()));

    connect(loadButton, SIGNAL(clicked()),
            this, SLOT(slotLoad()));

    connect(saveButton, SIGNAL(clicked()),
            this, SLOT(slotSave()));

    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

    // ------------------------------------------------------------------------

    m_folderView->addColumn("My Albums");
    m_folderView->setResizeMode(Q3ListView::LastColumn);
    m_folderView->setRootIsDecorated(true);

    KIconLoader *iconLoader = KIconLoader::global();
    for (int i=0; i<10; i++)
    {
        FolderItem* folderItem = new FolderItem(m_folderView, QString("Album %1").arg(i));
        folderItem->setPixmap(0, iconLoader->loadIcon("folder", KIconLoader::NoGroup, 32));
        if (i == 2)
        {
            m_folderView->setSelected(folderItem, true);
        }
    }

    // ------------------------------------------------------------------------

    slotPropertyChanged();
    slotUpdateTheme();
}

MainWindow::~MainWindow()
{
}

void MainWindow::slotLoad()
{
    QString path = KFileDialog::getOpenFileName(KUrl(), QString(), this, QString());
    if (path.isEmpty())
        return;

    QFileInfo fi(path);
    m_theme->name     = fi.fileName();
    m_theme->filePath = path;

    ThemeEngine::instance()->setCurrentTheme(*m_theme, m_theme->name, true);
    *m_theme = *(ThemeEngine::instance()->getCurrentTheme());
    slotPropertyChanged();
}

void MainWindow::slotSave()
{
    QString path = KFileDialog::getSaveFileName(KUrl(), QString(), this, QString());
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(IO_WriteOnly))
    {
        KMessageBox::error(this, i18n("Failed to open file for writing"));
        return;
    }

    QFileInfo fi(path);
    m_theme->name     = fi.fileName();
    m_theme->filePath = path;

    ThemeEngine::instance()->setCurrentTheme(*m_theme, m_theme->name, false);
    ThemeEngine::instance()->saveTheme();
}

void MainWindow::slotPropertyChanged()
{
    m_bevelCombo->blockSignals(true);
    m_gradientCombo->blockSignals(true);
    m_begColorBtn->blockSignals(true);
    m_endColorBtn->blockSignals(true);
    m_addBorderCheck->blockSignals(true);
    m_borderColorBtn->blockSignals(true);

    m_bevelCombo->setEnabled(false);
    m_bevelLabel->setEnabled(false);
    m_gradientCombo->setEnabled(false);
    m_gradientLabel->setEnabled(false);
    m_begColorBtn->setEnabled(false);
    m_begColorLabel->setEnabled(false);
    m_endColorBtn->setEnabled(false);
    m_endColorLabel->setEnabled(false);
    m_addBorderCheck->setEnabled(false);
    m_borderColorBtn->setEnabled(false);
    m_borderColorLabel->setEnabled(false);

    switch(m_propertyCombo->currentIndex())
    {
        case(BASE):
        {
            m_begColorLabel->setEnabled(true);
            m_begColorBtn->setEnabled(true);
            m_begColorBtn->setColor(m_theme->baseColor);
            break;
        }
        case(REGULARTEXT):
        {
            m_begColorLabel->setEnabled(true);
            m_begColorBtn->setEnabled(true);
            m_begColorBtn->setColor(m_theme->textRegColor);
            break;
        }
        case(SELECTEDTEXT):
        {
            m_begColorLabel->setEnabled(true);
            m_begColorBtn->setEnabled(true);
            m_begColorBtn->setColor(m_theme->textSelColor);
            break;
        }
        case(REGULARSPECIALTEXT):
        {
            m_begColorLabel->setEnabled(true);
            m_begColorBtn->setEnabled(true);
            m_begColorBtn->setColor(m_theme->textSpecialRegColor);
            break;
        }
        case(SELECTEDSPECIALTEXT):
        {
            m_begColorLabel->setEnabled(true);
            m_begColorBtn->setEnabled(true);
            m_begColorBtn->setColor(m_theme->textSpecialSelColor);
            break;
        }
        case(BANNER):
        {
            m_bevelCombo->setEnabled(true);
            m_gradientCombo->setEnabled(true);
            m_begColorBtn->setEnabled(true);
            m_endColorBtn->setEnabled(true);
            m_addBorderCheck->setEnabled(true);
            m_borderColorBtn->setEnabled(true);
            m_bevelLabel->setEnabled(true);
            m_gradientLabel->setEnabled(true);
            m_begColorLabel->setEnabled(true);
            m_endColorLabel->setEnabled(true);
            m_borderColorLabel->setEnabled(true);

            m_bevelCombo->setCurrentIndex(m_bevelReverseMap[m_theme->bannerBevel]);
            m_gradientCombo->setCurrentIndex(m_gradientReverseMap[m_theme->bannerGrad]);

            m_begColorBtn->setColor(m_theme->bannerColor);
            m_endColorBtn->setColor(m_theme->bannerColorTo);

            m_addBorderCheck->setChecked(m_theme->bannerBorder);
            m_borderColorBtn->setColor(m_theme->bannerBorderColor);

            break;
        }
        case(THUMBNAILREGULAR):
        {
            m_bevelCombo->setEnabled(true);
            m_gradientCombo->setEnabled(true);
            m_begColorBtn->setEnabled(true);
            m_endColorBtn->setEnabled(true);
            m_addBorderCheck->setEnabled(true);
            m_borderColorBtn->setEnabled(true);
            m_bevelLabel->setEnabled(true);
            m_gradientLabel->setEnabled(true);
            m_begColorLabel->setEnabled(true);
            m_endColorLabel->setEnabled(true);
            m_borderColorLabel->setEnabled(true);

            m_bevelCombo->setCurrentIndex(m_bevelReverseMap[m_theme->thumbRegBevel]);
            m_gradientCombo->setCurrentIndex(m_gradientReverseMap[m_theme->thumbRegGrad]);

            m_begColorBtn->setColor(m_theme->thumbRegColor);
            m_endColorBtn->setColor(m_theme->thumbRegColorTo);

            m_addBorderCheck->setChecked(m_theme->thumbRegBorder);
            m_borderColorBtn->setColor(m_theme->thumbRegBorderColor);

            break;
        }
        case(THUMBNAILSELECTED):
        {
            m_bevelCombo->setEnabled(true);
            m_gradientCombo->setEnabled(true);
            m_begColorBtn->setEnabled(true);
            m_endColorBtn->setEnabled(true);
            m_addBorderCheck->setEnabled(true);
            m_borderColorBtn->setEnabled(true);
            m_bevelLabel->setEnabled(true);
            m_gradientLabel->setEnabled(true);
            m_begColorLabel->setEnabled(true);
            m_endColorLabel->setEnabled(true);
            m_borderColorLabel->setEnabled(true);

            m_bevelCombo->setCurrentIndex(m_bevelReverseMap[m_theme->thumbSelBevel]);
            m_gradientCombo->setCurrentIndex(m_gradientReverseMap[m_theme->thumbSelGrad]);

            m_begColorBtn->setColor(m_theme->thumbSelColor);
            m_endColorBtn->setColor(m_theme->thumbSelColorTo);

            m_addBorderCheck->setChecked(m_theme->thumbSelBorder);
            m_borderColorBtn->setColor(m_theme->thumbSelBorderColor);

            break;
        }
        case(LISTVIEWREGULAR):
        {
            m_bevelCombo->setEnabled(true);
            m_gradientCombo->setEnabled(true);
            m_begColorBtn->setEnabled(true);
            m_endColorBtn->setEnabled(true);
            m_addBorderCheck->setEnabled(true);
            m_borderColorBtn->setEnabled(true);
            m_bevelLabel->setEnabled(true);
            m_gradientLabel->setEnabled(true);
            m_begColorLabel->setEnabled(true);
            m_endColorLabel->setEnabled(true);
            m_borderColorLabel->setEnabled(true);

            m_bevelCombo->setCurrentIndex(m_bevelReverseMap[m_theme->listRegBevel]);
            m_gradientCombo->setCurrentIndex(m_gradientReverseMap[m_theme->listRegGrad]);

            m_begColorBtn->setColor(m_theme->listRegColor);
            m_endColorBtn->setColor(m_theme->listRegColorTo);

            m_addBorderCheck->setChecked(m_theme->listRegBorder);
            m_borderColorBtn->setColor(m_theme->listRegBorderColor);

            break;
        }
        case(LISTVIEWSELECTED):
        {
            m_bevelCombo->setEnabled(true);
            m_gradientCombo->setEnabled(true);
            m_begColorBtn->setEnabled(true);
            m_endColorBtn->setEnabled(true);
            m_addBorderCheck->setEnabled(true);
            m_borderColorBtn->setEnabled(true);
            m_bevelLabel->setEnabled(true);
            m_gradientLabel->setEnabled(true);
            m_begColorLabel->setEnabled(true);
            m_endColorLabel->setEnabled(true);
            m_borderColorLabel->setEnabled(true);

            m_bevelCombo->setCurrentIndex(m_bevelReverseMap[m_theme->listSelBevel]);
            m_gradientCombo->setCurrentIndex(m_gradientReverseMap[m_theme->listSelGrad]);

            m_begColorBtn->setColor(m_theme->listSelColor);
            m_endColorBtn->setColor(m_theme->listSelColorTo);

            m_addBorderCheck->setChecked(m_theme->listSelBorder);
            m_borderColorBtn->setColor(m_theme->listSelBorderColor);

            break;
        }
    };

    m_bevelCombo->blockSignals(false);
    m_gradientCombo->blockSignals(false);
    m_begColorBtn->blockSignals(false);
    m_endColorBtn->blockSignals(false);
    m_addBorderCheck->blockSignals(false);
    m_borderColorBtn->blockSignals(false);
}

void MainWindow::slotUpdateTheme()
{
    switch(m_propertyCombo->currentIndex())
    {
        case(BASE):
        {
            m_theme->baseColor = m_begColorBtn->color();
            break;
        }
        case(REGULARTEXT):
        {
            m_theme->textRegColor = m_begColorBtn->color();
            break;
        }
        case(SELECTEDTEXT):
        {
            m_theme->textSelColor = m_begColorBtn->color();
            break;
        }
        case(REGULARSPECIALTEXT):
        {
            m_theme->textSpecialRegColor = m_begColorBtn->color();
            break;
        }
        case(SELECTEDSPECIALTEXT):
        {
            m_theme->textSpecialSelColor = m_begColorBtn->color();
            break;
        }
        case(BANNER):
        {
            m_theme->bannerBevel = (Theme::Bevel) m_bevelMap[m_bevelCombo->currentIndex()];
            m_theme->bannerGrad  = (Theme::Gradient) m_gradientMap[m_gradientCombo->currentIndex()];

            m_theme->bannerColor   = m_begColorBtn->color();
            m_theme->bannerColorTo = m_endColorBtn->color();

            m_theme->bannerBorder      = m_addBorderCheck->isChecked();
            m_theme->bannerBorderColor = m_borderColorBtn->color();

            break;
        }
        case(THUMBNAILREGULAR):
        {
            m_theme->thumbRegBevel = (Theme::Bevel) m_bevelMap[m_bevelCombo->currentIndex()];
            m_theme->thumbRegGrad  = (Theme::Gradient) m_gradientMap[m_gradientCombo->currentIndex()];

            m_theme->thumbRegColor   = m_begColorBtn->color();
            m_theme->thumbRegColorTo = m_endColorBtn->color();

            m_theme->thumbRegBorder      = m_addBorderCheck->isChecked();
            m_theme->thumbRegBorderColor = m_borderColorBtn->color();

            break;
        }
        case(THUMBNAILSELECTED):
        {
            m_theme->thumbSelBevel = (Theme::Bevel) m_bevelMap[m_bevelCombo->currentIndex()];
            m_theme->thumbSelGrad  = (Theme::Gradient) m_gradientMap[m_gradientCombo->currentIndex()];

            m_theme->thumbSelColor   = m_begColorBtn->color();
            m_theme->thumbSelColorTo = m_endColorBtn->color();

            m_theme->thumbSelBorder      = m_addBorderCheck->isChecked();
            m_theme->thumbSelBorderColor = m_borderColorBtn->color();

            break;
        }
        case(LISTVIEWREGULAR):
        {
            m_theme->listRegBevel = (Theme::Bevel) m_bevelMap[m_bevelCombo->currentIndex()];
            m_theme->listRegGrad  = (Theme::Gradient) m_gradientMap[m_gradientCombo->currentIndex()];

            m_theme->listRegColor   = m_begColorBtn->color();
            m_theme->listRegColorTo = m_endColorBtn->color();

            m_theme->listRegBorder      = m_addBorderCheck->isChecked();
            m_theme->listRegBorderColor = m_borderColorBtn->color();

            break;
        }
        case(LISTVIEWSELECTED):
        {
            m_theme->listSelBevel = (Theme::Bevel) m_bevelMap[m_bevelCombo->currentIndex()];
            m_theme->listSelGrad  = (Theme::Gradient) m_gradientMap[m_gradientCombo->currentIndex()];

            m_theme->listSelColor   = m_begColorBtn->color();
            m_theme->listSelColorTo = m_endColorBtn->color();

            m_theme->listSelBorder      = m_addBorderCheck->isChecked();
            m_theme->listSelBorderColor = m_borderColorBtn->color();

            break;
        }
    };

    ThemeEngine::instance()->setCurrentTheme(*m_theme, "Digikam ThemeEditor Theme");
}

}  // namespace Digikam
