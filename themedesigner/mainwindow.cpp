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

// Qt includes.

#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qsplitter.h>
#include <qheader.h>
#include <qlayout.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qdatetime.h>

// KDE includes.

#include <klocale.h>
#include <kcolordialog.h>
#include <kcolorbutton.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kfiledialog.h>
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
#include "mainwindow.h"
#include "mainwindow.moc"

namespace Digikam
{

MainWindow::MainWindow()
          : QWidget(0, 0, WDestructiveClose)
{
    setCaption(i18n("digiKam Theme Designer"));

    AlbumSettings *albumSettings = new AlbumSettings();
    albumSettings->readSettings();

    // Initialize theme engine ------------------------------------

    ThemeEngine::instance()->scanThemes();
    m_theme = new Theme(*(ThemeEngine::instance()->getCurrentTheme()));

    // Actual views ------------------------------------------------

    QGridLayout* layout = new QGridLayout(this);

    QSplitter* splitter = new QSplitter(this);
    splitter->setOrientation( QSplitter::Horizontal );
    splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    m_folderView = new FolderView(splitter);
    m_iconView   = new ThemedIconView(splitter);
    m_propView   = new ImagePropertiesTab(splitter, false);

    // Property Editor ---------------------------------------------

    QGroupBox *groupBox = new QGroupBox(this);
    QVBoxLayout* vlay   = new QVBoxLayout(groupBox);

    QLabel* label1  = new QLabel("Property: ", groupBox);
    m_propertyCombo = new QComboBox(groupBox);

    m_bevelLabel = new QLabel("Bevel: ", groupBox);
    m_bevelCombo = new QComboBox(groupBox);

    m_gradientLabel = new QLabel("Gradient: ", groupBox);
    m_gradientCombo = new QComboBox(groupBox);

    m_begColorLabel = new QLabel("Start Color: ", groupBox);
    m_begColorBtn   = new KColorButton(groupBox);

    m_endColorLabel = new QLabel("End Color: ", groupBox);
    m_endColorBtn   = new KColorButton(groupBox);

    m_addBorderCheck = new QCheckBox("Add Border", groupBox);

    m_borderColorLabel = new QLabel("Border Color: ", groupBox);
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

    layout->setMargin(5);
    layout->setSpacing(5);
    layout->addWidget(splitter, 0, 0);
    layout->addWidget(groupBox, 0, 1);

    // -------------------------------------------------------------

    m_propertyCombo->insertItem( "Base",          BASE);
    m_propertyCombo->insertItem( "Regular Text",  REGULARTEXT);
    m_propertyCombo->insertItem( "Selected Text", SELECTEDTEXT);
    m_propertyCombo->insertItem( "Special Regular Text",  REGULARSPECIALTEXT);
    m_propertyCombo->insertItem( "Special Selected Text", SELECTEDSPECIALTEXT);
    m_propertyCombo->insertItem( "Banner",    BANNER);
    m_propertyCombo->insertItem( "Thumbnail Regular", THUMBNAILREGULAR);
    m_propertyCombo->insertItem( "Thumbnail Selected", THUMBNAILSELECTED);
    m_propertyCombo->insertItem( "ListView Regular",  LISTVIEWREGULAR);
    m_propertyCombo->insertItem( "ListView Selected",  LISTVIEWSELECTED);

    m_bevelCombo->insertItem( "Flat",  FLAT);
    m_bevelCombo->insertItem( "Raised", RAISED);
    m_bevelCombo->insertItem( "Sunken", SUNKEN );

    m_gradientCombo->insertItem( "Solid",      SOLID);
    m_gradientCombo->insertItem( "Horizontal", HORIZONTAL);
    m_gradientCombo->insertItem( "Vertical",   VERTICAL );
    m_gradientCombo->insertItem( "Diagonal",   DIAGONAL );

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

    // Bottom button bar -------------------------------------------------------

    QHBoxLayout* buttonLayout = new QHBoxLayout(0);
    buttonLayout->setMargin(5);
    buttonLayout->setSpacing(5);
    buttonLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding,
                                          QSizePolicy::Minimum)); 

    QPushButton* loadButton = new QPushButton( this );
    loadButton->setText( "&Load" );
    buttonLayout->addWidget( loadButton );

    QPushButton* saveButton = new QPushButton( this );
    saveButton->setText( "&Save" );
    buttonLayout->addWidget( saveButton );

    QPushButton* closeButton = new QPushButton( this );
    closeButton->setText( "&Close" );
    buttonLayout->addWidget( closeButton );

    layout->addMultiCellLayout(buttonLayout, 1, 1, 0, 1);

    connect(loadButton, SIGNAL(clicked()),
            this, SLOT(slotLoad()));
    connect(saveButton, SIGNAL(clicked()),
            this, SLOT(slotSave()));
    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

    // ------------------------------------------------------------------------

    m_folderView->addColumn("My Albums");
    m_folderView->setResizeMode(QListView::LastColumn);
    m_folderView->setRootIsDecorated(true);

    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();    
    for (int i=0; i<10; i++)
    {
        FolderItem* folderItem = new FolderItem(m_folderView, QString("Album %1").arg(i));
        folderItem->setPixmap(0, iconLoader->loadIcon("folder", KIcon::NoGroup,
                                                      32, KIcon::DefaultState, 0, true));
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
    QString path = KFileDialog::getOpenFileName(QString::null, QString::null,
                                                this);
    if (path.isEmpty())
        return;

    QFileInfo fi(path);
    m_theme->name = fi.fileName();
    m_theme->filePath = path;

    ThemeEngine::instance()->setCurrentTheme(*m_theme, m_theme->name, true);
    *m_theme = *(ThemeEngine::instance()->getCurrentTheme());
    slotPropertyChanged();
}

