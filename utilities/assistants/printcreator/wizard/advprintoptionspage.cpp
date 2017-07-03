/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-08
 * Description : a tool to print images
 *
 * Copyright (C) 2009-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "advprintoptionspage.h"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QToolButton>
#include <QPushButton>
#include <QPainter>

// KDE includes

#include <kconfigdialogmanager.h>
#include <kconfig.h>

// Local includes

#include "advprintphoto.h"
#include "advprintimagesconfig.h"
#include "ui_advprintoptionspage.h"
#include "digikam_debug.h"
#include "signalblocker.h"

namespace Digikam
{

class AdvPrintOptionsPage::Private
    : public Ui_AdvPrintOptionsPage
{
public:

    Private()
    {
        m_parent              = 0;
        m_photos              = 0;
        m_currentPhoto        = 0;
        m_scaleGroup          = new QButtonGroup;
        m_positionGroup       = new QButtonGroup;
        m_configDialogManager = 0;
    }

public:

    QWidget*               m_parent;
    QList<AdvPrintPhoto*>* m_photos;
    int                    m_currentPhoto;
    QButtonGroup*          m_scaleGroup;
    QButtonGroup*          m_positionGroup;
    KConfigDialogManager*  m_configDialogManager;

public:

    void initPositionFrame()
    {
        mPositionFrame->setStyleSheet
        (
            QLatin1String(
            "QFrame {"
            " background-color: palette(mid);"
            " border: 1px solid palette(dark);"
            "}"
            "QToolButton {"
            " border: none;"
            " background: palette(base);"
            "}"
            "QToolButton:hover {"
            " background: palette(alternate-base);"
            " border: 1px solid palette(highlight);"
            "}"
            "QToolButton:checked {"
            " background-color: palette(highlight);"
            "}"
            )
        );

        QGridLayout* const layout = new QGridLayout ( mPositionFrame );
        layout->setMargin ( 0 );
        layout->setSpacing ( 1 );

        for ( int row = 0; row < 3; ++row )
        {
            for ( int col = 0; col < 3; ++col )
            {
                QToolButton* const button = new QToolButton ( mPositionFrame );
                button->setFixedSize ( 40, 40 );
                button->setCheckable ( true );
                layout->addWidget ( button, row, col );

                Qt::Alignment alignment;

                if ( row == 0 )
                {
                    alignment = Qt::AlignTop;
                }
                else if ( row == 1 )
                {
                    alignment = Qt::AlignVCenter;
                }
                else
                {
                    alignment = Qt::AlignBottom;
                }
                if ( col == 0 )
                {
                    alignment |= Qt::AlignLeft;
                }
                else if ( col == 1 )
                {
                    alignment |= Qt::AlignHCenter;
                }
                else
                {
                    alignment |= Qt::AlignRight;
                }

                m_positionGroup->addButton(button, int(alignment));
            }
        }
    }
};

AdvPrintOptionsPage::AdvPrintOptionsPage(QWidget* const parent,
                                         QList<AdvPrintPhoto*>* const photoList)
    : QWidget(),
      d(new Private)
{
    d->setupUi(this);
    d->m_parent              = parent;
    d->m_photos              = photoList;
    d->m_configDialogManager = new KConfigDialogManager(this, AdvPrintImagesConfig::self());

    d->initPositionFrame();

    d->m_scaleGroup->addButton(d->mNoScale, NoScale);
    d->m_scaleGroup->addButton(d->mScaleToPage, ScaleToPage);
    d->m_scaleGroup->addButton(d->mScaleTo, ScaleToCustomSize);
    d->mPhotoXPage->setRange(0, d->m_photos->size());

    d->mPX->setSpecialValueText ( i18n ( "disabled" ) );
    d->mPY->setSpecialValueText ( i18n ( "disabled" ) );
    d->mPhotoXPage->setSpecialValueText ( i18n ( "disabled" ) );

    //Change cursor to waitCursor during transition
    QApplication::setOverrideCursor ( QCursor ( Qt::WaitCursor ) );

//     showAdvPrintAdditionalInfo();
    imagePreview();
    enableButtons();
    QApplication::restoreOverrideCursor();

    connect(d->kcfg_PrintWidth, SIGNAL (valueChanged(double)),
            this, SLOT (adjustHeightToRatio()) );

    connect(d->kcfg_PrintHeight, SIGNAL (valueChanged(double)),
            this, SLOT (adjustWidthToRatio()) );

    connect(d->kcfg_PrintKeepRatio, SIGNAL (toggled(bool)),
            this, SLOT (adjustHeightToRatio()) );

    connect(d->mPhotoXPage, SIGNAL (valueChanged(int)),
            this, SLOT (photoXpageChanged(int)) );

    connect(d->mPX, SIGNAL (valueChanged(int)),
            this, SLOT (horizontalPagesChanged(int)) );

    connect(d->mPY, SIGNAL (valueChanged(int)),
            this, SLOT (verticalPagesChanged(int)) );

    connect(d->mRightButton, SIGNAL (clicked()),
            this, SLOT (selectNext()) );

    connect(d->mLeftButton, SIGNAL (clicked()),
            this, SLOT (selectPrev()) );

    connect(d->mSaveSettings, SIGNAL (clicked()),
            this, SLOT (saveConfig()) );

    connect(d->mNoScale, SIGNAL (clicked(bool)),
            this, SLOT (scaleOption()) );

    connect(d->mScaleToPage, SIGNAL (clicked(bool)),
            SLOT (scaleOption()) );

    connect(d->mScaleTo, SIGNAL (clicked(bool)),
            this, SLOT (scaleOption()) );

    connect(d->kcfg_PrintAutoRotate, SIGNAL (toggled(bool)),
            this, SLOT (autoRotate(bool)) );

    connect(d->m_positionGroup, SIGNAL (buttonClicked(int)),
            this, SLOT (positionChosen(int)));

    layout()->setMargin ( 0 );
}

AdvPrintOptionsPage::~AdvPrintOptionsPage()
{
    delete d;
}

double AdvPrintOptionsPage::unitToInches ( AdvPrintOptionsPage::Unit unit )
{
    if ( unit == AdvPrintOptionsPage::Inches )
    {
      return 1.;
    }
    else if ( unit == AdvPrintOptionsPage::Centimeters )
    {
      return 1/2.54;
    }
    else   // Millimeters
    {
      return 1/25.4;
    }
}

Qt::Alignment AdvPrintOptionsPage::alignment() const
{
    int id = d->m_positionGroup->checkedId();
//     qCDebug(DIGIKAM_GENERAL_LOG) << "alignment=" << id;

    return Qt::Alignment ( id );
}

AdvPrintOptionsPage::Unit AdvPrintOptionsPage::scaleUnit() const
{
    d->m_photos->at(d->m_currentPhoto)->m_pAddInfo->mUnit =
        AdvPrintOptionsPage::Unit(d->kcfg_PrintUnit->currentIndex());

    return AdvPrintOptionsPage::Unit(d->kcfg_PrintUnit->currentIndex());
}

double AdvPrintOptionsPage::scaleWidth() const
{
    d->m_photos->at(d->m_currentPhoto)->m_cropRegion = QRect (0, 0,
                    (int)(d->kcfg_PrintWidth->value()  * unitToInches(scaleUnit())),
                    (int)(d->kcfg_PrintHeight->value() * unitToInches(scaleUnit())));

    return d->kcfg_PrintWidth->value() * unitToInches ( scaleUnit() );
}

double AdvPrintOptionsPage::scaleHeight() const
{
    d->m_photos->at(d->m_currentPhoto)->m_cropRegion = QRect(0, 0,
                    (int)(d->kcfg_PrintWidth->value()  * unitToInches(scaleUnit())),
                    (int)(d->kcfg_PrintHeight->value() * unitToInches(scaleUnit())));

    return d->kcfg_PrintHeight->value() * unitToInches ( scaleUnit() );
}

void AdvPrintOptionsPage::adjustWidthToRatio()
{
    if ( !d->kcfg_PrintKeepRatio->isChecked() )
    {
        return;
    }

    double width = d->m_photos->at ( d->m_currentPhoto )->width() * d->kcfg_PrintHeight->value() /
                   d->m_photos->at ( d->m_currentPhoto )->height();
    d->m_photos->at ( d->m_currentPhoto )->m_pAddInfo->mPrintHeight = d->kcfg_PrintHeight->value();
    d->m_photos->at ( d->m_currentPhoto )->m_pAddInfo->mPrintWidth  =  width ? width : 1.;
    SignalBlocker blocker ( d->kcfg_PrintWidth );
    d->kcfg_PrintWidth->setValue ( d->m_photos->at ( d->m_currentPhoto )->m_pAddInfo->mPrintWidth );
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << " width "
                                 << d->m_photos->at ( d->m_currentPhoto )->pAddInfo->mPrintWidth
                                 << " height "
                                 <<  d->m_photos->at ( d->m_currentPhoto )->pAddInfo->mPrintHeight;
*/
}

void AdvPrintOptionsPage::adjustHeightToRatio()
{
    if ( !d->kcfg_PrintKeepRatio->isChecked() )
    {
      return;
    }

    double height = d->m_photos->at ( d->m_currentPhoto )->height() * d->kcfg_PrintWidth->value() / d->m_photos->at ( d->m_currentPhoto )->width();
    d->m_photos->at ( d->m_currentPhoto )->m_pAddInfo->mPrintWidth  = d->kcfg_PrintWidth->value();
    d->m_photos->at ( d->m_currentPhoto )->m_pAddInfo->mPrintHeight =  height ? height : 1. ;
    SignalBlocker blocker ( d->kcfg_PrintHeight );
    d->kcfg_PrintHeight->setValue ( d->m_photos->at ( d->m_currentPhoto )->m_pAddInfo->mPrintHeight );

/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "height "
                                 <<  d->m_photos->at ( d->m_currentPhoto )->pAddInfo->mPrintHeight
                                 << " width "
                                 << d->m_photos->at ( d->m_currentPhoto )->pAddInfo->mPrintWidth;
*/
}

void AdvPrintOptionsPage::manageQPrintDialogChanges ( QPrinter * /*printer*/ )
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "It has been called!";
}

