/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-14
 * Description : main digiKam theme designer window
 *
 * Copyright (C) 2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QCheckBox>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>

// KDE includes

#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandardguiitem.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kurl.h>

// Local includes

#include "albumsettings.h"
#include "folderitem.h"
#include "folderview.h"
#include "imagepropertiestab.h"
#include "theme.h"
#include "themediconview.h"
#include "themeengine.h"

namespace Digikam
{

class MainWindowPriv
{
public:

    MainWindowPriv()
    {
        bevelLabel       = 0;
        gradientLabel    = 0;
        begColorLabel    = 0;
        endColorLabel    = 0;
        borderColorLabel = 0;
        propertyCombo    = 0;
        bevelCombo       = 0;
        gradientCombo    = 0;
        addBorderCheck   = 0;
        endColorBtn      = 0;
        begColorBtn      = 0;
        borderColorBtn   = 0;
        folderView       = 0;
        iconView         = 0;
        propView         = 0;
        theme            = 0;
    }

    QLabel*             bevelLabel;
    QLabel*             gradientLabel;
    QLabel*             begColorLabel;
    QLabel*             endColorLabel;
    QLabel*             borderColorLabel;

    QCheckBox*          addBorderCheck;

    QMap<int,int>       bevelMap;
    QMap<int,int>       bevelReverseMap;
    QMap<int,int>       gradientMap;
    QMap<int,int>       gradientReverseMap;

    KComboBox*          propertyCombo;
    KComboBox*          bevelCombo;
    KComboBox*          gradientCombo;

    KColorButton*       endColorBtn;
    KColorButton*       begColorBtn;
    KColorButton*       borderColorBtn;

    FolderView*         folderView;
    ThemedIconView*     iconView;
    ImagePropertiesTab* propView;
    Theme*              theme;
};

MainWindow::MainWindow()
          : KDialog(0), d(new MainWindowPriv)
{
    setWindowTitle(i18n("digiKam Theme Designer"));
    setAttribute(Qt::WA_DeleteOnClose);

    // --------------------------------------------------------

    setButtons(User1|User2|Close);
    setButtonGuiItem(User2, KStandardGuiItem::open());
    setButtonGuiItem(User1, KStandardGuiItem::save());
    setButtonToolTip(User2, i18n("Load theme"));
    setButtonToolTip(User1, i18n("Save theme"));
    setButtonToolTip(Close, i18n("Close the theme designer"));
    setDefaultButton(Close);

    // --------------------------------------------------------

    AlbumSettings::instance();
    AlbumSettings::instance()->readSettings();

    // Initialize theme engine ------------------------------------

    ThemeEngine::instance()->scanThemes();
    d->theme = new Theme(*(ThemeEngine::instance()->getCurrentTheme()));

    // Actual views ------------------------------------------------

    QSplitter* splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);
    splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    d->folderView = new FolderView(splitter);
    d->iconView   = new ThemedIconView(splitter);
    d->propView   = new ImagePropertiesTab(splitter);

    // Property Editor ---------------------------------------------

    QGroupBox *groupBox = new QGroupBox(this);
    QVBoxLayout* vlay   = new QVBoxLayout(groupBox);

    QLabel* label1   = new QLabel(i18n("Property: "), groupBox);
    d->propertyCombo = new KComboBox(groupBox);

    d->bevelLabel = new QLabel(i18n("Bevel: "), groupBox);
    d->bevelCombo = new KComboBox(groupBox);

    d->gradientLabel = new QLabel(i18n("Gradient: "), groupBox);
    d->gradientCombo = new KComboBox(groupBox);

    d->begColorLabel = new QLabel(i18n("Start Color: "), groupBox);
    d->begColorBtn   = new KColorButton(groupBox);

    d->endColorLabel = new QLabel(i18n("End Color: "), groupBox);
    d->endColorBtn   = new KColorButton(groupBox);

    d->addBorderCheck = new QCheckBox(i18n("Add Border"), groupBox);

    d->borderColorLabel = new QLabel(i18n("Border Color: "), groupBox);
    d->borderColorBtn   = new KColorButton(groupBox);

