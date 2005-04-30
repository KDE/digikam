/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-03-09
 * Description :
 *
 * Copyright 2003 by Renchi Raju
 * Copyright 2005 by Tom Albers
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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qframe.h>
#include <qheader.h>
#include <qpushbutton.h>

// KDE includes.

#include <kdatepicker.h>
#include <ktextedit.h>
#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kcursor.h>

#include <kdeversion.h>
#if KDE_IS_VERSION(3,2,0)
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

// Local includes.

#include "album.h"
#include "albumsettings.h"
#include "albumpropsedit.h"

#include <libkexif/kexifdata.h>

AlbumPropsEdit::AlbumPropsEdit(PAlbum* album, bool create)
    : KDialogBase( Plain, create ? i18n("New Album") : i18n("Edit Album"),
                   Help|Ok|Cancel, Ok,
                   0, 0, true, true )
{
    setHelp("albumpropsedit.anchor", "digikam");
    album_ = album;

    QGridLayout *topLayout = new QGridLayout( plainPage(), 2, 6,
                                              0, spacingHint() );

    QLabel *topLabel = new QLabel( plainPage() );
    if (create)
    {
        topLabel->setText( i18n( "<qt><b>Create new Album in </b>%1</qt>")
                           .arg(album->getTitle()));
    }
    else
    {
        topLabel->setText( i18n( "<qt><b><i>%1</i> Album Properties</b></qt>")
                           .arg(album->getTitle()));
    }
    topLabel->setAlignment(Qt::AlignAuto | Qt::AlignVCenter | Qt::SingleLine);
    topLayout->addMultiCellWidget( topLabel, 0, 0, 0, 1  );

    // --------------------------------------------------------

    QFrame *topLine = new QFrame( plainPage() );
    topLine->setFrameShape( QFrame::HLine );
    topLine->setFrameShadow( QFrame::Sunken );
    topLayout->addMultiCellWidget( topLine, 1, 1, 0, 1  );

    // --------------------------------------------------------

    QLabel *titleLabel = new QLabel( plainPage( ) );
    titleLabel->setText( i18n( "&Title:" ) );
    topLayout->addWidget( titleLabel, 2, 0 );

    titleEdit_ = new QLineEdit( plainPage( ) );
    topLayout->addWidget( titleEdit_, 2, 1 );
    titleLabel->setBuddy( titleEdit_ );

    QLabel *collectionLabel = new QLabel( plainPage( ) );
    collectionLabel->setText( i18n( "Co&llection:" ) );
    topLayout->addWidget( collectionLabel, 3, 0 );

    collectionCombo_ = new QComboBox( plainPage( ) );
    collectionCombo_->setEditable(true);
    topLayout->addWidget( collectionCombo_, 3, 1 );
    collectionLabel->setBuddy( collectionCombo_ );

    QLabel *commentsLabel = new QLabel( plainPage( ) );
    commentsLabel->setText( i18n( "Co&mments:" ) );
    topLayout->addWidget( commentsLabel, 4, 0, Qt::AlignAuto|Qt::AlignTop );

    commentsEdit_ = new KTextEdit( plainPage( ) );
    topLayout->addWidget( commentsEdit_, 4, 1 );
    commentsLabel->setBuddy( commentsEdit_ );
    commentsEdit_->setMaximumHeight(int(commentsEdit_->fontMetrics().height() * 2.5));

    QLabel *dateLabel = new QLabel( plainPage( ) );
    dateLabel->setText( i18n( "Album &date:" ) );
    topLayout->addWidget( dateLabel, 5, 0, Qt::AlignAuto|Qt::AlignTop );

    datePicker_ = new KDatePicker( plainPage( ) );
    topLayout->addWidget( datePicker_, 5, 1 );
    dateLabel->setBuddy( datePicker_ );

    QPushButton *avgButton = new QPushButton( 
                                i18n("This is a button which calculates "
                                     "the average date",
                                     "&Average" ), plainPage( ) );
    topLayout->addWidget( avgButton, 6, 1);

    setTabOrder(titleEdit_, collectionCombo_);
    setTabOrder(collectionCombo_, commentsEdit_);
    setTabOrder(commentsEdit_, datePicker_);
    commentsEdit_->setTabChangesFocus(true);
    titleEdit_->selectAll();
    titleEdit_->setFocus();

    // Initialize ---------------------------------------------

    AlbumSettings *settings = AlbumSettings::instance();
    if (settings)
    {
        collectionCombo_->insertItem( QString::null );
        QStringList collections = settings->getAlbumCollectionNames();
        collectionCombo_->insertStringList( collections );
        int collectionIndex = collections.findIndex( album->getCollection() );
        if ( collectionIndex != -1 )
        {
            // + 1 because of the empty item
            collectionCombo_->setCurrentItem(collectionIndex + 1);
        }
    }

    if (create)
    {
        titleEdit_->setText( i18n("New Album") );
        datePicker_->setDate( QDate::currentDate() );
    }
    else
    {
        titleEdit_->setText( album->getTitle() );
        commentsEdit_->setText( album->getCaption() );
        datePicker_->setDate( album->getDate() );
    }

    // Connections -------------------------------------------

    connect(titleEdit_, SIGNAL(textChanged(const QString&)),
            SLOT(slotTitleChanged(const QString&)));
    connect(avgButton, SIGNAL( clicked() ),
            SLOT( slotAverageButtonClicked()));
    
    adjustSize();
}