int AdvPrintOptionsPage::photoXPage() const
{
    return d->mPhotoXPage->value();
}

int AdvPrintOptionsPage::mp_horPages() const
{
    return d->mPX->value();
}

int AdvPrintOptionsPage::mp_verPages() const
{
    return d->mPY->value();
}
bool AdvPrintOptionsPage::printUsingAtkinsLayout() const
{
    return ( d->mPhotoXPage->value() > 0 );
}

void AdvPrintOptionsPage::enableButtons()
{
    if ( d->m_photos->size() == 1 )
    {
        d->mLeftButton->setEnabled ( false );
        d->mRightButton->setEnabled ( false );
    }
    else if ( d->m_currentPhoto == 0 )
    {
        d->mLeftButton->setEnabled ( false );
        d->mRightButton->setEnabled ( true );
    }
    else if ( d->m_currentPhoto == d->m_photos->size()-1 )
    {
        d->mRightButton->setEnabled ( false );
        d->mLeftButton->setEnabled ( true );
    }
    else
    {
        d->mLeftButton->setEnabled ( true );
        d->mRightButton->setEnabled ( true );
    }
}

void AdvPrintOptionsPage::imagePreview()
{
//     qCDebug(DIGIKAM_GENERAL_LOG) << d->m_currentPhoto;

    AdvPrintPhoto* const pPhoto = d->m_photos->at ( d->m_currentPhoto );
    d->mPreview->setPixmap ( pPhoto->thumbnail() );

    if ( pPhoto->m_cropRegion != QRect() )
    {
        // TODO
    }
}

