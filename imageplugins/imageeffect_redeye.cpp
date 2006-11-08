/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-06-06
 * Description : Red eyes correction tool for image editor
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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

#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>

// KDE includes.

#include <kdialogbase.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kpassivepopup.h>

// Digikam includes.

#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"

// Local includes.

#include "imageeffect_redeye.h"

namespace DigikamImagesPluginCore
{

class RedEyePassivePopup : public KPassivePopup
{
public:

    RedEyePassivePopup(QWidget* parent)
        : KPassivePopup(parent), m_parent(parent)          
    {
    }

protected:

    virtual void positionSelf()
    {
        move(m_parent->x() + 30, m_parent->y() + 30);
    }

private:

    QWidget* m_parent;
};

void ImageEffect_RedEye::removeRedEye(QWidget* parent)
{
    // -- check if we actually have a selection --------------------

    Digikam::ImageIface iface(0, 0);
    uchar *data             = iface.getImageSelection();
    int w                   = iface.selectedWidth();
    int h                   = iface.selectedHeight();
    bool sixteenBit         = iface.originalSixteenBit();
    bool hasAlpha           = iface.originalHasAlpha();
    Digikam::DImg selection = Digikam::DImg(w, h, sixteenBit, hasAlpha ,data);
    delete [] data;

    Digikam::DImg newSelection = selection.copy();

    if (selection.isNull() || !w || !h)
    {
        RedEyePassivePopup* popup = new RedEyePassivePopup(parent);
        popup->setView(i18n("Red-Eye Correction Tool"),
                       i18n("You need to select a region including the eyes to use "
                            "the red-eye correction tool"));
        popup->setAutoDelete(true);
        popup->setTimeout(2500);
        popup->show();
        return;
    }

    // -- run the dlg ----------------------------------------------

    ImageEffect_RedEyeDlg dlg(parent);

    if (dlg.exec() != QDialog::Accepted)
        return;

    // -- save settings ----------------------------------------------

    bool aggressive = (dlg.result() == ImageEffect_RedEyeDlg::Aggressive);
    KConfig *config = kapp->config();
    config->setGroup("ImageViewer Settings");
    config->writeEntry("Red Eye Correction Plugin (Mild)", !aggressive);
    config->sync();

    // -- do the actual operations -----------------------------------

    parent->setCursor( KCursor::waitCursor() );
    
    struct channel
    {
        float red_gain;
        float green_gain;
        float blue_gain;
    };

    channel red_chan, green_chan, blue_chan;

    red_chan.red_gain     = 0.1;
    red_chan.green_gain   = 0.6;
    red_chan.blue_gain    = 0.3;

    green_chan.red_gain   = 0.0;
    green_chan.green_gain = 1.0;
    green_chan.blue_gain  = 0.0;

    blue_chan.red_gain    = 0.0;
    blue_chan.green_gain  = 0.0;
    blue_chan.blue_gain   = 1.0;

    float red_norm, green_norm, blue_norm;

    red_norm   = 1.0 / (red_chan.red_gain   + red_chan.green_gain   + red_chan.blue_gain);
    green_norm = 1.0 / (green_chan.red_gain + green_chan.green_gain + green_chan.blue_gain);
    blue_norm  = 1.0 / (blue_chan.red_gain  + blue_chan.green_gain  + blue_chan.blue_gain);

    if (!selection.sixteenBit())         // 8 bits image.
    {
        uchar* ptr  = selection.bits();
        uchar* nptr = newSelection.bits();
        uchar  r, g, b, a, r1, g1, b1;

        for (int i = 0 ; i < w * h ; i++) 
        {
            b = ptr[0];
            g = ptr[1];
            r = ptr[2];
            a = ptr[3];

            if ( aggressive || r >= ( 2 * g) )
            {
                r1 = QMIN(255, (int)(red_norm * (red_chan.red_gain   * r +
                                                 red_chan.green_gain * g +
                                                 red_chan.blue_gain  * b)));

                b1 = QMIN(255, (int)(green_norm * (green_chan.red_gain   * r +
                                                   green_chan.green_gain * g +
                                                   green_chan.blue_gain  * b)));

                g1 = QMIN(255, (int)(blue_norm * (blue_chan.red_gain   * r +
                                                  blue_chan.green_gain * g +
                                                  blue_chan.blue_gain  * b)));

                nptr[0] = b1;
                nptr[1] = g1;
                nptr[2] = r1;
                nptr[3] = QMIN( (int)((r-g) / 150.0 * 255.0), 255);
            }

            ptr += 4;
            nptr+= 4;
        }
    }
    else                                 // 16 bits image.
    {
        unsigned short* ptr  = (unsigned short*)selection.bits();
        unsigned short* nptr = (unsigned short*)newSelection.bits();
        unsigned short  r, g, b, a, r1, g1, b1;

        for (int i = 0 ; i < w * h ; i++) 
        {
            b = ptr[0];
            g = ptr[1];
            r = ptr[2];
            a = ptr[3];

            if ( aggressive || r >= ( 2 * g) )
            {
                r1 = QMIN(65535, (int)(red_norm * (red_chan.red_gain   * r +
                                                   red_chan.green_gain * g +
                                                   red_chan.blue_gain  * b)));

                b1 = QMIN(65535, (int)(green_norm * (green_chan.red_gain   * r +
                                                     green_chan.green_gain * g +
                                                     green_chan.blue_gain  * b)));

                g1 = QMIN(65535, (int)(blue_norm * (blue_chan.red_gain   * r +
                                                    blue_chan.green_gain * g +
                                                    blue_chan.blue_gain  * b)));

                nptr[0] = b1;
                nptr[1] = g1;
                nptr[2] = r1;
                nptr[3] = QMIN( (int)((r-g) / 38400.0 * 65535.0), 65535);
            }

            ptr += 4;
            nptr+= 4;
        }
    }

    // - Perform pixels blending using alpha channel to blur a little the result.

    selection.bitBlend_RGBA2RGB(newSelection, 0, 0, w, h);

    iface.putImageSelection(i18n("Red Eyes Correction"), selection.bits());
    parent->unsetCursor();
}

// -------------------------------------------------------------

ImageEffect_RedEyeDlg::ImageEffect_RedEyeDlg(QWidget* parent)
                     : KDialogBase(Plain, i18n("Red Eye Correction"),
                                   Help|Ok|Cancel, Ok, parent, 0, true, true)
{
    setHelp("redeyecorrectiontool.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QVButtonGroup* buttonGroup = new QVButtonGroup( i18n("Level of Red-Eye Correction"),
                                                    plainPage() );
    buttonGroup->setRadioButtonExclusive( true );

    QRadioButton* mildBtn = new QRadioButton( i18n("Mild (use if other parts of the face are also selected)"),
                                              buttonGroup );
    QRadioButton* aggrBtn = new QRadioButton( i18n("Aggressive (use if eye(s) have been selected exactly)" ),
                                              buttonGroup );

    topLayout->addWidget( buttonGroup );
    
    connect( buttonGroup, SIGNAL(clicked(int)),
             this, SLOT(slotClicked(int)) );

    KConfig *config = kapp->config();
    config->setGroup("ImageViewer Settings");
    bool mild = config->readBoolEntry("Red Eye Correction Plugin (Mild)", true);

    if (mild)
    {
        mildBtn->setChecked(true);
        m_selectedId = 0;
    }
    else
    {
        aggrBtn->setChecked(true);
        m_selectedId = 1;
    }
}

ImageEffect_RedEyeDlg::Result ImageEffect_RedEyeDlg::result() const
{
    return (Result)m_selectedId;
}

void ImageEffect_RedEyeDlg::slotClicked(int id)
{
    m_selectedId = id;
}

}  // NameSpace DigikamImagesPluginCore

#include "imageeffect_redeye.moc"
