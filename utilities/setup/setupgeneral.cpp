//////////////////////////////////////////////////////////////////////////////
//
//    SETUPGENERAL.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// QT includes.

#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qlistbox.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <klineeditdlg.h>

// // Local includes.

#include "thumbnailsize.h"
#include "albumsettings.h"
#include "setupgeneral.h"


SetupGeneral::SetupGeneral(QWidget* parent )
            : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent, 10);
   layout->setSpacing( KDialog::spacingHint() );

   // --------------------------------------------------------

   QHGroupBox *albumPathBox = new QHGroupBox(parent);
   albumPathBox->setTitle(i18n("Album Library Path"));

   albumPathEdit = new QLineEdit(albumPathBox);
   QPushButton *changePathButton = new QPushButton(i18n("&Change..."),
                                                   albumPathBox);
   connect(changePathButton, SIGNAL(clicked()),
           this, SLOT(slotChangeAlbumPath()));

   layout->addWidget(albumPathBox);

   // --------------------------------------------------------

   QButtonGroup *iconSizeButtonGroup = new QButtonGroup(2,
                                           Qt::Horizontal, 
                                           i18n("Default Thumbnail Size"),
                                           parent);
   
   iconSizeButtonGroup->setRadioButtonExclusive(true);

   smallIconButton_ = new QRadioButton(iconSizeButtonGroup);
   smallIconButton_->setText( i18n( "Small (64x64)" ) );

   mediumIconButton_ = new QRadioButton(iconSizeButtonGroup);
   mediumIconButton_->setText( i18n( "Medium (100x100)" ) );
   
   largeIconButton_ = new QRadioButton(iconSizeButtonGroup);
   largeIconButton_->setText( i18n( "Large (160x160)" ) );

   hugeIconButton_ = new QRadioButton(iconSizeButtonGroup);
   hugeIconButton_->setText( i18n( "Huge (256x256)" ) );

   layout->addWidget(iconSizeButtonGroup);

   // --------------------------------------------------------

   QGroupBox *iconTextGroup = new QGroupBox(2, Qt::Horizontal, 
                                            i18n("Extra Information in Thumbnail View"),
                                            parent);

   iconShowMimeBox_ = new QCheckBox(iconTextGroup);
   iconShowMimeBox_->setText(i18n("Show file type (e.g. image/jpeg)"));

   iconShowSizeBox_ = new QCheckBox(iconTextGroup);
   iconShowSizeBox_->setText(i18n("Show file size"));

   iconShowDateBox_ = new QCheckBox(iconTextGroup);
   iconShowDateBox_->setText(i18n("Show file modification date"));

   iconShowCommentsBox_ = new QCheckBox(iconTextGroup);
   iconShowCommentsBox_->setText(i18n("Show File Comments"));

   layout->addWidget(iconTextGroup);
   
   // --------------------------------------------------------

   QGroupBox* collectionGroup = new QVGroupBox(parent);
   collectionGroup->setTitle(i18n("Album Collection Types"));
   collectionGroup->setColumnLayout(0, Qt::Vertical );
   collectionGroup->layout()->setSpacing( KDialog::spacingHint() );
   collectionGroup->layout()->setMargin( KDialog::marginHint() );
   QGridLayout* collectionGroupLayout = new QGridLayout( collectionGroup->layout() );
   collectionGroupLayout->setAlignment( Qt::AlignTop );
   
   albumCollectionBox_ = new QListBox(collectionGroup);
   albumCollectionBox_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                                  QSizePolicy::MinimumExpanding));

   albumCollectionBox_->setMaximumHeight(80);
   albumCollectionBox_->setScrollBar(true);
	  
   collectionGroupLayout->addMultiCellWidget( albumCollectionBox_,
                                              0, 2, 0, 0 );

   addCollectionButton_ = new QPushButton( i18n("&Add..."),
                                           collectionGroup);
   collectionGroupLayout->addWidget( addCollectionButton_, 0, 1);

   delCollectionButton_ = new QPushButton( i18n("&Delete"),
                                           collectionGroup);
   collectionGroupLayout->addWidget( delCollectionButton_, 1, 1);
   delCollectionButton_->setEnabled(false);

   connect(albumCollectionBox_, SIGNAL(selectionChanged()),
           this, SLOT(slotCollectionSelectionChanged()));
           
   connect(addCollectionButton_, SIGNAL(clicked()),
           this, SLOT(slotAddCollection()));
           
   connect(delCollectionButton_, SIGNAL(clicked()),
           this, SLOT(slotDelCollection()));
   
   layout->addWidget(collectionGroup);

   // --------------------------------------------------------

   layout->addStretch();

   readSettings();
}

