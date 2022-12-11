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

static float COS_TO_SCORE[] = {
    0.000000,
    0.013366,
    0.016732,
    0.020099,
    0.023465,
    0.026831,
    0.030197,
    0.033563,
    0.036930,
    0.040296,
    0.043662,
    0.047028,
    0.050394,
    0.053761,
    0.057127,
    0.060493,
    0.063859,
    0.067225,
    0.070592,
    0.073958,
    0.077324,
    0.080690,
    0.084056,
    0.087423,
    0.090789,
    0.094155,
    0.097521,
    0.100887,
    0.104254,
    0.107620,
    0.110986,
    0.114352,
    0.117718,
    0.121085,
    0.124451,
    0.127817,
    0.131183,
    0.134549,
    0.137915,
    0.141282,
    0.144648,
    0.148014,
    0.151380,
    0.154746,
    0.158113,
    0.161479,
    0.164845,
    0.168211,
    0.171577,
    0.174944,
    0.178310,
    0.181676,
    0.185042,
    0.188408,
    0.191775,
    0.195141,
    0.198507,
    0.201873,
    0.205239,
    0.208606,
    0.211972,
    0.215338,
    0.218704,
    0.222070,
    0.225437,
    0.228803,
    0.232169,
    0.235535,
    0.238901,
    0.242268,
    0.245634,
    0.249000,
    0.257750,
    0.266500,
    0.275250,
    0.284000,
    0.292750,
    0.301500,
    0.310250,
    0.319000,
    0.328000,
    0.337000,
    0.346000,
    0.355000,
    0.364000,
    0.373000,
    0.399600,
    0.426200,
    0.452800,
    0.479400,
    0.506000,
    0.517800,
    0.529600,
    0.541400,
    0.553200,
    0.565000,
    0.600000,
    0.635000,
    0.670000,
    0.705000,
    0.715000,
};

// DDDDEBUG
int ttt = 0;

// FaceSDK --------------------------------------------------------------------

FaceSDK::FaceSDK(const char* license_path) {
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
                license_path);
        return;
    }

    // 初始化 SDK
    this->track_param_.min_face_size = 20;
    this->track_param_.max_face_size = 9999;
    this->track_param_.face_mask_mode = true;
    this->track_param_.bigger_face_mode = false;
    this->track_param_.need_track_model = false;
    this->track_param_.detect_interval = 5;
    this->track_param_.multi_landmark_mode = true;
    this->track_param_.need_pose_estimate = true;
    
    this->face_tracker_ = new facerecogsdk::FaceTracker(
            "../assets/face-tracker", 
            "config.ini", 
            this->track_param_);

    this->face_feature_ = new facerecogsdk::FaceFeature(
            "../assets/face-feature",        
            "config.ini");
}

FaceSDK::~FaceSDK() {
}

int FaceSDK::loadRawImage(
        const unsigned char* bgr_data,
        size_t image_width,
        size_t image_height,
        bool bigger_face_mode,
        int* face_num) { 
    this->image_ = cv::Mat(
            image_height, 
            image_width, 
            CV_8UC3, 
            (unsigned char*)bgr_data).clone();

    if (NULL == this->image_.data) {
        return -1;
    }

    return loadCVImage_(
            bigger_face_mode,
            face_num);
}

int FaceSDK::loadEncodedImage( 
        const unsigned char* enc_data,
        size_t enc_size,
        bool bigger_face_mode,
        int* face_num) {
    // Create a Size(1, nSize) Mat object of 8-bit, single-byte elements
    cv::Mat enc_buffer(1, enc_size, CV_8UC1, (void*)enc_data);
    
    this->image_ = cv::imdecode(enc_buffer, CV_LOAD_IMAGE_COLOR);
    if (NULL == this->image_.data) {
        return -1;
    }

    return loadCVImage_(
            bigger_face_mode,
            face_num);
}

int FaceSDK::getAllFaces(
        FaceDetectResult* face_detects) {
    if (NULL == face_detects) {
        return -1;
    }

    for (int i = 0; i < this->face_results_.size(); ++i) { 
        auto& face_status = this->face_results_[i];
        FaceDetectResult* face = face_detects + i;
        memcpy(face->bbox, face_status.bbox, sizeof(face_status.bbox)); // int * 4
        face->face_mask = face_status.face_mask;
        face->quality = calcFaceQuality_(face_status);
    }

    return this->face_results_.size();
}

