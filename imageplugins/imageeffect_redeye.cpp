/* ============================================================
 * File  : imageeffect_redeye.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-06
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <kdialogbase.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>

#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>

#include <imageiface.h>

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>
#include <cstring>

#include "imageeffect_redeye.h"

void ImageEffect_RedEye::removeRedEye(QWidget* parent)
{
    // -- run the dlg ----------------------------------------------

    ImageEffect_RedEyeDlg dlg(parent);
    if (dlg.exec() != QDialog::Accepted)
        return;

    // -- save settings ----------------------------------------------

    bool aggressive = (dlg.result() == ImageEffect_RedEyeDlg::Aggressive);
    KConfig *config = kapp->config();
    config->setGroup("ImageViewer Settings");
    config->writeEntry("Red Eye Correction Plugin (Mild)",
                       !aggressive);
    config->sync();
    
    // -- do the actual operations -----------------------------------
    
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getSelectedData();
    int   w    = iface.selectedWidth();
    int   h    = iface.selectedHeight();

    if (!data || !w || !h)
        return;

    uint* newData = new uint[w*h];
    memcpy(newData, data, w*h*sizeof(unsigned int));

    int   r,g,b,a;

    struct channel {
        float red_gain;
        float green_gain;
        float blue_gain;
    };
    channel red_chan, green_chan, blue_chan;

    red_chan.red_gain   = 0.1;
    red_chan.green_gain = 0.6;
    red_chan.blue_gain  = 0.3;

    green_chan.red_gain   = 0.0;
    green_chan.green_gain = 1.0;
    green_chan.blue_gain  = 0.0;

    blue_chan.red_gain   = 0.0;
    blue_chan.green_gain = 0.0;
    blue_chan.blue_gain  = 1.0;

    float red_norm, green_norm, blue_norm;

    red_norm = 1.0/(red_chan.red_gain + red_chan.green_gain + red_chan.blue_gain);
    green_norm = 1.0/(green_chan.red_gain + green_chan.green_gain + green_chan.blue_gain);
    blue_norm = 1.0/(blue_chan.red_gain + blue_chan.green_gain + blue_chan.blue_gain);
    
    uint* ptr  = data;
    uint* nptr = newData;

    int r1, g1, b1;
    
    for (int i=0; i<w*h; i++) {
        
        a = (*ptr >> 24) & 0xff;
        r = (*ptr >> 16) & 0xff;
        g = (*ptr >> 8)  & 0xff;
        b = (*ptr)       & 0xff;

        if ( aggressive || r >= ( 2 * g) )
        {
            r1 = (int) QMIN(255, red_norm * (red_chan.red_gain   * r +
                                       red_chan.green_gain * g +
                                       red_chan.blue_gain  * b));
            b1 = (int) QMIN(255, green_norm * (green_chan.red_gain   * r +
                                         green_chan.green_gain * g +
                                         green_chan.blue_gain  * b));
            g1 = (int) QMIN(255, blue_norm * (blue_chan.red_gain   * r +
                                        blue_chan.green_gain * g +
                                        blue_chan.blue_gain  * b));
            
            *nptr = QMIN(int((r-g)/150.0*255.0),255) << 24 | r1 << 16 | g1 << 8 | b1;
        }

        ptr++;
        nptr++;
    }

    Imlib_Context context = imlib_context_new();
    imlib_context_push(context);

    Imlib_Image imTop = imlib_create_image_using_copied_data(w, h, newData);
    imlib_context_set_image(imTop);
    imlib_image_set_has_alpha(1);

    // blurring usually gives better results, but users might complain
    // about loss of sharpness
    //imlib_image_blur(1);

    Imlib_Image imBot = imlib_create_image_using_copied_data(w, h, data);
    imlib_context_set_image(imBot);

    imlib_blend_image_onto_image(imTop, 0, 0, 0, w, h, 0, 0, w, h);

    ptr = imlib_image_get_data_for_reading_only();
    memcpy(data, ptr, w*h*sizeof(unsigned int));

    imlib_context_set_image(imTop);
    imlib_free_image_and_decache();
    
    imlib_context_set_image(imBot);
    imlib_free_image_and_decache();

    imlib_context_pop();
    imlib_context_free(context);
    
    delete [] newData;
    
    iface.putSelectedData(data);

    delete [] data;
}

ImageEffect_RedEyeDlg::ImageEffect_RedEyeDlg(QWidget* parent)
    : KDialogBase(Plain, i18n("Red Eye Correction"),
                  Ok|Cancel, Ok, parent, 0, true, true)
{
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint());

    QVButtonGroup* buttonGroup =
        new QVButtonGroup( i18n("Level of Red Eye Correction"),
                           plainPage() );
    buttonGroup->setRadioButtonExclusive( true );

    QRadioButton* mildBtn =
        new QRadioButton( i18n("Mild (Use if other parts of the face are also selected)"),
                          buttonGroup );
    QRadioButton* aggrBtn =
        new QRadioButton( i18n("Aggressive (Use if Eye(s) have been selected exactly)" ),
                          buttonGroup ); 

    topLayout->addWidget( buttonGroup );

    connect( buttonGroup, SIGNAL(clicked(int)),
             SLOT(slotClicked(int)) );
    
    bool mild;
    
    KConfig *config = kapp->config();
    config->setGroup("ImageViewer Settings");
    mild = config->readBoolEntry("Red Eye Correction Plugin (Mild)", true);

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
    
#include "imageeffect_redeye.moc"