    vlay->setAlignment(Qt::AlignTop);
    vlay->setSpacing(5);
    vlay->setMargin(5);
    vlay->addWidget(label1);
    vlay->addWidget(d->propertyCombo);
    vlay->addWidget(d->bevelLabel);
    vlay->addWidget(d->bevelCombo);
    vlay->addWidget(d->gradientLabel);
    vlay->addWidget(d->gradientCombo);
    vlay->addWidget(d->begColorLabel);
    vlay->addWidget(d->begColorBtn);
    vlay->addWidget(d->endColorLabel);
    vlay->addWidget(d->endColorBtn);
    vlay->addWidget( d->addBorderCheck );
    vlay->addWidget(d->borderColorLabel);
    vlay->addWidget(d->borderColorBtn);
    vlay->addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // -------------------------------------------------------------

    d->propertyCombo->insertItem(BASE, "Base");
    d->propertyCombo->insertItem(REGULARTEXT, "Regular Text");
    d->propertyCombo->insertItem(SELECTEDTEXT, "Selected Text");
    d->propertyCombo->insertItem(REGULARSPECIALTEXT, "Special Regular Text");
    d->propertyCombo->insertItem(SELECTEDSPECIALTEXT, "Special Selected Text");
    d->propertyCombo->insertItem(BANNER, "Banner");
    d->propertyCombo->insertItem(THUMBNAILREGULAR, "Thumbnail Regular");
    d->propertyCombo->insertItem(THUMBNAILSELECTED, "Thumbnail Selected");
    d->propertyCombo->insertItem(LISTVIEWREGULAR, "ListView Regular");
    d->propertyCombo->insertItem(LISTVIEWSELECTED, "ListView Selected");

    d->bevelCombo->insertItem(FLAT, "Flat");
    d->bevelCombo->insertItem(RAISED, "Raised");
    d->bevelCombo->insertItem(SUNKEN, "Sunken");

    d->gradientCombo->insertItem(SOLID, "Solid");
    d->gradientCombo->insertItem(HORIZONTAL, "Horizontal");
    d->gradientCombo->insertItem(VERTICAL, "Vertical");
    d->gradientCombo->insertItem(DIAGONAL, "Diagonal");

    d->bevelMap[FLAT]   = Theme::FLAT;
    d->bevelMap[RAISED] = Theme::RAISED;
    d->bevelMap[SUNKEN] = Theme::SUNKEN;

    d->gradientMap[SOLID]      = Theme::SOLID;
    d->gradientMap[HORIZONTAL] = Theme::HORIZONTAL;
    d->gradientMap[VERTICAL]   = Theme::VERTICAL;
    d->gradientMap[DIAGONAL]   = Theme::DIAGONAL;

    d->bevelReverseMap[Theme::FLAT]   = FLAT;
    d->bevelReverseMap[Theme::RAISED] = RAISED;
    d->bevelReverseMap[Theme::SUNKEN] = SUNKEN;

    d->gradientReverseMap[Theme::SOLID]      = SOLID;
    d->gradientReverseMap[Theme::HORIZONTAL] = HORIZONTAL;
    d->gradientReverseMap[Theme::VERTICAL]   = VERTICAL;
    d->gradientReverseMap[Theme::DIAGONAL]   = DIAGONAL;

    d->begColorBtn->setColor(Qt::black);
    d->endColorBtn->setColor(Qt::black);
    d->borderColorBtn->setColor(Qt::black);

    // ------------------------------------------------------------------------

    QWidget*     mainWidget = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setMargin(5);
    mainLayout->setSpacing(5);
    mainLayout->addWidget(splitter, 0, 0, 1, 1);
    mainLayout->addWidget(groupBox, 0, 1, 1, 1);
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);

    // ------------------------------------------------------------------------

    connect(d->propertyCombo, SIGNAL(activated(int)),
            this, SLOT(slotPropertyChanged()));

    connect(d->bevelCombo, SIGNAL(activated(int)),
            this, SLOT(slotUpdateTheme()));

    connect(d->gradientCombo, SIGNAL(activated(int)),
            this, SLOT(slotUpdateTheme()));

    connect(d->begColorBtn, SIGNAL(changed(const QColor&)),
            this, SLOT(slotUpdateTheme()));

    connect(d->endColorBtn, SIGNAL(changed(const QColor&)),
            this, SLOT(slotUpdateTheme()));

    connect(d->addBorderCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdateTheme()));

    connect(d->borderColorBtn, SIGNAL(changed(const QColor&)),
            this, SLOT(slotUpdateTheme()));

    connect(this, SIGNAL(user2Clicked()),
            this, SLOT(slotLoad()));

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotSave()));

    connect(this, SIGNAL(closeClicked()),
            this, SLOT(close()));

    // ------------------------------------------------------------------------

    d->folderView->addColumn("My Albums");
    d->folderView->setResizeMode(Q3ListView::LastColumn);
    d->folderView->setRootIsDecorated(true);

    KIconLoader *iconLoader = KIconLoader::global();
    for (int i=0; i<10; ++i)
    {
        FolderItem* folderItem = new FolderItem(d->folderView, QString("Album %1").arg(i));
        folderItem->setPixmap(0, iconLoader->loadIcon("folder", KIconLoader::NoGroup, 32));
        if (i == 2)
        {
            d->folderView->setSelected(folderItem, true);
        }
    }

    // ------------------------------------------------------------------------

    slotPropertyChanged();
    slotUpdateTheme();
}

