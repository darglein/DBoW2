/**
 * File: ScoringObject.h
 * Date: November 2011
 * Author: Dorian Galvez-Lopez
 * Description: functions to compute bow scores
 * License: see the LICENSE.txt file
 *
 */

#ifndef __D_T_SCORING_OBJECT__
#define __D_T_SCORING_OBJECT__

#include "BowVector.h"

#include <cfloat>

namespace DBoW2
{
class L1Scoring
{
   public:
    static constexpr bool mustNormalize = true;
    static inline double LOG_EPS        = log(DBL_EPSILON);
    /** \
     * Computes score between two vectors \
     * @param v \
     * @param w \
     * @return score between v and w \
     */
    static double score(const BowVector& v1, const BowVector& v2)
    {
        BowVector::const_iterator v1_it, v2_it;
        const BowVector::const_iterator v1_end = v1.end();
        const BowVector::const_iterator v2_end = v2.end();

        v1_it = v1.begin();
        v2_it = v2.begin();

        double score = 0;

        while (v1_it != v1_end && v2_it != v2_end)
        {
            const WordValue& vi = v1_it->second;
            const WordValue& wi = v2_it->second;

            if (v1_it->first == v2_it->first)
            {
                score += fabs(vi - wi) - fabs(vi) - fabs(wi);

                // move v1 and v2 forward
                ++v1_it;
                ++v2_it;
            }
            else if (v1_it->first < v2_it->first)
            {
                // move v1 forward
                v1_it = v1.lower_bound(v2_it->first);
                // v1_it = (first element >= v2_it.id)
            }
            else
            {
                // move v2 forward
                v2_it = v2.lower_bound(v1_it->first);
                // v2_it = (first element >= v1_it.id)
            }
        }

        // ||v - w||_{L1} = 2 + Sum(|v_i - w_i| - |v_i| - |w_i|)
        //		for all i | v_i != 0 and w_i != 0
        // (Nister, 2006)
        // scaled_||v - w||_{L1} = 1 - 0.5 * ||v - w||_{L1}
        score = -score / 2.0;

        return score;  // [0..1]
    }

    //    /** \
//     * Says if a vector must be normalized according to the scoring function \
//     * @param norm (out) if true, norm to use
    //     * @return true iff vectors must be normalized \
//     */
    //    static bool mustNormalize(LNorm& norm)
    //    {
    //        norm = L1;
    //        return true;
    //    }
};



}  // namespace DBoW2

#endif
