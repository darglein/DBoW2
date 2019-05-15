/**
 * File: FORB.h
 * Date: June 2012
 * Author: Dorian Galvez-Lopez
 * Description: functions for ORB descriptors
 * License: see the LICENSE.txt file
 *
 */

#ifndef __D_T_F_ORB__
#define __D_T_F_ORB__

#include "FClass.h"

#define USE_CV_FORB

#ifdef USE_CV_FORB
#    include <opencv2/core.hpp>
#endif
#include <array>
#include <iostream>
#include <string>
#include <vector>

namespace DBoW2
{
/// Functions to manipulate BRIEF descriptors
class FORB
{
   public:
    /// Descriptor type
#ifdef USE_CV_FORB
    using TDescriptor = cv::Mat;
#else
    using TDescriptor = std::array<uint64_t, 4>;  // CV_8U

#endif
    /// Pointer to a single descriptor
    typedef const TDescriptor* pDescriptor;
    /// Descriptor length (in bytes)
    static const int L = 32;

    using BinaryDescriptor = std::array<int32_t, 8>;

    /**
     * Calculates the mean value of a set of descriptors
     * @param descriptors
     * @param mean mean descriptor
     */
    static void meanValue(const std::vector<pDescriptor>& descriptors, TDescriptor& mean)
    {
        std::cout << "maean " << descriptors.size() << std::endl;
        //#ifdef USE_CV_FORB
        //        mean = cv::Mat::zeros(1, FORB::L, CV_8U);
        //#else
        //        std::fill(mean.begin(), mean.end(), 0);
        //#endif
        if (descriptors.empty())
        {
            //            mean = cv::Mat::zeros(1, FORB::L, CV_8U);
            return;
        }
        else if (descriptors.size() == 1)
        {
#ifdef USE_CV_FORB
            mean = descriptors[0]->clone();
#else
            mean = *descriptors[0];
#endif
        }
        else
        {
            std::vector<int> sum(FORB::L * 8, 0);

            for (size_t i = 0; i < descriptors.size(); ++i)
            {
                const auto& d = *descriptors[i];
#ifdef USE_CV_FORB
                const unsigned char* p = (const unsigned char*)d.data;
#else
                const unsigned char* p = (const unsigned char*)d.data();
#endif


                for (int j = 0; j < 32; ++j, ++p)
                {
                    if (*p & (1 << 7)) ++sum[j * 8];
                    if (*p & (1 << 6)) ++sum[j * 8 + 1];
                    if (*p & (1 << 5)) ++sum[j * 8 + 2];
                    if (*p & (1 << 4)) ++sum[j * 8 + 3];
                    if (*p & (1 << 3)) ++sum[j * 8 + 4];
                    if (*p & (1 << 2)) ++sum[j * 8 + 5];
                    if (*p & (1 << 1)) ++sum[j * 8 + 6];
                    if (*p & (1)) ++sum[j * 8 + 7];
                }
            }
#ifdef USE_CV_FORB
            mean             = cv::Mat::zeros(1, FORB::L, CV_8U);
            unsigned char* p = (unsigned char*)mean.data;
#else
            std::fill(mean.begin(), mean.end(), 0);
            unsigned char* p = (unsigned char*)mean.data();
#endif


            const int N2 = (int)descriptors.size() / 2 + descriptors.size() % 2;
            for (size_t i = 0; i < sum.size(); ++i)
            {
                if (sum[i] >= N2)
                {
                    // set bit
                    *p |= 1 << (7 - (i % 8));
                }

                if (i % 8 == 7) ++p;
            }

            //            BinaryDescriptor bd;
            //            toBinary(mean, bd);
            //            for (auto a : bd) std::cout << a << std::endl;
            //            exit(0);
        }
    }

    /**
     * Calculates the distance between two descriptors
     * @param a
     * @param b
     * @return distance
     */

