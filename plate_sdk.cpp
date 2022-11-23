#include <fstream>
#include <sstream>
#include <string>

// OpenCV
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

// FaceRecogSDK
#include "plate_tracker.h"
#include "wb_auth.h"

#include "plate_sdk.h"

// PlateSDK --------------------------------------------------------------------

#define PLATE_CLS_THRESHOLD 0.3         // 车牌的分数阈值
#define PLATE_NO_LENGTH_LIMIT 7         // 车牌的长度阈值（一个 UTF-8 汉字占3个字节）

PlateSDK::PlateSDK(const std::string& license_path) {
    // 验证 License
    std::ifstream ifs(license_path);
    std::stringstream license_sstream;
    license_sstream << ifs.rdbuf();

    if (0 != facerecogsdk::license_init_offline(
                license_sstream.str(),
                NULL)) {
        fprintf(
                stderr, 
                "Failed to init license. [license_path='%s']\n",
                license_path.c_str());
        return;
    }

    // 初始化 SDK
	facerecogsdk::PlateDetectParam track_param;
    track_param.min_plate_size = 10;
    track_param.max_plate_size = 9999;
    track_param.bigger_plate_mode = false;
    track_param.align_plate = true;
    track_param.street_view_mode = true;

    plate_tracker_.reset(new facerecogsdk::PlateTracker(
            "../assets/plate-tracker", 
            "config.ini", 
            track_param));
}

PlateSDK::~PlateSDK() {
}

int PlateSDK::getPlate(
        const cv::Mat& frame,
        std::vector<PlateResult> *plate_results) {
    cv::Mat frame_aligned;
    const cv::Mat* p_frame_aligned;

    int aligned_cols = ceil(frame.cols / 4.0) * 4;
    int aligned_rows = ceil(frame.rows / 4.0) * 4;

    int top = 0;
    int bottom = aligned_rows - frame.rows;
    int left = 0;
    int right = aligned_cols - frame.cols;

    if (bottom > 0 || right > 0) {
        // RKNN 的 Linux-RGA 要求 Align 到 4 的倍数
        cv::copyMakeBorder(
                frame, 
                frame_aligned, 
                top, 
                bottom, 
                left, 
                right, 
                cv::BORDER_CONSTANT, 
                0);
        p_frame_aligned = &frame_aligned;
    } else {
        p_frame_aligned = &frame;
    }

    std::vector<facerecogsdk::PlateStatus> results;

    plate_tracker_->DetectBGR(
            p_frame_aligned->data, 
            p_frame_aligned->cols, 
            p_frame_aligned->rows, 
            1, 
            nullptr, 
            results);

    for (int i = 0; i < results.size(); ++i) {
        float score = results[i].vechile.score * results[i].plate.score;
        if (score < PLATE_CLS_THRESHOLD) {
            continue;
        }
        if (results[i].text.length() < PLATE_NO_LENGTH_LIMIT) {
            continue;
        }

        PlateResult pr;
        pr.bbox[0] = results[i].vechile.bbox[0] + results[i].plate.bbox[0];
        pr.bbox[1] = results[i].vechile.bbox[1] + results[i].plate.bbox[1];
        pr.bbox[2] = results[i].vechile.bbox[0] + results[i].plate.bbox[2];
        pr.bbox[3] = results[i].vechile.bbox[1] + results[i].plate.bbox[3];
        pr.score = results[i].vechile.score * results[i].plate.score;
        pr.text = results[i].text;

        plate_results->push_back(std::move(pr));
    }

    return 0;
}

int PlateSDK::getPlateFile(
        const std::string& file_path,
        cv::Mat* frame,
        std::vector<PlateResult>* plate_result) {
    *frame = cv::imread(file_path);
    
    if (frame->data == nullptr) {
        fprintf(stderr, "Invalid image. [filename='%s']\n", file_path.c_str());
        return 1;
    }

    return getPlate(
            *frame, 
            plate_result);
}

