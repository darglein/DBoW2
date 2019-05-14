/**
 * File: BowVector.h
 * Date: March 2011
 * Author: Dorian Galvez-Lopez
 * Description: bag of words vector
 * License: see the LICENSE.txt file
 *
 */

#ifndef __D_T_BOW_VECTOR__
#define __D_T_BOW_VECTOR__

#include <iostream>
#include <map>
#include <math.h>
#include <vector>
namespace DBoW2
{
/// Id of words
typedef unsigned int WordId;

/// Value of a word
typedef double WordValue;

/// Id of nodes in the vocabulary treee
typedef unsigned int NodeId;

/// L-norms for normalization
enum LNorm
{
    L1,
    L2
};

/// Weighting type
enum WeightingType
{
    TF_IDF,
    TF,
    IDF,
    BINARY
};

/// Scoring type
enum ScoringType
{
    L1_NORM,
    L2_NORM,
    CHI_SQUARE,
    KL,
    BHATTACHARYYA,
    DOT_PRODUCT
};

/// Vector of words to represent images
class BowVector : public std::map<WordId, WordValue>
{
   public:
    /**
     * Adds a value to a word value existing in the vector, or creates a new
     * word with the given value
     * @param id word id to look for
     * @param v value to create the word with, or to add to existing word
     */
    void addWeight(WordId id, WordValue v)
    {
        BowVector::iterator vit = this->lower_bound(id);

        if (vit != this->end() && !(this->key_comp()(id, vit->first)))
        {
            vit->second += v;
        }
        else
        {
            this->insert(vit, BowVector::value_type(id, v));
        }
    }

    /**
     * Adds a word with a value to the vector only if this does not exist yet
     * @param id word id to look for
     * @param v value to give to the word if this does not exist
     */
    void addIfNotExist(WordId id, WordValue v)
    {
        BowVector::iterator vit = this->lower_bound(id);

        if (vit == this->end() || (this->key_comp()(id, vit->first)))
        {
            this->insert(vit, BowVector::value_type(id, v));
        }
    }

    /**
     * L1-Normalizes the values in the vector
     * @param norm_type norm used
     */
    void normalize()
    {
        double norm = 0.0;
        BowVector::iterator it;

        {
            for (it = begin(); it != end(); ++it) norm += std::abs(it->second);
        }
        if (norm > 0.0)
        {
            for (it = begin(); it != end(); ++it) it->second /= norm;
        }
    }
};

}  // namespace DBoW2

#endif
