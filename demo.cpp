/**
 * Original File: demo.cpp
 * Original Author: Dorian Galvez-Lopez
 *
 * Modified by: Darius Rückert
 * Modifications:
 *  - Updated vocabulary tests
 *  - Removed database tests
 *
 * Original License: BSD-like
 *          https://github.com/dorian3d/DBoW2/blob/master/LICENSE.txt
 * License of modifications: MIT
 *          https://github.com/darglein/DBoW2/blob/master/LICENSE.txt
 *
 */


#include "MiniBow.h"

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>

using namespace DBoW2;
using namespace std;


using Descriptor    = FORB::TDescriptor;
using OrbVocabulary = DBoW2::TemplatedVocabulary<Descriptor, FORB, L1Scoring>;

void changeStructure(const cv::Mat& plain, vector<Descriptor>& out)
{
    out.resize(plain.rows);

    for (int i = 0; i < plain.rows; ++i)
    {
        auto ptr    = (uint64_t*)plain.ptr(i);
        auto outptr = (uint64_t*)out[i].data();
        for (auto j = 0; j < 4; ++j)
        {
            outptr[j] = ptr[j];
        }
    }
}

void loadFeatures(vector<vector<Descriptor>>& features)
{
    const int NIMAGES = 4;

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


// ----------------------------------------------------------------------------

void testVocMatching(const vector<vector<Descriptor>>& features, OrbVocabulary& voc)
{
    // lets do something with this vocabulary
    cout << "Matching images against themselves (0 low, 1 high): " << endl;
    BowVector v1, v2;

    double out = 0;


    //    exit(0);
    cv::TickMeter tm;
    tm.start();
    //    for (int i = 0; i < 100; ++i)
    {
        for (int i = 0; i < features.size(); i++)
        {
            voc.transform(features[i], v1);
            for (int j = 0; j < features.size(); j++)
            {
                voc.transform(features[j], v2);

                double score = voc.score(v1, v2);
                out += score;
                cout << "Image " << i << " vs Image " << j << ": " << score << endl;
            }
        }
    }
    tm.stop();
    cout << tm.getTimeMilli() << " " << out << endl << endl;
}

void testVocCreation(const vector<vector<Descriptor>>& features, OrbVocabulary& voc)
{
    // branching factor and depth levels
    const int k                = 9;
    const int L                = 3;
    const WeightingType weight = TF_IDF;



    voc = OrbVocabulary(k, L, weight);

    cout << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
    voc.create(features);
    cout << "Vocabulary information: " << endl << voc << endl << endl;


    //    exit(0);
    cout << "Testing loading saving..." << endl;

    voc.saveRaw("testvoc.minibow");
    cout << voc << endl;
    OrbVocabulary db2;
    db2.loadRaw("testvoc.minibow");
    cout << db2 << endl;
    cout << "... done." << endl << endl;
}


int main()
{
    vector<vector<Descriptor>> features;
    loadFeatures(features);

    OrbVocabulary trainedVoc;
    testVocCreation(features, trainedVoc);

    cout << "Testing Matching with trained Voc..." << endl;
    testVocMatching(features, trainedVoc);

    OrbVocabulary orbVoc("ORBvoc.minibow");
    cout << "Testing Matching with ORB-SLAM Voc..." << endl;
    testVocMatching(features, orbVoc);


    return 0;
}