MainWindow::~MainWindow()
{
    delete d->theme;
}

void MainWindow::slotLoad()
{
    KUrl themesUrl(KGlobal::dirs()->findResourceDir("themes", QString()));

    QString path = KFileDialog::getOpenFileName(themesUrl, QString(), this, QString());
    if (path.isEmpty())
        return;

    QFileInfo fi(path);
    d->theme->name     = fi.fileName();
    d->theme->filePath = path;

    ThemeEngine::instance()->setCurrentTheme(*d->theme, d->theme->name, true);
    *d->theme = *(ThemeEngine::instance()->getCurrentTheme());
    slotPropertyChanged();
}

void MainWindow::slotSave()
{
    QString path = KFileDialog::getSaveFileName(KUrl(), QString(), this, QString());
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        KMessageBox::error(this, i18n("Failed to open file for writing"));
        return;
    }

    QFileInfo fi(path);
    d->theme->name     = fi.fileName();
    d->theme->filePath = path;

    ThemeEngine::instance()->setCurrentTheme(*d->theme, d->theme->name, false);
    ThemeEngine::instance()->saveTheme();
}

void MainWindow::slotPropertyChanged()
{
    d->bevelCombo->blockSignals(true);
    d->gradientCombo->blockSignals(true);
    d->begColorBtn->blockSignals(true);
    d->endColorBtn->blockSignals(true);
    d->addBorderCheck->blockSignals(true);
    d->borderColorBtn->blockSignals(true);

    d->bevelCombo->setEnabled(false);
    d->bevelLabel->setEnabled(false);
    d->gradientCombo->setEnabled(false);
    d->gradientLabel->setEnabled(false);
    d->begColorBtn->setEnabled(false);
    d->begColorLabel->setEnabled(false);
    d->endColorBtn->setEnabled(false);
    d->endColorLabel->setEnabled(false);
    d->addBorderCheck->setEnabled(false);
    d->borderColorBtn->setEnabled(false);
    d->borderColorLabel->setEnabled(false);

    switch(d->propertyCombo->currentIndex())
    {
        case(BASE):
        {
            d->begColorLabel->setEnabled(true);
            d->begColorBtn->setEnabled(true);
            d->begColorBtn->setColor(d->theme->baseColor);
            break;
        }
        case(REGULARTEXT):
        {
            d->begColorLabel->setEnabled(true);
            d->begColorBtn->setEnabled(true);
            d->begColorBtn->setColor(d->theme->textRegColor);
            break;
        }
        case(SELECTEDTEXT):
        {
            d->begColorLabel->setEnabled(true);
            d->begColorBtn->setEnabled(true);
            d->begColorBtn->setColor(d->theme->textSelColor);
            break;
        }
        case(REGULARSPECIALTEXT):
        {
            d->begColorLabel->setEnabled(true);
            d->begColorBtn->setEnabled(true);
            d->begColorBtn->setColor(d->theme->textSpecialRegColor);
            break;
        }
        case(SELECTEDSPECIALTEXT):
        {
            d->begColorLabel->setEnabled(true);
            d->begColorBtn->setEnabled(true);
            d->begColorBtn->setColor(d->theme->textSpecialSelColor);
            break;
        }
        case(BANNER):
        {
            d->bevelCombo->setEnabled(true);
            d->gradientCombo->setEnabled(true);
            d->begColorBtn->setEnabled(true);
            d->endColorBtn->setEnabled(true);
            d->addBorderCheck->setEnabled(true);
            d->borderColorBtn->setEnabled(true);
            d->bevelLabel->setEnabled(true);
            d->gradientLabel->setEnabled(true);
            d->begColorLabel->setEnabled(true);
            d->endColorLabel->setEnabled(true);
            d->borderColorLabel->setEnabled(true);

            d->bevelCombo->setCurrentIndex(d->bevelReverseMap[d->theme->bannerBevel]);
            d->gradientCombo->setCurrentIndex(d->gradientReverseMap[d->theme->bannerGrad]);

            d->begColorBtn->setColor(d->theme->bannerColor);
            d->endColorBtn->setColor(d->theme->bannerColorTo);

            d->addBorderCheck->setChecked(d->theme->bannerBorder);
            d->borderColorBtn->setColor(d->theme->bannerBorderColor);

            break;
        }
        case(THUMBNAILREGULAR):
        {
            d->bevelCombo->setEnabled(true);
            d->gradientCombo->setEnabled(true);
            d->begColorBtn->setEnabled(true);
            d->endColorBtn->setEnabled(true);
            d->addBorderCheck->setEnabled(true);
            d->borderColorBtn->setEnabled(true);
            d->bevelLabel->setEnabled(true);
            d->gradientLabel->setEnabled(true);
            d->begColorLabel->setEnabled(true);
            d->endColorLabel->setEnabled(true);
            d->borderColorLabel->setEnabled(true);

            d->bevelCombo->setCurrentIndex(d->bevelReverseMap[d->theme->thumbRegBevel]);
            d->gradientCombo->setCurrentIndex(d->gradientReverseMap[d->theme->thumbRegGrad]);

            d->begColorBtn->setColor(d->theme->thumbRegColor);
            d->endColorBtn->setColor(d->theme->thumbRegColorTo);

            d->addBorderCheck->setChecked(d->theme->thumbRegBorder);
            d->borderColorBtn->setColor(d->theme->thumbRegBorderColor);

            break;
        }
        case(THUMBNAILSELECTED):
        {
            d->bevelCombo->setEnabled(true);
            d->gradientCombo->setEnabled(true);
            d->begColorBtn->setEnabled(true);
            d->endColorBtn->setEnabled(true);
            d->addBorderCheck->setEnabled(true);
            d->borderColorBtn->setEnabled(true);
            d->bevelLabel->setEnabled(true);
            d->gradientLabel->setEnabled(true);
            d->begColorLabel->setEnabled(true);
            d->endColorLabel->setEnabled(true);
            d->borderColorLabel->setEnabled(true);

            d->bevelCombo->setCurrentIndex(d->bevelReverseMap[d->theme->thumbSelBevel]);
            d->gradientCombo->setCurrentIndex(d->gradientReverseMap[d->theme->thumbSelGrad]);

            d->begColorBtn->setColor(d->theme->thumbSelColor);
            d->endColorBtn->setColor(d->theme->thumbSelColorTo);

            d->addBorderCheck->setChecked(d->theme->thumbSelBorder);
            d->borderColorBtn->setColor(d->theme->thumbSelBorderColor);

            break;
        }
        case(LISTVIEWREGULAR):
        {
            d->bevelCombo->setEnabled(true);
            d->gradientCombo->setEnabled(true);
            d->begColorBtn->setEnabled(true);
            d->endColorBtn->setEnabled(true);
            d->addBorderCheck->setEnabled(true);
            d->borderColorBtn->setEnabled(true);
            d->bevelLabel->setEnabled(true);
            d->gradientLabel->setEnabled(true);
            d->begColorLabel->setEnabled(true);
            d->endColorLabel->setEnabled(true);
            d->borderColorLabel->setEnabled(true);

            d->bevelCombo->setCurrentIndex(d->bevelReverseMap[d->theme->listRegBevel]);
            d->gradientCombo->setCurrentIndex(d->gradientReverseMap[d->theme->listRegGrad]);

            d->begColorBtn->setColor(d->theme->listRegColor);
            d->endColorBtn->setColor(d->theme->listRegColorTo);

            d->addBorderCheck->setChecked(d->theme->listRegBorder);
            d->borderColorBtn->setColor(d->theme->listRegBorderColor);

            break;
        }
        case(LISTVIEWSELECTED):
        {
            d->bevelCombo->setEnabled(true);
            d->gradientCombo->setEnabled(true);
            d->begColorBtn->setEnabled(true);
            d->endColorBtn->setEnabled(true);
            d->addBorderCheck->setEnabled(true);
            d->borderColorBtn->setEnabled(true);
            d->bevelLabel->setEnabled(true);
            d->gradientLabel->setEnabled(true);
            d->begColorLabel->setEnabled(true);
            d->endColorLabel->setEnabled(true);
            d->borderColorLabel->setEnabled(true);

            d->bevelCombo->setCurrentIndex(d->bevelReverseMap[d->theme->listSelBevel]);
            d->gradientCombo->setCurrentIndex(d->gradientReverseMap[d->theme->listSelGrad]);

            d->begColorBtn->setColor(d->theme->listSelColor);
            d->endColorBtn->setColor(d->theme->listSelColorTo);

            d->addBorderCheck->setChecked(d->theme->listSelBorder);
            d->borderColorBtn->setColor(d->theme->listSelBorderColor);

            break;
        }
    };

    d->bevelCombo->blockSignals(false);
    d->gradientCombo->blockSignals(false);
    d->begColorBtn->blockSignals(false);
    d->endColorBtn->blockSignals(false);
    d->addBorderCheck->blockSignals(false);
    d->borderColorBtn->blockSignals(false);
}

