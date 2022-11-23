#include <cstdio>
#include <memory>
#include <fstream>
#include <sstream>
#include <string>

// OpenCV
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

// FaceRecogSDK
#include "face_tracker.h"
#include "face_feature.h"
#include "wb_auth.h"

#include "face_sdk.h"

// 在 similarity_transform.cpp 中实现
cv::Mat similarTransform(const cv::Mat& src, const cv::Mat& dst);

// FaceSDK --------------------------------------------------------------------

FaceSDK::FaceSDK(const std::string& license_path) {
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
	facerecogsdk::FaceDetectParam track_param;
    track_param.min_face_size = 10;
    track_param.max_face_size = 9999;
    track_param.face_mask_mode = true;
    track_param.bigger_face_mode = false;
    track_param.need_track_model = false;
    track_param.detect_interval = 5;
    track_param.landmark_for_masked_face = true;
    track_param.multi_landmark_mode = true;
    track_param.need_pose_estimate = true;
    
    face_tracker_.reset(new facerecogsdk::FaceTracker(
            "../assets/face-tracker", 
            "config.ini", 
            track_param));

    face_feature_.reset(new facerecogsdk::FaceFeature(
            "../assets/face-feature",        
            "config.ini"));
}

FaceSDK::~FaceSDK() {
}

int FaceSDK::getFaceFeature(
        const cv::Mat& frame,
        bool bigger_face_mode,
        const int roi_bbox[],
        std::vector<FaceResult> *face_results) {
    int roi_x = 0;
    int roi_y = 0;
    int roi_w = frame.cols;
    int roi_h = frame.rows;

    if (nullptr != roi_bbox) {
        roi_x = std::max(0, roi_bbox[0]);
        roi_y = std::max(0, roi_bbox[1]);
        roi_w = std::min(frame.cols - 1, roi_bbox[2]) - roi_bbox[0];
        roi_h = std::min(frame.rows - 1, roi_bbox[3]) - roi_bbox[1];
    }

    // XXX: RGA 只能输入 width/height 为 4 的整数倍的图片，修正 ROI 信息
    roi_w = floor(roi_w / 4.0) * 4; 
    roi_h = floor(roi_h / 4.0) * 4; 

    cv::Mat frame_aligned = frame(cv::Rect(roi_x, roi_y, roi_w, roi_h)).clone();

    // 人脸检测
    std::vector<facerecogsdk::FaceStatus> results;

    facerecogsdk::FaceDetectParam track_param;
    track_param.min_face_size = 10;
    track_param.max_face_size = 9999;
    track_param.face_mask_mode = true;
    track_param.bigger_face_mode = false;
    track_param.need_track_model = false;
    track_param.detect_interval = 5;
    track_param.landmark_for_masked_face = true;
    track_param.multi_landmark_mode = true;
    track_param.need_pose_estimate = true;
    if (bigger_face_mode) {
        track_param.bigger_face_mode = true; 
        track_param.cls_threshold = 0.8;
    } else {
        track_param.bigger_face_mode = false; 
        track_param.cls_threshold = 0.5;
    }

    face_tracker_->DetectBGR(
            frame_aligned.data, 
            frame_aligned.cols, 
            frame_aligned.rows, 
            1, 
            &track_param, 
            nullptr, 
            results);

    float face_feature[FRS_FEATURE_LENGTH];
    for (int i = 0; i < results.size(); ++i) {
        // 检查人脸大小
        if (results[i].face_width < FRS_MIN_FACE_SIZE
                || results[i].face_height < FRS_MIN_FACE_SIZE) {
            continue;
        }

        // // 检查人脸角度
        if (!checkFaceAttitude_(
                FRS_PITCH_THRESHOLD,
                FRS_YAW_THRESHOLD,
                FRS_ROLL_THRESHOLD,
                results[i].pitch, 
                results[i].yaw, 
                results[i].roll)) {
            continue;
        }

        // 人脸对齐
        cv::Mat face_aligned;
        alignFace_(
                frame_aligned,
                results[i].xy5Points, 
                112, /* face crop size */
                &face_aligned);

        // 计算人脸特征
        face_feature_->ExtractBGR(
                nullptr, 
                face_aligned.data, 
                face_aligned.cols, 
                face_aligned.rows, 
                1, 
                face_feature);

        face_results->push_back(FaceResult(results[i], face_feature, roi_bbox)); 
    }

    return 0;
}

int FaceSDK::getFaceFeatureFile(
        const std::string& file_path,
        bool bigger_face_mode,
        const int roi_bbox[],
        cv::Mat* frame,
        std::vector<FaceResult>* face_result) {
    *frame = cv::imread(file_path);
    
    if (frame->data == nullptr) {
        fprintf(stderr, "Invalid image. [filename='%s']\n", file_path.c_str());
        return 1;
    }

    return getFaceFeature(
            *frame,
            bigger_face_mode,
            roi_bbox,
            face_result);
}

void FaceSDK::alignFace_(
        const cv::Mat& src, 
        const float* xy_5points, 
        const int crop_size, 
        cv::Mat* dst) {
    cv::Mat srcpts(5, 2, CV_32F, cv::Scalar(0));
    cv::Mat dstpts(5, 2, CV_32F, cv::Scalar(0));
    float template_landmark[10] = {
        38.2946f, 51.6963f,
        73.5318f, 51.5014f,
        56.0252f, 71.7366f,
        41.5493f, 92.3655f,
        70.7299f, 92.2041f
    };

    std::unique_ptr<float[]> template_scale_landmark(new float[10]);
    for (int i = 0; i < 10; i++) {
        template_scale_landmark[i] = template_landmark[i] * 1;
    }

    memcpy(dstpts.data, xy_5points, 10 * sizeof(float));
    memcpy(srcpts.data, template_scale_landmark.get(), 10 * sizeof(float));

    cv::Mat warp_33 = similarTransform(dstpts, srcpts);
    cv::Mat warp_23(2, 3, CV_32F);
    memcpy(warp_23.data, warp_33.data, 2 * 3 * sizeof(float));

    cv::warpAffine(src, *dst, warp_23, cv::Size(crop_size, crop_size));
}


bool FaceSDK::checkFaceAttitude_(
        int pitch_thres, 
        int yaw_thres, 
        int roll_thres, 
        int pitch, 
        int yaw, 
        int roll) {	
	if (std::abs(pitch) > pitch_thres 
            || std::abs(yaw) > yaw_thres 
            || std::abs(roll) > roll_thres) {
		return false;
	}
	return true;
}

