/**
 * File: Demo.cpp
 * Date: November 2011
 * Author: Dorian Galvez-Lopez
 * Description: demo application of DBoW2
 * License: see the LICENSE.txt file
 */

#include <iostream>
#include <vector>

// DBoW2
#include "DBoW2.h"  // defines OrbVocabulary and OrbDatabase

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>

using namespace DBoW2;
using namespace std;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

using Descriptor = FORB::TDescriptor;

void loadFeatures(vector<vector<Descriptor>>& features);
void changeStructure(const cv::Mat& plain, vector<Descriptor>& out);
void testVocCreation(const vector<vector<Descriptor>>& features);
void testDatabase(const vector<vector<Descriptor>>& features);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// number of training images
const int NIMAGES = 4;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void wait()
{
    cout << endl << "Press enter to continue" << endl;
    //  getchar();
}

// ----------------------------------------------------------------------------

int main()
{
    vector<vector<Descriptor>> features;
    loadFeatures(features);

    testVocCreation(features);

    wait();

    testDatabase(features);

    return 0;
}

// ----------------------------------------------------------------------------

void loadFeatures(vector<vector<Descriptor>>& features)
{
    features.clear();
    features.reserve(NIMAGES);

    cv::Ptr<cv::ORB> orb = cv::ORB::create();

    cout << "Extracting ORB features..." << endl;
    for (int i = 0; i < NIMAGES; ++i)
    {
        stringstream ss;
        ss << "images/image" << i << ".png";

        cv::Mat image = cv::imread(ss.str(), 0);
        cv::Mat mask;
        vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;

        orb->detectAndCompute(image, mask, keypoints, descriptors);

        features.push_back(vector<Descriptor>());
        changeStructure(descriptors, features.back());
    }
}

// ----------------------------------------------------------------------------

void changeStructure(const cv::Mat& plain, vector<Descriptor>& out)
{
    out.resize(plain.rows);

    for (int i = 0; i < plain.rows; ++i)
    {
        auto ptr = (int32_t*)plain.ptr(i);
        //        out[i].create(1, 32, CV_8UC1);
        auto outptr = (int32_t*)out[i].data();
        for (auto j = 0; j < 8; ++j)
        {
            //            out[i][j] = ptr[j];
            outptr[j] = ptr[j];
        }
    }
}

// ----------------------------------------------------------------------------

void testVocCreation(const vector<vector<Descriptor>>& features)
{
    // branching factor and depth levels
    const int k                = 9;
    const int L                = 3;
    const WeightingType weight = TF_IDF;
    const ScoringType score    = L1_NORM;

    OrbVocabulary voc(k, L, weight, score);

    cout << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
    voc.create(features);
    cout << "... done!" << endl;

    cout << "Vocabulary information: " << endl << voc << endl << endl;

    // lets do something with this vocabulary
    cout << "Matching images against themselves (0 low, 1 high): " << endl;
    BowVector v1, v2;
    for (int i = 0; i < NIMAGES; i++)
    {
        voc.transform(features[i], v1);
        for (int j = 0; j < NIMAGES; j++)
        {
            voc.transform(features[j], v2);

            double score = voc.score(v1, v2);
            cout << "Image " << i << " vs Image " << j << ": " << score << endl;
        }
    }

    // save the vocabulary to disk
    cout << endl << "Saving vocabulary..." << endl;
    //    voc.save("small_voc.yml.gz");
    voc.saveRaw("small_voc.voca");

    cout << voc << endl;

    OrbVocabulary db2;
    db2.loadRaw("small_voc.voca");
    cout << "... done! This is: " << endl << db2 << endl;

    cout << "Done" << endl;
    exit(0);
}

// ----------------------------------------------------------------------------

void testDatabase(const vector<vector<Descriptor>>& features)
{
    cout << "Creating a small database..." << endl;

    // load the vocabulary from disk
    OrbVocabulary voc("small_voc.yml.gz");

    OrbDatabase db(voc, false, 0);  // false = do not use direct index
    // (so ignore the last param)
    // The direct index is useful if we want to retrieve the features that
    // belong to some vocabulary node.
    // db creates a copy of the vocabulary, we may get rid of "voc" now

    // add images to the database
    for (int i = 0; i < NIMAGES; i++)
    {
        db.add(features[i]);
    }

    cout << "... done!" << endl;


    // and query the database
    cout << "Querying the database: " << endl;

    QueryResults ret;
    for (int i = 0; i < NIMAGES; i++)
    {
        db.query(features[i], ret, 4);

        // ret[0] is always the same image in this case, because we added it to the
        // database. ret[1] is the second best match.

        cout << "Searching for Image " << i << ". " << ret << endl;
    }

    cout << endl;
}

// ----------------------------------------------------------------------------
