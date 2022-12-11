//
// Writed by junezhang 2020/02/14
//

#ifndef __FACE_STATUS_PARAM_H__
#define __FACE_STATUS_PARAM_H__

#define LANDMARK_NUM 106

namespace facerecogsdk
{
    // TODO: 优化一下关键点的内存浪费
    struct FaceStatus {
        float score = 0;
        int bbox[4];  // (x1, y1, x2, y2)
        int face_width;
        int face_height;
        int face_mask = -1;  // 是否佩戴口罩，-1: 未检测，0: 佩戴口罩，1: 未佩戴口罩
        float xy5Points[10];
        float landmarks[LANDMARK_NUM * 2];
        int traceId = -1;  // 追踪到人脸的 trace id，从1开始, 也表示不同人脸的总数, detect 模式下，为 -1;
        int frameId = -1;  // 追踪到人脸的 frame id，从1开始, 也表示同一人脸出现的帧数, detect 模式下，为 -1;
        int remainNum = -1;  // 表示人脸在视频内的持续帧数, 包括检测帧, 跟踪帧和跟踪丢失帧，从1开始, detect 模式下，为 -1;
        float pointsVis[LANDMARK_NUM];
        int pitch = 0;  // 人脸绕 x 轴角度
        int yaw = 0;  // 人脸绕 y 轴角度
        int roll = 0;  // 人脸绕 z 轴角度
    };

    struct FaceDetectParam {
        // face detect
        int min_face_size = 10;
        int max_face_size = 9999;
        bool bigger_face_mode = false;  // 是否只检测面积最大的人脸
        bool face_mask_mode = false;    // 是否启用口罩检测，false: 不使用，true: 使用
        float cls_threshold = 0.8;

        // face track
        bool need_track_model = true;   // 是否使用人脸追踪 
        int detect_interval = 5;        // 追踪间隔（帧）

        // face align(landmark)
        bool multi_landmark_mode = true;        // 是否进行 106 点关键点检测
        bool need_pose_estimate = true;         // 是否进行姿态估计
    };

    struct Image {
        int width;
        int height;
        unsigned char* data;  //RGB image without roatation
    };
} // end of namespace facerecogsdk

#endif  //__FACE_STATUS_PARAM_H__
