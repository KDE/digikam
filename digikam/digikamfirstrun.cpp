#include <iostream>

#include <klocale.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qstring.h>
#include <qdir.h>

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include "digikamfirstrun.h"

DigikamFirstRun::DigikamFirstRun( KConfig* config,
                                  QWidget* parent,
                                  const char* name, bool modal,
                                  WFlags fl )
    : QDialog( parent, name, modal, fl )

{
    config_ = config;
    
    QVBoxLayout* mainLayout = new QVBoxLayout( this, 6, 11 );

    QGroupBox *topBox = new QGroupBox( this );
    topBox->setTitle( i18n( "Album Library Path" ) );
    topBox->setColumnLayout(0, Qt::Vertical );
    topBox->layout()->setSpacing( 6 );
    topBox->layout()->setMargin( 11 );
    QVBoxLayout* topBoxLayout =
        new QVBoxLayout( topBox->layout() );
    topBoxLayout->setAlignment( Qt::AlignTop );

    QLabel* infoLabel = new QLabel( topBox );
    infoLabel->setText( i18n( "It looks like you are running Digikam for the first time \n"
                              "or upgrading from a previous version. The way Digikam\n"
                              "organizes albums has changed from the previous versions.\n"
                              "You need to set an Album Library Path below, where New Albums \n"
                              "can be created. \n"
                              "(Recommended path would be a directory Pictures)") );
    topBoxLayout->addWidget( infoLabel );

    QGroupBox* pathBox = new QGroupBox( topBox );
    pathBox->setColumnLayout(0, Qt::Vertical );
    pathBox->layout()->setSpacing( 5 );
    pathBox->layout()->setMargin( 5 );
    QHBoxLayout* pathBoxLayout =
        new QHBoxLayout( pathBox->layout() );
    pathBoxLayout->setAlignment( Qt::AlignTop );

    pathEdit_ = new QLineEdit( pathBox );
    pathBoxLayout->addWidget( pathEdit_ );

    QPushButton *pathChangeButton =
        new QPushButton( i18n("&Select"), pathBox );
    pathBoxLayout->addWidget( pathChangeButton );
    
    topBoxLayout->addWidget( pathBox );

    mainLayout->addWidget( topBox );

    QHBoxLayout* botLayout = new QHBoxLayout( 0, 0, 6); 

    QSpacerItem* spacer = new QSpacerItem( 20, 20,
                                           QSizePolicy::Expanding,
                                           QSizePolicy::Minimum );
    botLayout->addItem( spacer );

    okButton_ = new QPushButton( i18n("&Ok"),
                                             this );
    okButton_->setAutoDefault( true );
    okButton_->setDefault( true );
    okButton_->setEnabled( false );
    botLayout->addWidget( okButton_ );

    cancelButton_ = new QPushButton( i18n("&Cancel"),
                                     this );
    cancelButton_->setAutoDefault( TRUE );
    botLayout->addWidget( cancelButton_ );

    mainLayout->addLayout( botLayout );
    
    // signals and slots connections
    connect( okButton_, SIGNAL( clicked() ),
             this, SLOT( accept() ) );
    connect( cancelButton_, SIGNAL( clicked() ),
             this, SLOT( reject() ) );
    connect( pathChangeButton, SIGNAL( clicked() ),
             this, SLOT( slotChangePath() ) );
    connect( pathEdit_, SIGNAL(textChanged(const QString&)),
             this, SLOT(slotPathEdited(const QString&)) );
}

DigikamFirstRun::~DigikamFirstRun()
{
}

void DigikamFirstRun::accept()
{
    config_->setGroup("General Settings");
    config_->writeEntry("Version", 0.60);

    config_->setGroup("Album Settings");
    config_->writePathEntry("Album Path",
                        pathEdit_->text());
    config_->sync();


    QDialog::accept();

    QString ErrorMsg, URL;

    if (kapp->startServiceByDesktopName("digikam", URL , &ErrorMsg) > 0)
    {
    	kdError() << ErrorMsg << endl;
	KMessageBox::sorry(0, i18n("Cannot restart Digikam like a KDE service.\nPlease restart Digikam manually."));
    }

}

void DigikamFirstRun::slotChangePath()
{
    QString  result =
        KFileDialog::getExistingDirectory(
            pathEdit_->text(),
            this);

    if (KURL(result).equals(KURL(QDir::homeDirPath()), true)) {
        KMessageBox::sorry(0, i18n("Cannot Use Home Directory as Album Library"));
        return;
    }

    if (!result.isEmpty()) {
        pathEdit_->setText(result);
    }
}

void DigikamFirstRun::slotPathEdited(const QString& newPath)
{
    if (newPath.isEmpty()) {
       okButton_->setEnabled(false);
       return;
    }

    if (!newPath.startsWith("/")) {
        pathEdit_->setText(QDir::homeDirPath()+"/"+newPath);
    }

    QDir dir(newPath);
    okButton_->setEnabled(dir.exists() && dir != QDir(QDir::homeDirPath ()));
}

#include "digikamfirstrun.moc"