int FaceSDK::getFaceFeature(
        int face_idx,
        float* face_feature) {
    cv::Mat face_aligned;
    if (0 != getFaceImage(face_idx, &face_aligned)) {
        return -1;
    }
 
    face_feature_->ExtractBGR(
            nullptr, 
            face_aligned.data, 
            face_aligned.cols, 
            face_aligned.rows, 
            1, 
            face_feature);

    return 0;
}

int FaceSDK::getFaceImage(
        int face_idx,
        cv::Mat* face_aligned) {
    if (face_idx >= this->face_results_.size()) {
        return -1;
    }
    if (NULL == face_aligned) {
        return -1;
    }

    alignFace_(
            this->image_,
            this->face_results_[face_idx].xy5Points, 
            112, /* face crop size */
            face_aligned);
 
    return 0;
}

int FaceSDK::faceSimilarity(
        const float* face_feature0,
        const float* face_feature1) {
    float distance = 0;
    for (int i = 0; i < FRS_FEATURE_LENGTH; i += 4) {
        float tmp_value0 = face_feature0[i+0] * face_feature1[i+0];
        float tmp_value1 = face_feature0[i+1] * face_feature1[i+1];
        float tmp_value2 = face_feature0[i+2] * face_feature1[i+2];
        float tmp_value3 = face_feature0[i+3] * face_feature1[i+3];

        distance += tmp_value0
            + tmp_value1
            + tmp_value2
            + tmp_value3;
    }

    for (int i = 100; i > 0; i--) {
        if (distance >= COS_TO_SCORE[i]) {
            return i;
        }
    }

    return 0;
}

int FaceSDK::loadCVImage_(
        bool bigger_face_mode,
        int* face_num) {
    if (NULL == face_num) {
        return -1;
    }

    this->face_results_.clear();

    if (bigger_face_mode) {
        this->track_param_.bigger_face_mode = true; 
        this->track_param_.cls_threshold = 0.4;
    } else {
        this->track_param_.bigger_face_mode = false; 
        this->track_param_.cls_threshold = 0.5;
    }

    face_tracker_->DetectBGR(
            this->image_.data, 
            this->image_.cols,
            this->image_.rows,
            1, 
            &this->track_param_, 
            nullptr, 
            this->face_results_);

    *face_num = this->face_results_.size();
    return 0;
}


void FaceSDK::alignFace_(
        const cv::Mat& src, 
        const float* xy_5points, 
        const int crop_size, 
        cv::Mat* dst) {
    cv::Mat srcpts(5, 2, CV_32F, cv::Scalar(0));
    cv::Mat dstpts(5, 2, CV_32F, cv::Scalar(0));
    static float template_landmark[10] = {
        38.2946f, 51.6963f,
        73.5318f, 51.5014f,
        56.0252f, 71.7366f,
        41.5493f, 92.3655f,
        70.7299f, 92.2041f
    };

    memcpy(dstpts.data, xy_5points, 10 * sizeof(float));
    memcpy(srcpts.data, template_landmark, 10 * sizeof(float));

    cv::Mat warp_33 = similarTransform(dstpts, srcpts);
    cv::Mat warp_23(2, 3, CV_32F);
    memcpy(warp_23.data, warp_33.data, 2 * 3 * sizeof(float));

    cv::warpAffine(src, *dst, warp_23, cv::Size(crop_size, crop_size));
}


int FaceSDK::calcFaceQuality_(
        const facerecogsdk::FaceStatus& face) {
    // 人脸像素
    float pixel_score = std::min(
            1.0f, 
            float(std::min(face.face_width, face.face_height)) / FRS_RECOG_MIN_FACE_SIZE);

    // 人脸角度
    float front_score = 
            std::max(0.0f, 1.0f - std::abs(float(face.pitch) / FRS_RECOG_PITCH_THRESHOLD)) * 0.4f
            + std::max(0.0f, 1.0f - std::abs(float(face.yaw) / FRS_RECOG_YAW_THRESHOLD)) * 0.4f
            + std::max(0.0f, 1.0f - std::abs(float(face.roll) / FRS_RECOG_ROLL_THRESHOLD)) * 0.2f;

    int score = int((face.score * 0.4 + pixel_score * 0.3 + front_score * 0.3) * 100);
    return score;
}


