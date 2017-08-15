#include "healingclonetool.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QPoint>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "dnuminput.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "imagebrushguidewidget.h"

namespace Digikam
{
class HealingCloneTool::Private
{
public:

    Private() :
        radiusInput(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configRadiusAdjustmentEntry;

    DIntNumInput*           radiusInput;
    DDoubleNumInput*        blurPercent;
    ImageBrushGuideWidget*  previewWidget;
    EditorToolSettings*     gboxSettings;
    QPoint                  sourcePoint;
    QPoint                  destinationStartPoint;
    QPushButton*            src;
    QPushButton*            start;
    DImg                    currentImg;

};

const QString HealingCloneTool::Private::configGroupName(QLatin1String("Healing Clone Tool"));
const QString HealingCloneTool::Private::configRadiusAdjustmentEntry(QLatin1String("RadiusAdjustment"));

// --------------------------------------------------------

HealingCloneTool::HealingCloneTool(QObject * const parent)
    : EditorTool(parent),
      d(new Private)
{
    setObjectName(QLatin1String("healing clone"));
    setToolName(i18n("Healing Clone Tool"));
    setToolIcon(QIcon::fromTheme(QLatin1String("healimage")));
    setToolHelp(QLatin1String("healingclonetool.anchor"));

    d->gboxSettings  = new EditorToolSettings;
    d->previewWidget = new ImageBrushGuideWidget(0, true, ImageGuideWidget::PickColorMode);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::PreviewTargetImage);
    // --------------------------------------------------------

    QLabel* const label  = new QLabel(i18n("Brush Radius:"));
    d->radiusInput = new DIntNumInput();
    d->radiusInput->setRange(0, 50, 1);
    d->radiusInput->setDefaultValue(0);
    d->radiusInput->setWhatsThis(i18n("A radius of 0 has no effect, "
                                      "1 and above determine the brush radius "
                                      "that determines the size of parts copied in the image."));

    // --------------------------------------------------------
    QLabel* const label2  = new QLabel(i18n("Radial Blur Percent:"));
    d->blurPercent = new DDoubleNumInput();
    d->blurPercent->setRange(0,100, 0.1);
    d->blurPercent->setDefaultValue(0);
    d->blurPercent->setWhatsThis(i18n("A percent of 0 has no effect, and the prush just copies "
                                      "above than 0 represents a ratio mixing"
                                      " the destination color with source."));
    // --------------------------------------------------------
    QLabel* const label_src  = new QLabel(i18n("Source:"));
    d->src = new QPushButton(i18n("click to set"), d->gboxSettings->plainPage());
    d->start = new QPushButton(i18n("start"),d->gboxSettings->plainPage());

    // --------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* const grid = new QGridLayout( );
    grid->addWidget(label_src,      1, 0, 1, 2);
    grid->addWidget(d->src,         2, 0, 1, 2);
    grid->addWidget(d->start,       3, 0, 1, 2);
    grid->addWidget(new DLineWidget(Qt::Horizontal, d->gboxSettings->plainPage()), 5, 0, 1, 2);
    grid->addWidget(label,          6, 0, 1, 2);
    grid->addWidget(d->radiusInput, 7, 0, 1, 2);
    grid->addWidget(label2,         8, 0, 1, 2);
    grid->addWidget(d->blurPercent, 9, 0, 1, 2);
    grid->setRowStretch(10, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(grid);

    // --------------------------------------------------------

    setPreviewModeMask(PreviewToolBar::AllPreviewModes);
    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);

    // --------------------------------------------------------
    d->previewWidget->setSrcSet(false);
    connect(d->radiusInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotRadiusChanged(int)));
    connect(d->src, SIGNAL(clicked(bool)),
            d->previewWidget, SLOT(slotSrcSet()));
    connect(d->previewWidget, SIGNAL(signalClone(QPoint&,QPoint&)),
            this, SLOT(slotReplace(QPoint&,QPoint&)));

}

HealingCloneTool::~HealingCloneTool()
{
    delete d;
}

void HealingCloneTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->radiusInput->setValue(group.readEntry(d->configRadiusAdjustmentEntry, d->radiusInput->defaultValue()));
}

void HealingCloneTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configRadiusAdjustmentEntry, d->radiusInput->value());
    config->sync();
}

void HealingCloneTool::finalRendering()
{
    //qApp->setOverrideCursor( Qt::WaitCursor );

    ImageIface iface;
    DImg dest = d->previewWidget->imageIface()->preview();

    FilterAction action(QLatin1String("digikam:healingCloneTool"), 1);

    iface.setOriginal(i18n("healingClone"), action, dest);

    //qApp->restoreOverrideCursor();
}

void HealingCloneTool::slotResetSettings()
{
    d->radiusInput->blockSignals(true);
    d->radiusInput->slotReset();
    d->radiusInput->blockSignals(false);
}

void HealingCloneTool::slotReplace(QPoint &srcPoint, QPoint &dstPoint)
{
    ImageIface* const iface        = d->previewWidget->imageIface();
    DImg * const current           = iface->previewReference();
    clone(current, srcPoint, dstPoint, d->radiusInput->value());
    d->previewWidget->updatePreview();
}

void HealingCloneTool::slotRadiusChanged(int r)
{
    d->previewWidget->setMaskPenSize(r);
}


// the preview idea
    //DImg img = d->previewWidget->ge getOriginalRegionImage();
    //ImageIface* iface        = d->previewWidget->imageIface();
    //iface->setPreview((*iface->original()).smoothScale(iface->previewSize()));

    //
    //DImg preview = filter()->getTargetImage();
    //ImageIface* iface        = d->previewWidget->imageIface();
    //iface->setPreview(*iface->original());



void HealingCloneTool::clone(DImg * const img, QPoint &srcPoint, QPoint &dstPoint, int radius)
{
    double blurPercent = d->blurPercent->value()/100;
    for(int i = -1* radius; i < radius; i++){
        for(int j = -1* radius; j < radius; j++){
            int rPercent = (i*i) + (j*j);
            if (rPercent < (radius * radius)) // Check for inside the circle
            {
                double rP = blurPercent * rPercent/(radius * radius);
                DColor cSrc = img->getPixelColor(srcPoint.x()+i, srcPoint.y()+j);
                DColor cDst = img->getPixelColor(dstPoint.x()+i, dstPoint.y()+j);
                cSrc.multiply(1 - rP);
                cDst.multiply(rP);
                cSrc.blendAdd(cDst);
                img->setPixelColor(dstPoint.x()+i, dstPoint.y()+j, cSrc);

            }
        }
    }
}

} // namespace Digikam
