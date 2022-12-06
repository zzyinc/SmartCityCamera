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
// 人脸相似度阈值
#define FRS_SCORE_THRESHOLD 77

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
     * 计算人脸余弦相似度
     *
     * @param [other] - SDK 返回的人脸检测结果
     *
     * @return - float 表示相似度分数，分数越高表示越相似
     */
    int similarity(const FaceResult& other) {
        float distance = 0;
        for (int i = 0; i < FRS_FEATURE_LENGTH; i += 4) {
            float tmp_value0 = this->feature[i] * other.feature[i];
            float tmp_value1 = this->feature[i+1] * other.feature[i+1];
            float tmp_value2 = this->feature[i+2] * other.feature[i+2];
            float tmp_value3 = this->feature[i+3] * other.feature[i+3];

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
