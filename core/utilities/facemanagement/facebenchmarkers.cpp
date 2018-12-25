/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Integrated, multithread face detection / recognition
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "facebenchmarkers.h"

// Local includes

#include "digikam_debug.h"
#include "tagscache.h"

namespace Digikam
{

DetectionBenchmarker::DetectionBenchmarker(FacePipeline::Private* const d)
    : totalImages(0),
      faces(0),
      totalPixels(0),
      facePixels(0),

      trueNegativeImages(0),
      falsePositiveImages(0),

      truePositiveFaces(0),
      falseNegativeFaces(0),
      falsePositiveFaces(0),
      d(d)
{
}

void DetectionBenchmarker::process(FacePipelineExtendedPackage::Ptr package)
{
    if (package->databaseFaces.isEmpty())
    {
        // Detection / Recognition
        qCDebug(DIGIKAM_GENERAL_LOG) << "Benchmarking image" << package->info.name();

        FaceUtils utils;
        QList<FaceTagsIface> groundTruth = utils.databaseFaces(package->info.id());

        QList<FaceTagsIface> testedFaces = utils.toFaceTagsIfaces(package->info.id(), package->detectedFaces,
                                                                  package->recognitionResults, package->image.originalSize());

        QList<FaceTagsIface> unmatchedTrueFaces   = groundTruth;
        QList<FaceTagsIface> unmatchedTestedFaces = testedFaces;
        QList<FaceTagsIface> matchedTrueFaces;

        int trueFaces                             = groundTruth.size();
        const double minOverlap                   = 0.75;

        qCDebug(DIGIKAM_GENERAL_LOG) << "There are" << trueFaces << "faces to be detected. The detector found" << testedFaces.size();

        ++totalImages;
        faces       += trueFaces;
        totalPixels += package->image.originalSize().width() * package->image.originalSize().height();

        foreach (const FaceTagsIface& trueFace, groundTruth)
        {
            ++faces;
            QRect rect  = trueFace.region().toRect();
            facePixels += rect.width() * rect.height();

            foreach (const FaceTagsIface& testedFace, testedFaces)
            {
                if (trueFace.region().intersects(testedFace.region(), minOverlap))
                {
                    matchedTrueFaces << trueFace;
                    unmatchedTrueFaces.removeOne(trueFace);
                    break;
                }
            }
        }

        foreach (const FaceTagsIface& testedFace, testedFaces)
        {
            foreach (const FaceTagsIface& trueFace, groundTruth)
            {
                if (trueFace.region().intersects(testedFace.region(), minOverlap))
                {
                    unmatchedTestedFaces.removeOne(testedFace);
                    break;
                }
            }
        }

        if (groundTruth.isEmpty())
        {
            if (testedFaces.isEmpty())
            {
                ++trueNegativeImages;
            }
            else
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "The image, truly without faces, is false-positive";
                ++falsePositiveImages;
            }
        }

        truePositiveFaces  += matchedTrueFaces.size();
        falseNegativeFaces += unmatchedTrueFaces.size();
        falsePositiveFaces += unmatchedTestedFaces.size();
        qCDebug(DIGIKAM_GENERAL_LOG) << "Faces detected correctly:" << matchedTrueFaces.size() << ", faces missed:" << unmatchedTrueFaces.size()
                                     << ", faces falsely detected:" << unmatchedTestedFaces.size();
    }

    package->processFlags  |= FacePipelinePackage::WrittenToDatabase;
    emit processed(package);
}