void MainWindow::slotUpdateTheme()
{
    switch(d->propertyCombo->currentIndex())
    {
        case(BASE):
        {
            d->theme->baseColor = d->begColorBtn->color();
            break;
        }
        case(REGULARTEXT):
        {
            d->theme->textRegColor = d->begColorBtn->color();
            break;
        }
        case(SELECTEDTEXT):
        {
            d->theme->textSelColor = d->begColorBtn->color();
            break;
        }
        case(REGULARSPECIALTEXT):
        {
            d->theme->textSpecialRegColor = d->begColorBtn->color();
            break;
        }
        case(SELECTEDSPECIALTEXT):
        {
            d->theme->textSpecialSelColor = d->begColorBtn->color();
            break;
        }
        case(BANNER):
        {
            d->theme->bannerBevel = (Theme::Bevel) d->bevelMap[d->bevelCombo->currentIndex()];
            d->theme->bannerGrad  = (Theme::Gradient) d->gradientMap[d->gradientCombo->currentIndex()];

            d->theme->bannerColor   = d->begColorBtn->color();
            d->theme->bannerColorTo = d->endColorBtn->color();

            d->theme->bannerBorder      = d->addBorderCheck->isChecked();
            d->theme->bannerBorderColor = d->borderColorBtn->color();

            break;
        }
        case(THUMBNAILREGULAR):
        {
            d->theme->thumbRegBevel = (Theme::Bevel) d->bevelMap[d->bevelCombo->currentIndex()];
            d->theme->thumbRegGrad  = (Theme::Gradient) d->gradientMap[d->gradientCombo->currentIndex()];

            d->theme->thumbRegColor   = d->begColorBtn->color();
            d->theme->thumbRegColorTo = d->endColorBtn->color();

            d->theme->thumbRegBorder      = d->addBorderCheck->isChecked();
            d->theme->thumbRegBorderColor = d->borderColorBtn->color();

            break;
        }
        case(THUMBNAILSELECTED):
        {
            d->theme->thumbSelBevel = (Theme::Bevel) d->bevelMap[d->bevelCombo->currentIndex()];
            d->theme->thumbSelGrad  = (Theme::Gradient) d->gradientMap[d->gradientCombo->currentIndex()];

            d->theme->thumbSelColor   = d->begColorBtn->color();
            d->theme->thumbSelColorTo = d->endColorBtn->color();

            d->theme->thumbSelBorder      = d->addBorderCheck->isChecked();
            d->theme->thumbSelBorderColor = d->borderColorBtn->color();

            break;
        }
        case(LISTVIEWREGULAR):
        {
            d->theme->listRegBevel = (Theme::Bevel) d->bevelMap[d->bevelCombo->currentIndex()];
            d->theme->listRegGrad  = (Theme::Gradient) d->gradientMap[d->gradientCombo->currentIndex()];

            d->theme->listRegColor   = d->begColorBtn->color();
            d->theme->listRegColorTo = d->endColorBtn->color();

            d->theme->listRegBorder      = d->addBorderCheck->isChecked();
            d->theme->listRegBorderColor = d->borderColorBtn->color();

            break;
        }
        case(LISTVIEWSELECTED):
        {
            d->theme->listSelBevel = (Theme::Bevel) d->bevelMap[d->bevelCombo->currentIndex()];
            d->theme->listSelGrad  = (Theme::Gradient) d->gradientMap[d->gradientCombo->currentIndex()];

            d->theme->listSelColor   = d->begColorBtn->color();
            d->theme->listSelColorTo = d->endColorBtn->color();

            d->theme->listSelBorder      = d->addBorderCheck->isChecked();
            d->theme->listSelBorderColor = d->borderColorBtn->color();

            break;
        }
    };

    ThemeEngine::instance()->setCurrentTheme(*d->theme, "Digikam ThemeEditor Theme");
}

}  // namespace Digikam