    static inline uint32_t popcnt32(uint32_t x)
    {
        __asm__("popcnt %1, %0" : "=r"(x) : "0"(x));
        return x;
    }

    static inline uint32_t popcnt64(uint64_t x)
    {
        __asm__("popcnt %1, %0" : "=r"(x) : "0"(x));
        return x;
    }

    static double distance(const TDescriptor& a, const TDescriptor& b)
    {
#ifdef USE_CV_FORB
        auto pa = (uint32_t*)a.data;
        auto pb = (uint32_t*)b.data;
#else
        auto pa = (uint32_t*)a.data();
        auto pb = (uint32_t*)b.data();
#endif
        const uint64_t *pa2, *pb2;
        //        pa = a.ptr<uint64_t>();  // a & b are actually CV_8U
        //        pb = b.ptr<uint64_t>();

        pa2 = (uint64_t*)pa;
        pb2 = (uint64_t*)pb;


#if 0
        int dist = 0;
        //        for (int i = 0; i < 4; i++, pa++, pb++)
        for (int i = 0; i < 4; i++, pa2++, pb2++)
        {
            //            uint32_t v = *pa ^ *pb;
            uint64_t v = *pa2 ^ *pb2;

            // TODO: if this is really a bottleneck we can also use AVX-2
            // to gain around 25% more performance
            // according to this source:
            // https://github.com/kimwalisch/libpopcnt
#    if 1
            //            dist += popcnt32(v);
            dist += popcnt64(v);
#    else
            v = v - ((v >> 1) & 0x55555555);
            v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
            dist += (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
#    endif
        }

        return dist;
#else

        // Bit count function got from:
        // http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
        // This implementation assumes that a.cols (CV_8U) % sizeof(uint64_t) == 0



        uint64_t v, ret = 0;
        for (size_t i = 0; i < 32 / sizeof(uint64_t); ++i, ++pa2, ++pb2)
        {
            v = *pa2 ^ *pb2;
            v = v - ((v >> 1) & (uint64_t) ~(uint64_t)0 / 3);
            v = (v & (uint64_t) ~(uint64_t)0 / 15 * 3) + ((v >> 2) & (uint64_t) ~(uint64_t)0 / 15 * 3);
            v = (v + (v >> 4)) & (uint64_t) ~(uint64_t)0 / 255 * 15;
            ret += (uint64_t)(v * ((uint64_t) ~(uint64_t)0 / 255)) >> (sizeof(uint64_t) - 1) * __CHAR_BIT__;
        }

        return ret;

        // // If uint64_t is not defined in your system, you can try this
        // // portable approach (requires DUtils from DLib)
        // const unsigned char *pa, *pb;
        // pa = a.ptr<unsigned char>();
        // pb = b.ptr<unsigned char>();
        //
        // int ret = 0;
        // for(int i = 0; i < a.cols; ++i, ++pa, ++pb)
        // {
        //   ret += DUtils::LUT::ones8bits[ *pa ^ *pb ];
        // }
        //
        // return ret;
#endif
    }



    static void toBinary(const TDescriptor& a, BinaryDescriptor& b)
    {
#ifdef USE_CV_FORB
        auto rowPtrInt = reinterpret_cast<const int32_t*>(a.data);
#else
        auto rowPtrInt  = reinterpret_cast<const int32_t*>(a.data());
#endif
        for (int i = 0; i < 8; ++i)
        {
            b[i] = rowPtrInt[i];
        }
    }
    static void fromBinary(const BinaryDescriptor& a, TDescriptor& b)
    {
#ifdef USE_CV_FORB
        b               = cv::Mat::zeros(1, FORB::L, CV_8U);
        auto* rowPtrInt = reinterpret_cast<int32_t*>(b.data);
#else
        auto* rowPtrInt = reinterpret_cast<int32_t*>(b.data());
#endif
        for (int i = 0; i < 8; ++i)
        {
            rowPtrInt[i] = a[i];
        }
    }
};

}  // namespace DBoW2

#endif