void AdvPrintOptionsPage::selectNext()
{
    //Change cursor to waitCursor during transition
    QApplication::setOverrideCursor ( QCursor ( Qt::WaitCursor ) );

//     qCDebug(DIGIKAM_GENERAL_LOG) << d->m_currentPhoto;

    d->m_photos->at ( d->m_currentPhoto )->m_pAddInfo->mPrintPosition = alignment();

    if ( d->m_currentPhoto+1 < d->m_photos->size() )
      d->m_currentPhoto++;

    showAdvPrintAdditionalInfo();
    imagePreview();
    enableButtons();
    QApplication::restoreOverrideCursor();
}

void AdvPrintOptionsPage::selectPrev()
{
    //Change cursor to waitCursor during transition
    QApplication::setOverrideCursor ( QCursor ( Qt::WaitCursor ) );

//     qCDebug(DIGIKAM_GENERAL_LOG) << d->m_currentPhoto;
    d->m_photos->at ( d->m_currentPhoto )->m_pAddInfo->mPrintPosition = alignment();

    if ( d->m_currentPhoto-1 >= 0 )
      d->m_currentPhoto--;

    showAdvPrintAdditionalInfo();
    imagePreview();
    enableButtons();
    QApplication::restoreOverrideCursor();
}

void AdvPrintOptionsPage::setAdvPrintAdditionalInfo()
{
    for (int i=0 ; i < d->m_photos->count() ; ++i)
    {
        AdvPrintPhoto* pPhoto = d->m_photos->at ( i );

        if ( pPhoto )
        {
            pPhoto->m_pAddInfo->mUnit                 = AdvPrintImagesConfig::printUnit();
            pPhoto->m_pAddInfo->mPrintPosition        = AdvPrintImagesConfig::printPosition();
            pPhoto->m_pAddInfo->mKeepRatio            = AdvPrintImagesConfig::printKeepRatio();
            pPhoto->m_pAddInfo->mScaleMode            = AdvPrintImagesConfig::printScaleMode();
            pPhoto->m_pAddInfo->mAutoRotate           = AdvPrintImagesConfig::printAutoRotate();
            pPhoto->m_pAddInfo->mPrintWidth           = AdvPrintImagesConfig::printWidth();
            pPhoto->m_pAddInfo->mPrintHeight          = AdvPrintImagesConfig::printHeight();
            pPhoto->m_pAddInfo->mEnlargeSmallerImages = AdvPrintImagesConfig::printEnlargeSmallerImages();

            if (pPhoto->m_pAddInfo->mKeepRatio)
            {
                double height = d->m_photos->at(i)->height() * pPhoto->m_pAddInfo->mPrintWidth / d->m_photos->at(i)->width();
                d->m_photos->at(i)->m_pAddInfo->mPrintHeight =  height ? height : AdvPrintImagesConfig::printHeight();
            }
/*
           qCDebug(DIGIKAM_GENERAL_LOG) << " photo "
                                        << i
                                        << " printWidth "
                                        <<  pPhoto->pAddInfo->mPrintWidth
                                        << " printHeight "
                                        << pPhoto->pAddInfo->mPrintHeight;
*/
        }
    }
}

