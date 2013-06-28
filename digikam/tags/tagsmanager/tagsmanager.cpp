#include "tagsmanager.h"
#include <kdebug.h>
#include <klocale.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <ktoolbar.h>
#include <kmainwindow.h>

TagsManager::TagsManager()
    : KDialog(0)
{
    //this->resize(800,600);

    /** No buttons **/
    this->setButtons(0x00);

    setupUi(this);
    //this->show();

    kDebug() << "My new Tags Manager class";
}

TagsManager::~TagsManager()
{
}

void TagsManager::setupUi(KDialog *Dialog)
{

     Dialog->resize(972, 722);
     Dialog->setWindowTitle(i18n("Tags Manager"));

     QVBoxLayout* mainLayout = new QVBoxLayout(Dialog);
     QHBoxLayout* firstLine = new QHBoxLayout(Dialog);

     tagPixmap = new QLabel();
     tagPixmap->setText("Tag Pixmap");
     tagPixmap->setMaximumWidth(40);
     tagPixmap->setPixmap(KIcon("tag").pixmap());

     lineEdit = new QLineEdit();
     lineEdit->setText(i18n("Search..."));
     lineEdit->setMaximumWidth(200);

     digikamPixmap = new QLabel();
     digikamPixmap->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/about/top-left-digikam.png")).scaled(40,40,Qt::KeepAspectRatio));
     digikamPixmap->setMaximumHeight(40);
     digikamPixmap->setScaledContents(true);


     QHBoxLayout* tempLayout = new QHBoxLayout();
     tempLayout->setAlignment(Qt::AlignLeft);
     tempLayout->addWidget(tagPixmap);
     tempLayout->addWidget(lineEdit);
     firstLine->addLayout(tempLayout);
     firstLine->addWidget(digikamPixmap);

     tagmngrLabel = new QLabel(Dialog);
     tagmngrLabel->setObjectName(QString::fromUtf8("label"));
     tagmngrLabel->setText(i18n("Tags Manager"));
     QFont font2;
     font2.setPointSize(12);
     font2.setBold(true);
     font2.setWeight(75);
     tagmngrLabel->setFont(font2);
     tagmngrLabel->setAutoFillBackground(true);

     treeWindow = new KMainWindow();
     toolbar = new KToolBar(treeWindow);

     addAction = new KAction(i18n("Add"),this);
     delAction = new KAction(i18n("Del"),this);

     organizeAction   = new KActionMenu(i18n("Organize"),this);
     organizeAction->setDelayed(false);
     organizeAction->addAction(new KAction("New Tag",this));

     syncexportAction = new KActionMenu(i18n("Sync & Export"),this);

     tagProperties = new KAction(i18n("Tag Properties"),this);

     toolbar->addAction(addAction);
     toolbar->addAction(delAction);
     toolbar->addAction(organizeAction);
     toolbar->addAction(syncexportAction);
     toolbar->addAction(tagProperties);
     treeWindow->addToolBar(toolbar);

     QHBoxLayout* thirdLine = new QHBoxLayout(Dialog);
     listView = new QListView(Dialog);
     listView->setObjectName(QString::fromUtf8("listView"));
     //listView->setGeometry(QRect(0, 100, 221, 611));
     listView->setContextMenuPolicy(Qt::CustomContextMenu);
     listView->setMaximumWidth(300);

     treeModel = new QTreeView(Dialog);
     treeModel->setObjectName(QString::fromUtf8("treeModel"));


     QVBoxLayout* listLayout = new QVBoxLayout();
     listLayout->addWidget(tagmngrLabel);
     listLayout->addWidget(listView);

     treeWindow->setCentralWidget(treeModel);

     thirdLine->addLayout(listLayout);
     thirdLine->setStretchFactor(listLayout,3);
     thirdLine->addWidget(treeWindow);
     thirdLine->setStretchFactor(treeWindow,9);

     mainLayout->addLayout(firstLine);
     mainLayout->addLayout(thirdLine);

     this->mainWidget()->setLayout(mainLayout);

 }
