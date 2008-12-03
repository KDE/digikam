/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Album properties dialog.
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <qdatetime.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qvalidator.h>

// KDE includes.

#include <kdatepicker.h>
#include <ktextedit.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurl.h>
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
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumpropsedit.h"
#include "albumpropsedit.moc"

namespace Digikam
{

class AlbumPropsEditPriv
{

public:

    AlbumPropsEditPriv()
    {
        titleEdit          = 0;
        collectionCombo    = 0;
        commentsEdit       = 0;
        datePicker         = 0;
        album              = 0;
    }

    QStringList     albumCollections;

    QComboBox      *collectionCombo;

    KLineEdit      *titleEdit;

    KTextEdit      *commentsEdit;

    KDatePicker    *datePicker;

    PAlbum         *album;
};

AlbumPropsEdit::AlbumPropsEdit(PAlbum* album, bool create)
              : KDialogBase( Plain, create ? i18n("New Album") : i18n("Edit Album"),
                             Help|Ok|Cancel, Ok,
                             0, 0, true, true )
{
    d = new AlbumPropsEditPriv;
    d->album = album;
    setHelp("albumpropsedit.anchor", "digikam");

    QGridLayout *topLayout = new QGridLayout( plainPage(), 2, 6,
                                              0, spacingHint() );

    QLabel *topLabel = new QLabel( plainPage() );
    if (create)
    {
        topLabel->setText( i18n( "<qt><b>Create new Album in \"<i>%1</i>\"</b></qt>")
                           .arg(album->title()));
    }
    else
    {
        topLabel->setText( i18n( "<qt><b>\"<i>%1</i>\" Album Properties</b></qt>")
                           .arg(album->title()));
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

    d->titleEdit = new KLineEdit( plainPage( ) );
    topLayout->addWidget( d->titleEdit, 2, 1 );
    titleLabel->setBuddy( d->titleEdit );

    QRegExp titleRx("[^/]+");
    QValidator *titleValidator = new QRegExpValidator(titleRx, this);
    d->titleEdit->setValidator(titleValidator);

    QLabel *collectionLabel = new QLabel( plainPage( ) );
    collectionLabel->setText( i18n( "Co&llection:" ) );
    topLayout->addWidget( collectionLabel, 3, 0 );

    d->collectionCombo = new QComboBox( plainPage( ) );
    d->collectionCombo->setEditable(true);
    topLayout->addWidget( d->collectionCombo, 3, 1 );
    collectionLabel->setBuddy( d->collectionCombo );

    QLabel *commentsLabel = new QLabel( plainPage( ) );
    commentsLabel->setText( i18n( "Ca&ption:" ) );
    topLayout->addWidget( commentsLabel, 4, 0, Qt::AlignAuto|Qt::AlignTop );

    d->commentsEdit = new KTextEdit( plainPage( ) );
    topLayout->addWidget( d->commentsEdit, 4, 1 );
    commentsLabel->setBuddy( d->commentsEdit );
    d->commentsEdit->setCheckSpellingEnabled(true);
    d->commentsEdit->setWordWrap(QTextEdit::WidgetWidth);
    d->commentsEdit->setWrapPolicy(QTextEdit::AtWhiteSpace);

    QLabel *dateLabel = new QLabel( plainPage( ) );
    dateLabel->setText( i18n( "Album &date:" ) );
    topLayout->addWidget( dateLabel, 5, 0, Qt::AlignAuto|Qt::AlignTop );

    d->datePicker = new KDatePicker( plainPage( ) );
    topLayout->addWidget( d->datePicker, 5, 1 );
    dateLabel->setBuddy( d->datePicker );

    QHBox *buttonRow = new QHBox( plainPage( ) );
    QPushButton *dateLowButton = new QPushButton(
            i18n("Selects the date of the oldest image",
                 "&Oldest" ), buttonRow );
    QPushButton *dateAvgButton = new QPushButton(
            i18n("Calculates the average date",
                 "&Average" ), buttonRow );
    QPushButton *dateHighButton = new QPushButton(
            i18n("Selects the date of the newest image",
                 "Newest" ), buttonRow );

    topLayout->addWidget( buttonRow, 6, 1);

    setTabOrder(d->titleEdit, d->collectionCombo);
    setTabOrder(d->collectionCombo, d->commentsEdit);
    setTabOrder(d->commentsEdit, d->datePicker);
    d->commentsEdit->setTabChangesFocus(true);
    d->titleEdit->selectAll();
    d->titleEdit->setFocus();

    // Initialize ---------------------------------------------

    AlbumSettings *settings = AlbumSettings::instance();
    if (settings)
    {
        d->collectionCombo->insertItem( QString() );
        QStringList collections = settings->getAlbumCollectionNames();
        d->collectionCombo->insertStringList( collections );
        int collectionIndex = collections.findIndex( album->collection() );

        if ( collectionIndex != -1 )
        {
            // + 1 because of the empty item
            d->collectionCombo->setCurrentItem(collectionIndex + 1);
        }
    }

    if (create)
    {
        d->titleEdit->setText( i18n("New Album") );
        d->datePicker->setDate( QDate::currentDate() );
    }
    else
    {
        d->titleEdit->setText( album->title() );
        d->commentsEdit->setText( album->caption() );
        d->datePicker->setDate( album->date() );
    }

    // -- slots connections -------------------------------------------

    connect(d->titleEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTitleChanged(const QString&)));

    connect(dateLowButton, SIGNAL( clicked() ),
            this, SLOT( slotDateLowButtonClicked()));

    connect(dateAvgButton, SIGNAL( clicked() ),
            this, SLOT( slotDateAverageButtonClicked()));

    connect(dateHighButton, SIGNAL( clicked() ),
            this, SLOT( slotDateHighButtonClicked()));

    adjustSize();
}

AlbumPropsEdit::~AlbumPropsEdit()
{
    delete d;
}

QString AlbumPropsEdit::title() const
{
    return d->titleEdit->text();
}

QString AlbumPropsEdit::comments() const
{
    return d->commentsEdit->text();
}

QDate AlbumPropsEdit::date() const
{
    return d->datePicker->date();
}

QString AlbumPropsEdit::collection() const
{
    QString name = d->collectionCombo->currentText();

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

    QString currentCollection = d->collectionCombo->currentText();
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

void AlbumPropsEdit::slotDateLowButtonClicked()
{
    setCursor( KCursor::waitCursor() );

    AlbumDB* db = AlbumManager::instance()->albumDB();
    QDate avDate = db->getAlbumLowestDate( d->album->id() );
    setCursor( KCursor::arrowCursor() );

    if ( avDate.isValid() )
        d->datePicker->setDate( avDate );
}

void AlbumPropsEdit::slotDateHighButtonClicked()
{
    setCursor( KCursor::waitCursor() );

    AlbumDB* db = AlbumManager::instance()->albumDB();
    QDate avDate = db->getAlbumHighestDate( d->album->id() );
    setCursor( KCursor::arrowCursor() );

    if ( avDate.isValid() )
        d->datePicker->setDate( avDate );
}

void AlbumPropsEdit::slotDateAverageButtonClicked()
{
    setCursor( KCursor::waitCursor() );

    AlbumDB* db = AlbumManager::instance()->albumDB();
    QDate avDate = db->getAlbumAverageDate( d->album->id() );
    setCursor( KCursor::arrowCursor() );

    if ( avDate.isValid() )
        d->datePicker->setDate( avDate );
    else
        KMessageBox::error( plainPage( ),
                            i18n( "Could not calculate an average."),
                            i18n( "Could Not Calculate Average" ) );
}

}  // namespace Digikam