void MainWindow::slotSave()
{
    QString path = KFileDialog::getSaveFileName(QString::null, QString::null, this);
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(IO_WriteOnly))
    {
        KMessageBox::error(this, "Failed to open file for writing");
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

    switch(m_propertyCombo->currentItem())
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

        m_bevelCombo->setCurrentItem(m_bevelReverseMap[m_theme->bannerBevel]);
        m_gradientCombo->setCurrentItem(m_gradientReverseMap[m_theme->bannerGrad]);

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

        m_bevelCombo->setCurrentItem(m_bevelReverseMap[m_theme->thumbRegBevel]);
        m_gradientCombo->setCurrentItem(m_gradientReverseMap[m_theme->thumbRegGrad]);

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

        m_bevelCombo->setCurrentItem(m_bevelReverseMap[m_theme->thumbSelBevel]);
        m_gradientCombo->setCurrentItem(m_gradientReverseMap[m_theme->thumbSelGrad]);

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

        m_bevelCombo->setCurrentItem(m_bevelReverseMap[m_theme->listRegBevel]);
        m_gradientCombo->setCurrentItem(m_gradientReverseMap[m_theme->listRegGrad]);

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

        m_bevelCombo->setCurrentItem(m_bevelReverseMap[m_theme->listSelBevel]);
        m_gradientCombo->setCurrentItem(m_gradientReverseMap[m_theme->listSelGrad]);

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
    switch(m_propertyCombo->currentItem())
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
            m_theme->bannerBevel = (Theme::Bevel) m_bevelMap[m_bevelCombo->currentItem()];
            m_theme->bannerGrad  = (Theme::Gradient) m_gradientMap[m_gradientCombo->currentItem()];

            m_theme->bannerColor   = m_begColorBtn->color();
            m_theme->bannerColorTo = m_endColorBtn->color();

            m_theme->bannerBorder  = m_addBorderCheck->isChecked();
            m_theme->bannerBorderColor = m_borderColorBtn->color();

            break;
        }
        case(THUMBNAILREGULAR):
        {
            m_theme->thumbRegBevel = (Theme::Bevel) m_bevelMap[m_bevelCombo->currentItem()];
            m_theme->thumbRegGrad  = (Theme::Gradient) m_gradientMap[m_gradientCombo->currentItem()];

            m_theme->thumbRegColor   = m_begColorBtn->color();
            m_theme->thumbRegColorTo = m_endColorBtn->color();

            m_theme->thumbRegBorder  = m_addBorderCheck->isChecked();
            m_theme->thumbRegBorderColor = m_borderColorBtn->color();

            break;
        }
        case(THUMBNAILSELECTED):
        {
            m_theme->thumbSelBevel = (Theme::Bevel) m_bevelMap[m_bevelCombo->currentItem()];
            m_theme->thumbSelGrad  = (Theme::Gradient) m_gradientMap[m_gradientCombo->currentItem()];

            m_theme->thumbSelColor   = m_begColorBtn->color();
            m_theme->thumbSelColorTo = m_endColorBtn->color();

            m_theme->thumbSelBorder  = m_addBorderCheck->isChecked();
            m_theme->thumbSelBorderColor = m_borderColorBtn->color();

            break;
        }
        case(LISTVIEWREGULAR):
        {
            m_theme->listRegBevel = (Theme::Bevel) m_bevelMap[m_bevelCombo->currentItem()];
            m_theme->listRegGrad  = (Theme::Gradient) m_gradientMap[m_gradientCombo->currentItem()];

            m_theme->listRegColor   = m_begColorBtn->color();
            m_theme->listRegColorTo = m_endColorBtn->color();

            m_theme->listRegBorder  = m_addBorderCheck->isChecked();
            m_theme->listRegBorderColor = m_borderColorBtn->color();

            break;
        }
        case(LISTVIEWSELECTED):
        {
            m_theme->listSelBevel = (Theme::Bevel) m_bevelMap[m_bevelCombo->currentItem()];
            m_theme->listSelGrad  = (Theme::Gradient) m_gradientMap[m_gradientCombo->currentItem()];

            m_theme->listSelColor   = m_begColorBtn->color();
            m_theme->listSelColorTo = m_endColorBtn->color();

            m_theme->listSelBorder  = m_addBorderCheck->isChecked();
            m_theme->listSelBorderColor = m_borderColorBtn->color();

            break;
        }
    };

    ThemeEngine::instance()->setCurrentTheme(*m_theme, "Digikam ThemeEditor Theme");
}

}  // NameSpace Digikam
