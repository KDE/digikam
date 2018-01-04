/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-08-08
 * Description : Base functions for dnn module, can be used for face recognition, 
 *               all codes are ported from dlib library (http://dlib.net/)
 *
 * Copyright (C) 2006-2016 by Davis E. King <davis at dlib dot net>
 * Copyright (C) 2017      by Yingjie Liu <yingjiewudi at gmail dot com>
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DLIB_DNn_VALIDATION_H_
#define DLIB_DNn_VALIDATION_H_

//#include "../svm/cross_validate_object_detection_trainer_abstract.h"
//#include "../svm/cross_validate_object_detection_trainer.h"
#include "layers.h"

//namespace dlib
//{

inline unsigned long number_of_truth_hits (
            const std::vector<full_object_detection>& truth_boxes,
            const std::vector<rectangle>& ignore,
            const std::vector<std::pair<double,rectangle> >& boxes,
            const test_box_overlap& overlap_tester,
            std::vector<std::pair<double,bool> >& all_dets,
            unsigned long& missing_detections
        )
        /*!
            ensures
                - returns the number of elements in truth_boxes which are overlapped by an
                  element of boxes.  In this context, two boxes, A and B, overlap if and only if
                  overlap_tester(A,B) == true.
                - No element of boxes is allowed to account for more than one element of truth_boxes.
                - The returned number is in the range [0,truth_boxes.size()]
                - Adds the score for each box from boxes into all_dets and labels each with
                  a bool indicating if it hit a truth box.  Note that we skip boxes that
                  don't hit any truth boxes and match an ignore box.
                - Adds the number of truth boxes which didn't have any hits into
                  missing_detections.
        !*/
        {
            if (boxes.size() == 0)
            {
                missing_detections += truth_boxes.size();
                return 0;
            }

            unsigned long count = 0;
            std::vector<bool> used(boxes.size(),false);
            for (unsigned long i = 0; i < truth_boxes.size(); ++i)
            {
                bool found_match = false;
                // Find the first box that hits truth_boxes[i]
                for (unsigned long j = 0; j < boxes.size(); ++j)
                {
                    if (used[j])
                        continue;

                    if (overlap_tester(truth_boxes[i].get_rect(), boxes[j].second))
                    {
                        used[j] = true;
                        ++count;
                        found_match = true;
                        break;
                    }
                }

                if (!found_match)
                    ++missing_detections;
            }

            for (unsigned long i = 0; i < boxes.size(); ++i)
            {
                // only out put boxes if they match a truth box or are not ignored.
                if (used[i] || !overlaps_any_box(overlap_tester, ignore, boxes[i].second))
                {
                    all_dets.push_back(std::make_pair(boxes[i].first, used[i]));
                }
            }

            return count;
        }

template <typename T, typename alloc>
    double average_precision (
        const std::vector<T,alloc>& items,
        unsigned long missing_relevant_items = 0
    )
    {
        using namespace impl;
        double relevant_count = 0;
        // find the precision values
        std::vector<double> precision;
        for (unsigned long i = 0; i < items.size(); ++i)
        {
            if (get_bool_part(items[i]))
            {
                ++relevant_count;
                precision.push_back(relevant_count / (i+1));
            }
        }

        double precision_sum = 0;
        double max_val = 0;
        // now sum over the interpolated precision values
        for (std::vector<double>::reverse_iterator i = precision.rbegin(); i != precision.rend(); ++i)
        {
            max_val = std::max(max_val, *i);
            precision_sum += max_val;
        }


        relevant_count += missing_relevant_items;

        if (relevant_count != 0)
            return precision_sum/relevant_count;
        else
            return 1;
    }

    template <
        typename SUBNET,
        typename image_array_type
        >
    const matrix<double,1,3> test_object_detection_function (
        loss_mmod<SUBNET>& detector,
        const image_array_type& images,
        const std::vector<std::vector<mmod_rect>>& truth_dets,
        const test_box_overlap& overlap_tester = test_box_overlap(),
        const double adjust_threshold = 0
    )
    {
        // make sure requires clause is not broken
        DLIB_CASSERT( is_learning_problem(images,truth_dets) == true , 
                    "\t matrix test_object_detection_function()"
                    << "\n\t invalid inputs were given to this function"
                    << "\n\t is_learning_problem(images,truth_dets): " << is_learning_problem(images,truth_dets)
                    << "\n\t images.size(): " << images.size() 
                    );



        double correct_hits = 0;
        double total_true_targets = 0;

        std::vector<std::pair<double,bool> > all_dets;
        unsigned long missing_detections = 0;

        resizable_tensor temp;

        for (unsigned long i = 0; i < images.size(); ++i)
        {
            std::vector<mmod_rect> hits; 
            detector.to_tensor(&images[i], &images[i]+1, temp);
            detector.subnet().forward(temp);
            detector.loss_details().to_label(temp, detector.subnet(), &hits, adjust_threshold);


            std::vector<full_object_detection> truth_boxes;
            std::vector<rectangle> ignore;
            std::vector<std::pair<double,rectangle>> boxes;
            // copy hits and truth_dets into the above three objects
            for (auto&& b : truth_dets[i])
            {
                if (b.ignore)
                    ignore.push_back(b);
                else
                    truth_boxes.push_back(full_object_detection(b.rect));
            }
            for (auto&& b : hits)
                boxes.push_back(std::make_pair(b.detection_confidence, b.rect));

            correct_hits += number_of_truth_hits(truth_boxes, ignore, boxes, overlap_tester, all_dets, missing_detections);
            total_true_targets += truth_boxes.size();
        }

        std::sort(all_dets.rbegin(), all_dets.rend());

        double precision, recall;

        double total_hits = all_dets.size();

        if (total_hits == 0)
            precision = 1;
        else
            precision = correct_hits / total_hits;

        if (total_true_targets == 0)
            recall = 1;
        else
            recall = correct_hits / total_true_targets;

        matrix<double, 1, 3> res;
        res = precision, recall, average_precision(all_dets, missing_detections);
        return res;
    }

// ----------------------------------------------------------------------------------------

//}

#endif // DLIB_DNn_VALIDATION_H_

