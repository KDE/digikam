#include "albumselectioncomponent.h"

// Qt includes

#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummodel.h"
#include "albummanager.h"
#include "albumselectcombobox.h"
#include "albumtreeview.h"
#include "searchutilities.h"

namespace Digikam
{
    
class ModelClearButton : public AnimatedClearButton
{
public:

    explicit ModelClearButton(AbstractCheckableAlbumModel* const model)
    {
        setPixmap(QIcon::fromTheme(qApp->isLeftToRight() ? QLatin1String("edit-clear-locationbar-rtl")
                                                         : QLatin1String("edit-clear-locationbar-ltr")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));
        stayVisibleWhenAnimatedOut(true);

        connect(this, SIGNAL(clicked()),
                model, SLOT(resetAllCheckedAlbums()));
    }
};

// ------------------------------------------------------------------------------------------

class SelectionComponent::Private
{
public:

    Private()
    {
        wholeElements  = 0;
        clearButton    = 0;
    }

    static const QString         configUseWholeEntry;

    QString                      configName;

    QCheckBox*                   wholeElements;
    ModelClearButton*            clearButton;
};

SelectionComponent::SelectionComponent(const QString& label, const QString& configName, QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{

}

SelectionComponent::~SelectionComponent()
{
    delete d;
}

void SelectionComponent::slotWholeAlbums(bool b)
{
    d->albumSelectCB->setEnabled(!b);
    d->clearButton->setEnabled(!b);
}

bool SelectionComponent::wholeElementsChecked() const
{
    return d->wholeElements->isChecked();
}

QList<int> SelectionComponent::selectedElementIds() const
{
    QList<int> albumIds;
    AlbumList  albums = selectedElements();

    foreach (Album* const album, albums)
    {
        albumIds << album->id();
    }

    return albumIds;
}

void SelectionComponent::setElementSelected(Album* const album, bool singleSelection)
{
    if (singleSelection)
    {
        d->albumSelectCB->model()->resetCheckedAlbums();
    }

    d->albumSelectCB->model()->setChecked(album, true);
    d->wholeElements->setChecked(false);
}

void SelectionComponent::resetSelection()
{
    if (d->albumSelection)
    {
        d->albumSelectCB->model()->resetCheckedAlbums();
    }

    if (d->tagSelection)
    {
        d->tagSelectCB->model()->resetCheckedAlbums();
    }
}

void AlbumSelectionComponent::loadState()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configName);

    if (d->albumSelection)
    {
        d->wholeAlbums->setChecked(group.readEntry(d->configUseWholeAlbumsEntry, true));
        d->albumSelectCB->view()->loadState();
        d->albumClearButton->animateVisible(!d->albumSelectCB->model()->checkedAlbums().isEmpty());

        slotWholeAlbums(wholeAlbumsChecked());
    }

    if (d->tagSelection)
    {
        d->wholeTags->setChecked(group.readEntry(d->configUseWholeTagsEntry, true));
        d->tagSelectCB->view()->loadState();
        d->tagClearButton->animateVisible(!d->tagSelectCB->model()->checkedAlbums().isEmpty());

        slotWholeTags(wholeTagsChecked());
        
    }

    if (d->selectionMode == All)
    {
        d->tabWidget->setCurrentIndex(group.readEntry(d->configAlbumTypeEntry, (int)PhysAlbum));
    }
}

void AlbumSelectionComponent::saveState()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configName);

    if (d->albumSelection)
    {
        group.writeEntry(d->configUseWholeAlbumsEntry, wholeAlbumsChecked());
        d->albumSelectCB->view()->saveState();
    }

    if (d->tagSelection)
    {
        group.writeEntry(d->configUseWholeTagsEntry, wholeTagsChecked());
        d->tagSelectCB->view()->saveState();
    }

    if (d->selectionMode == All)
    {
        group.writeEntry(d->configAlbumTypeEntry, typeSelection());
    }
}

/// ------------------------------------------------------------------

class AlbumSelectionComponent::Private
{
public:

    Private()
    {
        albumSelectCB  = 0;
    }

    AlbumTreeViewSelectComboBox* albumSelectCB;
};

