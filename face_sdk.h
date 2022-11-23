#pragma once

#include <memory>
#include <string>
#include <vector>

// OpenCV
#include "opencv2/core.hpp"

// FaceRecogSDK
#include "face_tracker.h"
#include "face_feature.h"

// 人脸特征长度，不可修改
#define FRS_FEATURE_LENGTH 512
// 检测结果中的最小人脸（长/宽像素），可修改
#define FRS_MIN_FACE_SIZE 16
// Pitch / Yaw/ Roll 检测阈值，可修改
#define FRS_PITCH_THRESHOLD 40
#define FRS_YAW_THRESHOLD 40
#define FRS_ROLL_THRESHOLD 30


struct FaceResult {
    facerecogsdk::FaceStatus face;
    float feature[FRS_FEATURE_LENGTH];

    explicit FaceResult() {
        memset(&this->face, 0, sizeof(face));
        memset(&this->feature, 0, sizeof(feature));
    }

    explicit FaceResult(const facerecogsdk::FaceStatus& face, const float* feature) {
        memcpy(&this->face, &face, sizeof(this->face));
        memcpy(&this->feature, feature, sizeof(this->feature));
    }

    explicit FaceResult(
            const facerecogsdk::FaceStatus& face, 
            const float* feature, 
            const int roi_bbox[]) {
        memcpy(&this->face, &face, sizeof(this->face));
        memcpy(&this->feature, feature, sizeof(this->feature));
        if (nullptr != roi_bbox) {
            this->face.bbox[0] += roi_bbox[0];
            this->face.bbox[1] += roi_bbox[1];
            this->face.bbox[2] += roi_bbox[0];
            this->face.bbox[3] += roi_bbox[1];
            for (int i = 0; i < 5; ++i){
                this->face.xy5Points[2 * i] += roi_bbox[0];
                this->face.xy5Points[2 * i + 1] += roi_bbox[1];
            }
        }
    }

    explicit FaceResult(const FaceResult& result) {
        memcpy(&this->face, &result.face, sizeof(this->face));
        memcpy(&this->feature, result.feature, sizeof(this->feature));
    }

    /*
     * 计算人脸相似度
     *
     * @param [other] - SDK 返回的人脸检测结果
     *
     * @return - float 表示相似度分数，分数越高表示越相似
     */
    float similarity(const FaceResult& other) {
        float distance = 0;
        for (int i = 0; i < FRS_FEATURE_LENGTH; i += 4) {
            float tmp_value0 = this->feature[i] - other.feature[i];
            float tmp_value1 = this->feature[i+1] - other.feature[i+1];
            float tmp_value2 = this->feature[i+2] - other.feature[i+2];
            float tmp_value3 = this->feature[i+3] - other.feature[i+3];

            distance += tmp_value0 * tmp_value0 
                    + tmp_value1 * tmp_value1
                    + tmp_value2 * tmp_value2
                    + tmp_value3 * tmp_value3;
        }

        if (distance < 0.9) {
            distance = 1.0;
        } else if (distance < 1.9) {
            distance = 1.9 - distance;
        } else {
            distance = 0.0;
        }

        return distance;
    }

};

/*
 * 人脸SDK接口
 */
class FaceSDK : public std::enable_shared_from_this<FaceSDK> {
public:

    /*
     * 初始化 SDK
     *
     * @params [license_path] - 传入离线 License 文件地址
     */
    explicit FaceSDK(const std::string& license_path);
    ~FaceSDK();

    /*
     * 人脸检测及人脸特征计算
     *
     * @params [frame] - OpenCV 的图片矩阵。输入图片长宽若非4的整数倍时，需花费额外时间进行填充操作
     * @params [bigger_face_mode] - True 表示输入为大头照，用于底库建库；False 表示输入为普通照片
     * @params [roi_bbox] - int[4]，分别为 ROI 的左上角、右下角 <x, y>，传入 NULL 表示对全图进行识别
     * @params [face_result] - 返回人脸检测及特征结果
     *
     * @return - 0 表示正常返回，其余为错误
     */
    int getFaceFeature(
            const cv::Mat& frame,
            bool bigger_face_mode,
            const int roi_bbox[],
            std::vector<FaceResult>* face_result);

    /*
     * 人脸检测及人脸特征计算
     *
     * @params [file_path] - 图片文件地址，支持 jpg/png 格式。输入图片长宽若非4的整数倍时，需花费额外时间进行填充操作
     * @params [bigger_face_mode] - True 表示输入为大头照，用于底库建库；False 表示输入为普通照片
     * @params [roi_bbox] - int[4]，分别为 ROI 的左上角、右下角 <x, y>，传入 NULL 表示对全图进行识别
     * @params [frame] - 返回读入的图片
     * @params [face_result] - 返回人脸检测及特征结果
     *
     * @return - 0 表示正常返回，其余为错误
     */
    int getFaceFeatureFile(
            const std::string& file_path,
            bool bigger_face_mode,
            const int roi_bbox[],
            cv::Mat* frame,
            std::vector<FaceResult>* face_result);

private:
    void alignFace_(
            const cv::Mat& src, 
            const float* xy_5points, 
            const int crop_size, 
            cv::Mat* dst);

    bool checkFaceAttitude_(
            int pitch_thres, 
            int yaw_thres, 
            int roll_thres, 
            int pitch, 
            int yaw, 
            int roll);	
 
    std::unique_ptr<facerecogsdk::FaceTracker> face_tracker_;
	std::unique_ptr<facerecogsdk::FaceFeature> face_feature_;
};
