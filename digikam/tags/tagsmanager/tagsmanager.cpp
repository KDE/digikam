#include "tagsmanager.h"
#include <kdebug.h>
#include <klocale.h>
#include <QVBoxLayout>
#include <QHBoxLayout>

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

     tagPixmap = new QLabel(Dialog);
     //tagPixmap->setGeometry(QRect(10, 10, 31, 31));
     tagPixmap->setText("Tag Pixmap");
     //tagPixmap->setScaledContents(true);
     tagPixmap->setMaximumWidth(40);
     lineEdit = new QLineEdit(Dialog);
     lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
     lineEdit->setText(i18n("Search..."));
     lineEdit->setMaximumWidth(200);
     //lineEdit->setGeometry(QRect(60, 10, 211, 41));

     digikamPixmap = new QLabel(Dialog);
     //digikamPixmap->setGeometry(QRect(50, 10, 900, 41));
     digikamPixmap->setAutoFillBackground(false);
     digikamPixmap->setText("Digikam pixmap");
     digikamPixmap->setAlignment(Qt::AlignRight);

     firstLine->addWidget(tagPixmap);
     firstLine->setAlignment(tagPixmap,Qt::AlignLeft);
     firstLine->addWidget(lineEdit);
     firstLine->setAlignment(tagPixmap,Qt::AlignLeft);
     firstLine->addWidget(digikamPixmap);
     firstLine->setAlignment(tagPixmap,Qt::AlignRight);

     QHBoxLayout* secondLine = new QHBoxLayout(Dialog);

     tagmngrLabel = new QLabel(Dialog);
     tagmngrLabel->setObjectName(QString::fromUtf8("label"));
     tagmngrLabel->setText(i18n("Tags Manager"));
     QFont font2;
     font2.setPointSize(12);
     font2.setBold(true);
     font2.setWeight(75);
     tagmngrLabel->setFont(font2);
     tagmngrLabel->setAutoFillBackground(true);
     tagmngrLabel->setAlignment(Qt::AlignCenter);

     addBttn = new QToolButton(Dialog);
     addBttn->setObjectName(QString::fromUtf8("toolButton_2"));
     //addBttn->setGeometry(QRect(220, 70, 31, 24));
     QIcon icon9;
     icon9.addFile(QString::fromUtf8("plus.png"), QSize(), QIcon::Normal, QIcon::Off);
     addBttn->setIcon(icon9);

     removeBttn = new QToolButton(Dialog);
     removeBttn->setObjectName(QString::fromUtf8("toolButton_21"));
     //removeBttn->setGeometry(QRect(260, 70, 31, 24));

     QIcon icon10;
     icon10.addFile(QString::fromUtf8("remove.png"), QSize(), QIcon::Normal, QIcon::Off);
     removeBttn->setIcon(icon10);

     organizeBox = new QComboBox(Dialog);
     QIcon icon6;
     icon6.addFile(QString::fromUtf8("1362446149_blockdevice.png"), QSize(), QIcon::Normal, QIcon::Off);
     organizeBox->addItem(icon6, QString());
     organizeBox->addItem(QString());
     organizeBox->setObjectName(QString::fromUtf8("comboBox"));
     organizeBox->setItemText(0, i18n("Organize"));
     organizeBox->setItemText(1, i18n("New Tag"));
     //organizeBox->setGeometry(QRect(300, 70, 111, 24));

     syncExportBox = new QComboBox(Dialog);
     QIcon icon7;
     icon7.addFile(QString::fromUtf8("1362444297_database.png"), QSize(), QIcon::Normal, QIcon::On);
     syncExportBox->addItem(icon7, QString());
     syncExportBox->setItemText(0, i18n("Sync & Export"));
     //syncExportBox->setGeometry(QRect(420, 70, 141, 24));

     pushButton = new QPushButton(Dialog);
     pushButton->setObjectName(QString::fromUtf8("pushButton"));
     pushButton->setText(i18n("Tag Properties"));
     //pushButton->setGeometry(QRect(800, 70, 141, 25));
     QIcon icon8;
     icon8.addFile(QString::fromUtf8("1362446779_desktop.png"), QSize(), QIcon::Normal, QIcon::Off);
     pushButton->setIcon(icon8);
     pushButton->setDefault(true);

     //secondLine->addWidget(tagmngrLabel);
     secondLine->addWidget(addBttn);
     secondLine->addWidget(removeBttn);
     secondLine->addWidget(organizeBox);
     secondLine->addWidget(syncExportBox);
     secondLine->addWidget(pushButton);

     QHBoxLayout* thirdLine = new QHBoxLayout(Dialog);
     listView = new QListView(Dialog);
     listView->setObjectName(QString::fromUtf8("listView"));
     //listView->setGeometry(QRect(0, 100, 221, 611));
     listView->setContextMenuPolicy(Qt::CustomContextMenu);
     listView->setMaximumWidth(300);

     treeModel = new QTreeView(Dialog);
     treeModel->setObjectName(QString::fromUtf8("treeModel"));
     //treeModel->setGeometry(QRect(220, 100, 721, 611));


     QVBoxLayout* listLayout = new QVBoxLayout();
     listLayout->addWidget(tagmngrLabel);
     listLayout->addWidget(listView);

     QVBoxLayout* treeLayout = new QVBoxLayout();
     treeLayout->addLayout(secondLine);
     treeLayout->addWidget(treeModel);

     thirdLine->addLayout(listLayout);
     thirdLine->setStretchFactor(listLayout,3);
     thirdLine->addLayout(treeLayout);
     thirdLine->setStretchFactor(treeLayout,9);

     mainLayout->addLayout(firstLine);
     //mainLayout->addLayout(secondLine);
     mainLayout->addLayout(thirdLine);

     this->mainWidget()->setLayout(mainLayout);

 }