AlbumSelectionComponent::AlbumSelectionComponent(const QString& label, const QString& configName, QWidget* const parent)
    : SelectionComponent(label,configName, parent),
      da(new Private)
{
    d->wholeElements   = new QCheckBox(i18n("Whole albums collection"), this);
    da->albumSelectCB = new AlbumTreeViewSelectComboBox(this);
    da->albumSelectCB->setToolTip(i18nc("@info:tooltip", "Select all albums that should be processed."));
    da->albumSelectCB->setDefaultModel();
    da->albumSelectCB->setNoSelectionText(i18nc("@info:status", "Any albums"));
    da->albumSelectCB->addCheckUncheckContextMenuActions();

    d->clearButton = new ModelClearButton(da->albumSelectCB->view()->albumModel());
    d->clearButton->setToolTip(i18nc("@info:tooltip", "Reset selected albums"));

    QGridLayout* const pAlbumsGrid = new QGridLayout(this);
    pAlbumsGrid->addWidget(d->wholeElements,     0, 0, 1, 2);
    pAlbumsGrid->addWidget(da->albumSelectCB,    1, 0, 1, 1);
    pAlbumsGrid->addWidget(d->clearButton, 1, 1, 1, 1);
    pAlbumsGrid->setSpacing(0);

    connect(d->wholeElements, SIGNAL(toggled(bool)),
            this, SLOT(slotWholeAlbums(bool)));

    connect(d->wholeElements, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSelectionChanged()));

    connect(da->albumSelectCB->view()->albumModel(), SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
            this, SLOT(slotUpdateClearButtons()));

    da->albumSelectCB->view()->setObjectName(d->configName);
    da->albumSelectCB->view()->setEntryPrefix(QLatin1String("AlbumComboBox-"));
    da->albumSelectCB->view()->setRestoreCheckState(true);
}

AlbumList AlbumSelectionComponent::selectedElements() const
{
    AlbumList albums;

    
    if (wholeElementsChecked())
    {
        albums << AlbumManager::instance()->allPAlbums();
    }
    else
    {
        albums << da->albumSelectCB->model()->checkedAlbums();
    }

    return albums;
}

/// ------------------------------------------------------------------

class TagSelectionComponent::Private
{
public:

    Private()
    {
        tagSelectCB  = 0;
    }

    TagTreeViewSelectComboBox*   tagSelectCB;
};

TagSelectionComponent::TagSelectionComponent(const QString& label, const QString& configName, QWidget* const parent)
    : SelectionComponent(label,configName, parent),
      dt(new Private)
{
    d->wholeElements   = new QCheckBox(i18n("Whole albums collection"), this);
    dt->tagSelectCB = new TagTreeViewSelectComboBox(this);
    dt->tagSelectCB->setToolTip(i18nc("@info:tooltip", "Select all albums that should be processed."));
    dt->tagSelectCB->setDefaultModel();
    dt->tagSelectCB->setNoSelectionText(i18nc("@info:status", "Any albums"));
    dt->tagSelectCB->addCheckUncheckContextMenuActions();

    d->clearButton = new ModelClearButton(dt->tagSelectCB->view()->albumModel());
    d->clearButton->setToolTip(i18nc("@info:tooltip", "Reset selected albums"));

    QGridLayout* const pAlbumsGrid = new QGridLayout(this);
    pAlbumsGrid->addWidget(d->wholeElements,     0, 0, 1, 2);
    pAlbumsGrid->addWidget(dt->tagSelectCB,    1, 0, 1, 1);
    pAlbumsGrid->addWidget(d->clearButton, 1, 1, 1, 1);
    pAlbumsGrid->setSpacing(0);

    connect(d->wholeElements, SIGNAL(toggled(bool)),
            this, SLOT(slotWholeAlbums(bool)));

    connect(d->wholeElements, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSelectionChanged()));

    connect(dt->tagSelectCB->view()->albumModel(), SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
            this, SLOT(slotUpdateClearButtons()));

    dt->tagSelectCB->view()->setObjectName(d->configName);
    dt->tagSelectCB->view()->setEntryPrefix(QLatin1String("TagComboBox-"));
    dt->tagSelectCB->view()->setRestoreCheckState(true);
}

AlbumList TagSelectionComponent::selectedElements() const
{
    AlbumList albums;

    
    if (wholeElementsChecked())
    {
        albums << AlbumManager::instance()->allPAlbums();
    }
    else
    {
        albums << dt->tagSelectCB->model()->checkedAlbums();
    }

    return albums;
}

} // namespace Digikam