SetupGeneral::~SetupGeneral()
{
}

void SetupGeneral::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    
    if (!settings) return;

    settings->setAlbumLibraryPath(albumPathEdit->text());
    
    int iconSize = ThumbnailSize::Medium;
    
    if (smallIconButton_->isChecked())
        iconSize = ThumbnailSize::Small;
    
    if (largeIconButton_->isChecked())
        iconSize = ThumbnailSize::Large;
    
    if (hugeIconButton_->isChecked())
        iconSize = ThumbnailSize::Huge;
    
    settings->setDefaultIconSize(iconSize);
    settings->setIconShowMime(iconShowMimeBox_->isChecked());
    settings->setIconShowSize(iconShowSizeBox_->isChecked());
    settings->setIconShowDate(iconShowDateBox_->isChecked());
    settings->setIconShowComments(iconShowCommentsBox_->isChecked());

    QStringList collectionList;
    
    for (QListBoxItem *item = albumCollectionBox_->firstItem();
         item; item = item->next()) 
         {
         collectionList.append(item->text());
         }
         
    settings->setAlbumCollectionNames(collectionList);
    
    settings->saveSettings();
}

void SetupGeneral::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    
    if (!settings) return;

    albumPathEdit->setText(settings->getAlbumLibraryPath());
    
    switch(settings->getDefaultIconSize()) {
    case(ThumbnailSize::Small): {
        smallIconButton_->setChecked(true);
        break;
    }
    case(ThumbnailSize::Large): {
        largeIconButton_->setChecked(true);
        break;
    }
    case(ThumbnailSize::Huge): {
        hugeIconButton_->setChecked(true);
        break;
    }
    default:
        mediumIconButton_->setChecked(true);
        break;
    }

    iconShowMimeBox_->setChecked(settings->getIconShowMime());
    iconShowSizeBox_->setChecked(settings->getIconShowSize());
    iconShowDateBox_->setChecked(settings->getIconShowDate());
    iconShowCommentsBox_->setChecked(settings->getIconShowComments());

    albumCollectionBox_->insertStringList(settings->getAlbumCollectionNames());
}

void SetupGeneral::slotChangeAlbumPath()
{
    QString  result =
        KFileDialog::getExistingDirectory(
            albumPathEdit->text(),
            this);

    if (KURL(result).cmp(KURL(QDir::homeDirPath()), true)) {
        KMessageBox::sorry(0, i18n("Sorry Cannot Use Home Directory as Album Library"));
        return;
    }

    if (!result.isEmpty()) {
        albumPathEdit->setText(result);
    }
}

void SetupGeneral::slotCollectionSelectionChanged()
{
    if (albumCollectionBox_->currentItem() != -1)
        delCollectionButton_->setEnabled(true);
    else
        delCollectionButton_->setEnabled(false);
}

void SetupGeneral::slotAddCollection()
{
    bool ok;
    QString newCollection =
        KLineEditDlg::getText(i18n("Enter New Collection Name: "),
                              "", &ok, this);
    if (!ok) return;

    bool found = false;
    for (QListBoxItem *item = albumCollectionBox_->firstItem();
         item; item = item->next()) {
        if (newCollection == item->text()) {
            found = true;
            break;
        }
    }

    if (!found)
        albumCollectionBox_->insertItem(newCollection);
}

void SetupGeneral::slotDelCollection()
{
    int index = albumCollectionBox_->currentItem();
    if (index == -1)
        return;
    
    QListBoxItem* item = albumCollectionBox_->item(index);
    if (!item) return;
    delete item;
}

#include "setupgeneral.moc"
