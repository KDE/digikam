/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2013-06-14
 * Description : Alignment by Image Fland Mark Aligner.
 *               Funneling for complex, realistic images
 *               using sequence of distribution fields learned from congealReal
 *               Gary B. Huang, Vidit Jain, and Erik Learned-Miller.
 *               Unsupervised joint alignment of complex images.
 *               International Conference on Computer Vision (ICCV), 2007.
 *
 * Copyright (C) 2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007 by Gary B. Huang, UMass-Amherst
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

#include "flandmarkaligner.h"

// Qt includes

#include <QStandardPaths>

// local includes

#include "digikam_debug.h"
#include "flandmark_detector.h"

namespace Digikam
{

class FlandmarkAligner::Private
{
public:

    Private()
        : model(0)
    {
    }

    void loadTrainingData(const QString& path);

    bool isLoaded() const
    {
        return model;
    }

public:

    FLANDMARK_Model* model;
};

void FlandmarkAligner::Private::loadTrainingData(const QString& path)
{
    model = flandmark_init(path.toLocal8Bit());
}

FlandmarkAligner::FlandmarkAligner()
    : d(new Private)
{
    QString modelData = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                               QLatin1String("digikam/alignment-flandmark/flandmark_model.dat"));

    if (!QFileInfo(modelData).exists())
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "Model data for Congealing/Funnel not found. Should be at" << modelData;
        return;
    }

    d->loadTrainingData(modelData);
}

FlandmarkAligner::~FlandmarkAligner()
{
    delete d;
}

cv::Mat FlandmarkAligner::align(const cv::Mat& inputImage)
{
    if (!d->isLoaded())
    {
        return inputImage;
    }

    cv::Mat image;

    // ensure it's grayscale
    if (inputImage.channels() > 1)
    {
        cvtColor(inputImage, image, CV_RGB2GRAY);
    }
    else
    {
        image = inputImage;
    }

    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Detecting" << d->model->data.options.M << "landmarks";
    QVector<double> landmarks(2*d->model->data.options.M);
    // bbox with detected face (format: top_left_col top_left_row bottom_right_col bottom_right_row)
    int bbox[]        = {30,30,120,120}; //{ 0, 0, image.cols, image.rows };
    IplImage iplImage = image;
    flandmark_detect(&iplImage, bbox, d->model, landmarks.data());

    for (int i = 0; i < d->model->data.options.M; i++)
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Landmark" << i << landmarks.at(2*i) << ", " << landmarks.at(2*i+1);
    }

    return inputImage;
}

} // namespace Digikam
