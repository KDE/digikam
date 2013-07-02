#include "tagpropwidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <kkeysequencewidget.h>
#include <kseparator.h>
#include <kicon.h>
#include <kstandarddirs.h>
#include "searchtextbar.h"

namespace Digikam
{

class TagPropWidget::PrivateTagProp
{
public:

    PrivateTagProp()
    {
        titleEdit       = 0;
        iconButton      = 0;
        resetIconButton = 0;
       // mainRootAlbum   = 0;
        topLabel        = 0;
        keySeqWidget    = 0;
        create          = false;
    }

    bool                create;

    QLabel*             topLabel;

    QString             icon;

    QPushButton*        iconButton;
    QPushButton*        resetIconButton;

    KKeySequenceWidget* keySeqWidget;

    //TAlbum*             mainRootAlbum;
    SearchTextBar*      titleEdit;
};

TagPropWidget::TagPropWidget(QWidget* const parent)
    : QWidget(parent), d(new PrivateTagProp())
{
    //QVBoxLayout* layout = new QVBoxLayout(this);

    //layout->addWidget(new QLabel("Tag Properties"));
    //layout->addWidget(new QLabel("Tag Properties"));
/*
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("tagscreation.anchor", "digikam");
*/

    //QWidget* const this = new QWidget(this);
    //setMainWidget(this);

    // --------------------------------------------------------

    QGridLayout* const grid = new QGridLayout(this);
    QLabel* const logo      = new QLabel(this);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                    .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->topLabel = new QLabel(this);
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);

    KSeparator* const line = new KSeparator(Qt::Horizontal, this);

    // --------------------------------------------------------

    QLabel* const titleLabel = new QLabel(this);
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit = new SearchTextBar(this, "TagEditDlgTitleEdit", i18n("Enter tag name here..."));
    d->titleEdit->setCaseSensitive(false);
    titleLabel->setBuddy(d->titleEdit);

    QLabel* const tipLabel = new QLabel(this);
    tipLabel->setTextFormat(Qt::RichText);
    tipLabel->setWordWrap(true);
/**
    if (d->create)
    {
        AlbumList tList = AlbumManager::instance()->allTAlbums();

        for (AlbumList::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
        {
            TAlbum* const tag = static_cast<TAlbum*>(*it);
            d->titleEdit->completionObject()->addItem(tag->tagPath());
        }
    }
    else
    {
        d->titleEdit->setText(d->mainRootAlbum->title());
        tipLabel->hide();
    }
*/
    // --------------------------------------------------------

    QLabel* const iconTextLabel = new QLabel(this);
    iconTextLabel->setText(i18n("&Icon:"));

    d->iconButton         = new QPushButton(this);
    d->iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(d->iconButton);

    // In create mode, by default assign the icon of the parent (if not root) to this new tag.
    //d->icon = album->icon();

    //d->iconButton->setIcon(SyncJob::getTagThumbnail(album));

    d->resetIconButton = new QPushButton(KIcon("view-refresh"), i18n("Reset"), this);


     //   d->resetIconButton->hide();


    // --------------------------------------------------------

    QLabel* const kscTextLabel = new QLabel(this);
    kscTextLabel->setText(i18n("&Shortcut:"));

    d->keySeqWidget      = new KKeySequenceWidget(this);
    kscTextLabel->setBuddy(d->keySeqWidget);
    // Do not inherit tag shortcut, only creates a conflict shortcut, see bug 309558.
    //KShortcut ksc(album->property(TagPropertyName::tagKeyboardShortcut()));
    //d->keySeqWidget->setKeySequence(ksc.primary(), KKeySequenceWidget::NoValidate);
    //d->keySeqWidget->setCheckActionCollections(TagsActionMngr::defaultManager()->actionCollections());

    QLabel* const tipLabel2 = new QLabel(this);
    tipLabel2->setTextFormat(Qt::RichText);
    tipLabel2->setWordWrap(true);
    tipLabel2->setText(i18n("<p><b>Note</b>: this shortcut can be used to assign or unassign tag to items.</p>"));

    // --------------------------------------------------------

    grid->addWidget(logo,               0, 0, 1, 1);
    grid->addWidget(d->topLabel,        0, 1, 1, 4);
    grid->addWidget(line,               1, 0, 1, 4);
    grid->addWidget(tipLabel,           2, 0, 1, 4);
    grid->addWidget(titleLabel,         3, 0, 1, 1);
    grid->addWidget(d->titleEdit,       3, 1, 1, 3);
    grid->addWidget(iconTextLabel,      4, 0, 1, 1);
    grid->addWidget(d->iconButton,      4, 1, 1, 1);
    grid->addWidget(d->resetIconButton, 4, 2, 1, 1);
    grid->addWidget(kscTextLabel,       5, 0, 1, 1);
    grid->addWidget(d->keySeqWidget,    5, 1, 1, 3);
    grid->addWidget(tipLabel2,          6, 0, 1, 4);
    grid->setRowStretch(7, 10);
    grid->setColumnStretch(3, 10);
    grid->setMargin(0);
    //grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------
/**
    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChanged()));

    connect(d->resetIconButton, SIGNAL(clicked()),
            this, SLOT(slotIconResetClicked()));

    connect(d->titleEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotTitleChanged(QString)));
*/
    // --------------------------------------------------------

    //slotTitleChanged(d->titleEdit->text());
    //d->titleEdit->setFocus();
    adjustSize();
}

}
