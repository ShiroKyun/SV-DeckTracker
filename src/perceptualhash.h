#ifndef PERCEPTUALHASH_H_
#define PERCEPTUALHASH_H_

#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/imgproc/imgproc.hpp"

typedef unsigned long long int ulong64;

class PerceptualHash {
public:
    struct ComparisonResult {
        int distance;
        size_t index;
    };
    static ulong64 phash(const cv::Mat& image);
    static int hammingDistance(const ulong64& a, const ulong64& b);
    static ComparisonResult best(const ulong64& hash, const std::vector<ulong64>& dataSet);
    static std::vector<ComparisonResult> nbest(int n, const ulong64& hash, const std::vector<ulong64>& dataSet);
private:
    static cv::Mat k;
};


#endif /* PERCEPTUALHASH_H_ */