AlbumPropsEdit::~AlbumPropsEdit()
{
}

QString AlbumPropsEdit::title() const
{
    return titleEdit_->text();
}

QString AlbumPropsEdit::comments() const
{
    return commentsEdit_->text();
}

QDate AlbumPropsEdit::date() const
{
    return datePicker_->date();
}

QString AlbumPropsEdit::collection() const
{
    QString name = collectionCombo_->currentText();

    if (name.isEmpty())
    {
        name = i18n( "Uncategorized Album" );
    }

    return name;
}

QStringList AlbumPropsEdit::albumCollections() const
{
    QStringList collections;
    AlbumSettings *settings = AlbumSettings::instance();
    if (settings)
    {
        collections = settings->getAlbumCollectionNames();
    }

    QString currentCollection = collectionCombo_->currentText();
    if ( collections.findIndex( currentCollection ) == -1 )
    {
        collections.append(currentCollection);
    }

    collections.sort();
    return collections;
}

bool AlbumPropsEdit::editProps(PAlbum *album, QString& title,
                               QString& comments, QDate& date, QString& collection,
                               QStringList& albumCollections)
{
    AlbumPropsEdit dlg(album);

    bool ok = dlg.exec() == QDialog::Accepted;

    title            = dlg.title();
    comments         = dlg.comments();
    date             = dlg.date();
    collection       = dlg.collection();
    albumCollections = dlg.albumCollections();

    return ok;
}

bool AlbumPropsEdit::createNew(PAlbum  *parent,
                               QString& title,
                               QString& comments,
                               QDate&   date,
                               QString& collection,
                               QStringList& albumCollections)
{
    AlbumPropsEdit dlg(parent, true);

    bool ok = dlg.exec() == QDialog::Accepted;

    title            = dlg.title();
    comments         = dlg.comments();
    date             = dlg.date();
    collection       = dlg.collection();
    albumCollections = dlg.albumCollections();

    return ok;
}

void AlbumPropsEdit::slotTitleChanged(const QString& newtitle)
{
    enableButtonOK(!newtitle.isEmpty());    
}

void AlbumPropsEdit::slotAverageButtonClicked()
{
    setCursor( KCursor::waitCursor() );
    QDate guessedDate = averageDate( true );
    kdDebug() << "Exif-based returned: " << guessedDate << endl; 
    if ( guessedDate.isValid() )
    {
        setCursor( KCursor::arrowCursor() );
        datePicker_->setDate( guessedDate );
    }
    else
    {
        guessedDate = averageDate ( false );
        setCursor( KCursor::arrowCursor() );
        kdDebug() << "File-based resturned: " << guessedDate << endl;
        if ( guessedDate.isValid() )
            datePicker_->setDate( guessedDate );
        else
            KMessageBox::error( plainPage( ), 
                                i18n( "Could not calculate an average based "
                                      "on creation date (EXIF) or based on the "
                                      "modification date of the files." ),
                                i18n( "Could Not Calculate Average" ) );
    }
}

QDate AlbumPropsEdit::averageDate( bool basedOnExif ) const
{
    int differenceInSecs = 0;
    int amountOfImages = 0;
    KExifData exifData;
    QDateTime baseDateTime;

    QDir dir(album_->getKURL().path());
    dir.setFilter( QDir::Files );
    const QFileInfoList *list = dir.entryInfoList();
    if (!list) return QDate();

    QFileInfoListIterator it (*list);
    QFileInfo *fi;
    while ( ( fi = it.current() ) != 0)
    {
        QDateTime fileDateTime;
        if (basedOnExif)
        {
            exifData.readFromFile( fi->absFilePath() );
            fileDateTime = exifData.getExifDateTime();
        } else
            fileDateTime = fi->lastModified();   // fi->created() ?

        if ( fileDateTime.isValid() )
        {
            ++amountOfImages;
            if ( baseDateTime.isNull() )
                baseDateTime=fileDateTime;
            else
                differenceInSecs += fileDateTime.secsTo( baseDateTime );
        }
        ++it;
    }

    if ( amountOfImages > 0 )
    {
        QDateTime averageDateTime;
        averageDateTime.setTime_t( baseDateTime.toTime_t() -
                                   (int)( differenceInSecs/amountOfImages ) );
        return ( averageDateTime.date() );
    }
    else
        return QDate();
}

#include "albumpropsedit.moc"