void AdvPrintOptionsPage::showAdvPrintAdditionalInfo()
{
    QAbstractButton* button     = 0;
    int i                       = d->m_currentPhoto;
    AdvPrintPhoto* const pPhoto = d->m_photos->at ( i );

    if ( pPhoto )
    {
        d->kcfg_PrintUnit->setCurrentIndex ( pPhoto->m_pAddInfo->mUnit );
        button = d->m_positionGroup->button ( pPhoto->m_pAddInfo->mPrintPosition );

        if ( button )
        {
            button->setChecked ( true );
        }
        else
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown button for position group";
        }

        button = d->m_scaleGroup->button ( pPhoto->m_pAddInfo->mScaleMode );

        if ( button )
        {
            button->setChecked ( true );
        }
        else
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown button for scale group";
        }

        d->kcfg_PrintKeepRatio->setChecked ( pPhoto->m_pAddInfo->mKeepRatio );
        d->kcfg_PrintAutoRotate->setChecked ( pPhoto->m_pAddInfo->mAutoRotate );
        d->kcfg_PrintEnlargeSmallerImages->setChecked ( pPhoto->m_pAddInfo->mEnlargeSmallerImages );
        d->kcfg_PrintWidth->setValue ( pPhoto->m_pAddInfo->mPrintWidth );
        d->kcfg_PrintHeight->setValue ( pPhoto->m_pAddInfo->mPrintHeight );

        if ( d->kcfg_PrintKeepRatio->isChecked() )
        {
            adjustHeightToRatio();
        }
    }
}

void AdvPrintOptionsPage::loadConfig()
{
    QAbstractButton* button = 0;
    button                  = d->m_positionGroup->button ( AdvPrintImagesConfig::printPosition() );

    if ( button )
    {
        button->setChecked ( true );
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown button for position group";
    }

    button = d->m_scaleGroup->button ( AdvPrintImagesConfig::printScaleMode() );

    if ( button )
    {
        button->setChecked ( true );
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown button for scale group";
    }

    d->m_configDialogManager->updateWidgets();

    // config has been read, now we set photo additional info
    setAdvPrintAdditionalInfo();
}

void AdvPrintOptionsPage::saveConfig()
{
    int position        = d->m_positionGroup->checkedId();
    AdvPrintImagesConfig::setPrintPosition ( position );

    ScaleMode scaleMode = ScaleMode ( d->m_scaleGroup->checkedId() );
    AdvPrintImagesConfig::setPrintScaleMode ( scaleMode );

    bool checked        = d->kcfg_PrintAutoRotate->isChecked();
    AdvPrintImagesConfig::setPrintAutoRotate ( checked );

    d->m_configDialogManager->updateSettings();

    AdvPrintImagesConfig::self()->save();
}