// NOTE: Bench performance code. No need i18n here
QString DetectionBenchmarker::result() const
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Per-image:" << trueNegativeImages << falsePositiveFaces;
    qCDebug(DIGIKAM_GENERAL_LOG) << "Per-face:" << truePositiveFaces << falseNegativeFaces << falsePositiveFaces; // 26 7 1
    int negativeImages = trueNegativeImages + falsePositiveImages;
    int trueFaces      = truePositiveFaces  + falseNegativeFaces;
    QString specificityWarning, sensitivityWarning;

    if (negativeImages < 0.2 * totalImages)
    {
        specificityWarning = QString::fromUtf8("<p><b>Note:</b><br/> "
                                     "Only %1 of the %2 test images have <i>no</i> depicted faces. "
                                     "This means the result is cannot be representative; "
                                     "it can only be used to compare preselected collections, "
                                     "and the specificity and false-positive rate have little meaning. </p>")
                                     .arg(negativeImages).arg(totalImages);
        negativeImages     = qMax(negativeImages, 1);
    }

    if (trueFaces == 0)
    {
        sensitivityWarning = QString::fromUtf8("<p><b>Note:</b><br/> "
                                     "No picture in the test collection contained a face. "
                                     "This means that sensitivity and PPV have no meaning and will be zero. </p>");
        trueFaces          = 1;
    }

    // collection properties
    double pixelCoverage     = facePixels                  / totalPixels;
    // per-image
    double specificity       = double(trueNegativeImages)  / negativeImages;
    double falsePositiveRate = double(falsePositiveImages) / negativeImages;
    // per-face
    double sensitivity       = double(truePositiveFaces)   / trueFaces;
    double ppv               = double(truePositiveFaces)   / (truePositiveFaces + falsePositiveFaces);

    return QString::fromUtf8("<p>"
                             "<u>Collection Properties:</u><br/>"
                             "%1 Images <br/>"
                             "%2 Faces <br/>"
                             "%3% of pixels covered by faces <br/>"
                             "</p>"
                             "%8"
                             "%9"
                             "<p>"
                             "<u>Per-Image Performance:</u> <br/>"
                             "Specificity: %4% <br/>"
                             "False-Positive Rate: %5%"
                             "</p>"
                             "<p>"
                             "<u>Per-Face Performance:</u> <br/>"
                             "Sensitivity: %6% <br/>"
                             "Positive Predictive Value: %7% <br/>"
                             "</p>"
                             "<p>"
                             "In other words, if a face is detected as face, it will "
                             "with a probability of %7% truly be a face. "
                             "Of all true faces, %6% will be detected. "
                             "Given face with no images on it, the detector will with a probability "
                             "of %5% falsely find a face on it. "
                             "</p>")
                             .arg(totalImages).arg(faces).arg(pixelCoverage * 100, 0, 'f', 1)
                             .arg(specificity * 100, 0, 'f', 1).arg(falsePositiveRate * 100, 0, 'f', 1)
                             .arg(sensitivity * 100, 0, 'f', 1).arg(ppv * 100, 0, 'f', 1)
                             .arg(specificityWarning).arg(sensitivityWarning);
}

// ----------------------------------------------------------------------------------------

RecognitionBenchmarker::Statistics::Statistics()
    : knownFaces(0),
      correctlyRecognized(0)
{
}

RecognitionBenchmarker::RecognitionBenchmarker(FacePipeline::Private* const d)
    : d(d)
{
}

// NOTE: Bench performance code. No need i18n here
QString RecognitionBenchmarker::result() const
{
    int totalImages = 0;

    foreach (const Statistics& stat, results)
    {
        totalImages += stat.knownFaces;
    }

    QString s = QString::fromUtf8("<p>"
                        "<u>Collection Properties:</u><br/>"
                        "%1 Images <br/>"
                        "%2 Identities <br/>"
                        "</p><p>").arg(totalImages).arg(results.size());

    for (QMap<int, Statistics>::const_iterator it = results.begin() ; it != results.end() ; ++it)
    {
        const Statistics& stat = it.value();
        double correctRate     = double(stat.correctlyRecognized) / stat.knownFaces;
        s                     += TagsCache::instance()->tagName(it.key());
        s                     += QString::fromUtf8(": %1 faces, %2 (%3%) correctly recognized<br/>")
                                 .arg(stat.knownFaces).arg(stat.correctlyRecognized).arg(correctRate * 100);
    }

    s += QLatin1String("</p>");
    return s;
}

void RecognitionBenchmarker::process(FacePipelineExtendedPackage::Ptr package)
{
    FaceUtils utils;

    for (int i = 0 ; i < package->databaseFaces.size() ; ++i)
    {
        Identity identity  = utils.identityForTag(package->databaseFaces[i].tagId(), database);
        Statistics& result = results[package->databaseFaces[i].tagId()];
        result.knownFaces++;

        if (identity == package->recognitionResults[i])
        {
            result.correctlyRecognized++;
        }
    }

    emit processed(package);
}

} // namespace Digikam
