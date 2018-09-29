/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2010-03-03
 * Description : LBPH interface.
 *
 * Copyright (C)      2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2013 by Mahesh Hegde <maheshmhegade at gmail dot com>
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

#include "lbphfacemodel.h"

// Qt includes

#include <QList>

// local includes

#include "digikam_debug.h"

namespace Digikam
{

LBPHistogramMetadata::LBPHistogramMetadata()
    : databaseId(0),
      identity(0),
      storageStatus(Created)
{
}

LBPHistogramMetadata::~LBPHistogramMetadata()
{
}

// ------------------------------------------------------------------------------------

LBPHFaceModel::LBPHFaceModel()
    : cv::Ptr<LBPHFaceRecognizer>(LBPHFaceRecognizer::create()),
      databaseId(0)
{
    ptr()->setThreshold(100.0);
}

LBPHFaceModel::~LBPHFaceModel()
{
}

LBPHFaceRecognizer* LBPHFaceModel::ptr()
{
    LBPHFaceRecognizer* const ptr = cv::Ptr<LBPHFaceRecognizer>::operator Digikam::LBPHFaceRecognizer*();

    if (!ptr)
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "LBPHFaceRecognizer pointer is null";

    return ptr;
}

const LBPHFaceRecognizer* LBPHFaceModel::ptr() const
{
    const LBPHFaceRecognizer* const ptr = cv::Ptr<LBPHFaceRecognizer>::operator Digikam::LBPHFaceRecognizer*();

    if (!ptr)
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "LBPHFaceRecognizer pointer is null";

    return ptr;
}

int LBPHFaceModel::radius() const
{
    return ptr()->getRadius();
}

void LBPHFaceModel::setRadius(int radius)
{
    ptr()->setRadius(radius);
}

int LBPHFaceModel::neighbors() const
{
    return ptr()->getNeighbors();
}

void LBPHFaceModel::setNeighbors(int neighbors)
{
    ptr()->setNeighbors(neighbors);
}

int LBPHFaceModel::gridX() const
{
    return ptr()->getGrid_x();
}

void LBPHFaceModel::setGridX(int grid_x)
{
    ptr()->setGrid_x(grid_x);
}

int LBPHFaceModel::gridY() const
{
    return ptr()->getGrid_y();
}

void LBPHFaceModel::setGridY(int grid_y)
{
    ptr()->setGrid_y(grid_y);
}

OpenCVMatData LBPHFaceModel::histogramData(int index) const
{
    return OpenCVMatData(ptr()->getHistograms().at(index));
}

QList<LBPHistogramMetadata> LBPHFaceModel::histogramMetadata() const
{
    return m_histogramMetadata;
}

void LBPHFaceModel::setWrittenToDatabase(int index, int id)
{
    m_histogramMetadata[index].databaseId    = id;
    m_histogramMetadata[index].storageStatus = LBPHistogramMetadata::InDatabase;
}

void LBPHFaceModel::setHistograms(const QList<OpenCVMatData>& histograms, const QList<LBPHistogramMetadata>& histogramMetadata)
{
    /*
     * Does not work with standard OpenCV, as these two params are declared read-only in OpenCV.
     * One reason why we copied the code.
     */
    std::vector<cv::Mat> newHistograms;
    cv::Mat newLabels;
    newHistograms.reserve(histograms.size());
    newLabels.reserve(histogramMetadata.size());

    foreach (const OpenCVMatData& histogram, histograms)
    {
        newHistograms.push_back(histogram.toMat());
    }

    m_histogramMetadata.clear();

    foreach (const LBPHistogramMetadata& metadata, histogramMetadata)
    {
        newLabels.push_back(metadata.identity);
        m_histogramMetadata << metadata;
    }

    std::vector<cv::Mat> currentHistograms = ptr()->getHistograms();
    cv::Mat currentLabels                  = ptr()->getLabels();

    currentHistograms.insert(currentHistograms.end(), newHistograms.begin(), newHistograms.end());
    currentLabels.push_back(newLabels);

    ptr()->setHistograms(currentHistograms);
    ptr()->setLabels(currentLabels);

/*
    //Most cumbersome and inefficient way through a file storage which we were forced to use if we used standard OpenCV
    cv::FileStorage store(".yml", cv::FileStorage::WRITE + cv::FileStorage::MEMORY);
    // store current parameters to preserve them
    store << "radius"     << radius();
    store << "neighbors"  << neighbors();
    store << "grid_x"     << gridX();
    store << "grid_y"     << gridY();
    // Write histogram data
    store << "histograms" << "[";

    foreach (const OpenCVMatData& histogram, histograms)
    {
        store << histogram.toMat();
    }

    store << "]";
    // write matching labels
    cv::Mat labels;

    foreach (const LBPHistogramMetadata& metadata, histogramMetadata)
    {
        labels.push_back(metadata.identity);
    }

    store << "labels" << labels;
    // harvest
    cv::String yaml = store.releaseAndGetString();

    cv::FileStorage read(yaml, cv::FileStorage::READ + cv::FileStorage::MEMORY);
    ptr()->load(read);
*/
}

void LBPHFaceModel::update(const std::vector<cv::Mat>& images, const std::vector<int>& labels, const QString& context)
{
    ptr()->update(images, labels);

    // Update local information
    // We assume new labels are simply appended
    cv::Mat currentLabels = ptr()->getLabels();

    for (int i = m_histogramMetadata.size() ; i < currentLabels.rows ; i++)
    {
        LBPHistogramMetadata metadata;
        metadata.storageStatus = LBPHistogramMetadata::Created;
        metadata.identity      = currentLabels.at<int>(i);
        metadata.context       = context;
        m_histogramMetadata << metadata;
    }
}

} // namespace Digikam