void AdvPrintOptionsPage::photoXpageChanged ( int i )
{
    bool disabled = ( i>0 );
    d->mPositionFrame->setDisabled ( disabled );
    d->mGroupScaling->setDisabled ( disabled );
    d->mGroupImage->setDisabled ( disabled );
    d->kcfg_PrintAutoRotate->setDisabled ( disabled );
    d->mPreview->setDisabled ( disabled );

    if ( disabled )
    {
        d->mRightButton->setDisabled ( disabled );
        d->mLeftButton->setDisabled ( disabled );
        SignalBlocker block_mPX ( d->mPX );
        d->mPX->setValue ( 0 );
        SignalBlocker block_mPY ( d->mPY );
        d->mPY->setValue ( 0 );
    }
    else
    {
        enableButtons();
    }
}

void AdvPrintOptionsPage::horizontalPagesChanged ( int i )
{
    bool disabled = ( i > 0 );
    d->mPositionFrame->setDisabled ( disabled );
    d->mGroupScaling->setDisabled ( disabled );
    d->mGroupImage->setDisabled ( disabled );
    d->kcfg_PrintAutoRotate->setDisabled ( disabled );
    d->mPreview->setDisabled ( disabled );

    if ( disabled )
    {
        d->mRightButton->setDisabled ( disabled );
        d->mLeftButton->setDisabled ( disabled );
        SignalBlocker blocker ( d->mPhotoXPage );
        d->mPhotoXPage->setValue ( 0 );

        if ( d->mPY->value() == 0 )
        {
            SignalBlocker block_mPY ( d->mPY );
            d->mPY->setValue ( 1 );
        }
    }
    else
    {
        SignalBlocker block_mPX ( d->mPY );
        d->mPY->setValue ( 0 );
        enableButtons();
    }
}

void AdvPrintOptionsPage::verticalPagesChanged ( int i )
{
    bool disabled = ( i > 0 );
    d->mPositionFrame->setDisabled ( disabled );
    d->mGroupScaling->setDisabled ( disabled );
    d->mGroupImage->setDisabled ( disabled );
    d->kcfg_PrintAutoRotate->setDisabled ( disabled );
    d->mPreview->setDisabled ( disabled );

    if ( disabled )
    {
        d->mRightButton->setDisabled ( disabled );
        d->mLeftButton->setDisabled ( disabled );
        SignalBlocker blocker ( d->mPhotoXPage );
        d->mPhotoXPage->setValue ( 0 );

        if ( d->mPX->value() == 0 )
        {
            SignalBlocker block_mPX ( d->mPX );
            d->mPX->setValue ( 1 );
        }
    }
    else
    {
        SignalBlocker block_mPX ( d->mPX );
        d->mPX->setValue ( 0 );
        enableButtons();
    }
}

void AdvPrintOptionsPage::scaleOption()
{
    ScaleMode scaleMode         = ScaleMode ( d->m_scaleGroup->checkedId() );
//   qCDebug(DIGIKAM_GENERAL_LOG) << "ScaleMode " << int ( scaleMode );
    int i                       = d->m_currentPhoto;
    AdvPrintPhoto* const pPhoto = d->m_photos->at ( i );

    if (pPhoto)
    {
        pPhoto->m_pAddInfo->mScaleMode = scaleMode;
    }

    if (scaleMode == ScaleToCustomSize && d->kcfg_PrintKeepRatio->isChecked())
    {
        adjustHeightToRatio();
    }
}

void AdvPrintOptionsPage::autoRotate ( bool value )
{
    int i                       = d->m_currentPhoto;
    AdvPrintPhoto* const pPhoto = d->m_photos->at ( i );

    if ( pPhoto )
    {
        pPhoto->m_pAddInfo->mAutoRotate = value;
    }
}

void AdvPrintOptionsPage::positionChosen(int id)
{
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Current photo "
                                 << d->m_currentPhoto
                                 << "position "
                                 << id;
*/
    d->m_photos->at(d->m_currentPhoto)->m_pAddInfo->mPrintPosition = Qt::Alignment(id);
}

} // namespace Digikam
