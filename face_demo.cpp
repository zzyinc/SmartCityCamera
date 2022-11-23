#include <array>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include <unistd.h>
#include <dirent.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "face_sdk.h"

#define LICENSE_PATH "./face_demo_license.key"
#define USER_IMAGES_PATH "../data/users"
#define TEST_IMAGES_PATH "../data/images"
#define OUTPUT_IMAGES_PATH "../data/images_result"

static void get_files(
        const char* dir, 
        std::vector<std::string>* names);

int main(int argc, char **argv) {
    FaceSDK face_sdk(LICENSE_PATH);

    // 读取人脸大头照目录
    std::vector<std::string> user_names;
    get_files(USER_IMAGES_PATH, &user_names);

    printf("Scan user dir... [file_nums=%zu]\n", user_names.size());

    std::map<std::string, FaceResult> user_face;

    for (auto &user_name : user_names) {
        char file_name[128];
        snprintf(
                file_name, 
                sizeof(file_name), 
                "%s/%s",
                USER_IMAGES_PATH,
                user_name.c_str());

        cv::Mat frame;
        std::vector<FaceResult> face_results; 
        int feature_res = face_sdk.getFaceFeatureFile(
                file_name,
                true,       // bigger_face_mode
                nullptr,
                &frame,
                &face_results);

        if (0 != feature_res) {
            printf(
                    "\tFailed to get face feature. [file_name='%s' res=%d]\n",
                    file_name,
                    feature_res);
            continue;
        }

        if (face_results.size() != 1) {
            printf(
                    "More than one face. [file_name='%s' face_num=%zu]\n",
                    file_name,
                    face_results.size());
            continue;
        }

        user_face[user_name] = face_results[0];
        printf("Loaded user. [user_name='%s']\n", user_name.c_str());
    }

    // 读取比对大图
    std::vector<std::string> image_names;
    get_files(TEST_IMAGES_PATH, &image_names);

    printf("Scan image dir... [file_num=%zu]\n", image_names.size());

    // 黑夜/白天共同的 ROI 区域
    int roi_bbox[] = {256, 180, 1400, 900};

    for (auto &user_name : image_names) {
        char file_name[128];
        snprintf(
                file_name, 
                sizeof(file_name), 
                "%s/%s",
                TEST_IMAGES_PATH,
                user_name.c_str());

        double t = clock(); 

        cv::Mat frame;
        std::vector<FaceResult> face_results; 
        int feature_res = face_sdk.getFaceFeatureFile(
                file_name,
                false,   // bigger_face_mode
                // roi_bbox,
                nullptr, 
                &frame,
                &face_results);

        if (0 != feature_res) {
            printf(
                    "\tFailed to get face feature. [file_name='%s' res=%d]\n",
                    file_name,
                    feature_res);
            continue;
        }

        printf(
                "Found %zu users in image. [file_name='%s'(%d,%d) time=%.3fms]\n",
                face_results.size(),
                file_name,
                frame.size().width, 
                frame.size().height, 
                (clock()-t)/1000.0);

        // cv::rectangle(
        //         frame,
        //         cv::Point(roi_bbox[0], roi_bbox[1]),
        //         cv::Point(roi_bbox[2], roi_bbox[3]),
        //         cv::Scalar(200, 25, 25),
        //         5 /* line width */);

        for (auto& result : face_results) {
            float sim_in_lib = 0.0;
            std::string sim_user;

            for (auto it = user_face.begin(); it != user_face.end(); ++it) {
                float sim = result.similarity(it->second);
                if (sim > sim_in_lib) {
                    sim_in_lib = sim;
                    sim_user = it->first;
                }
            }

            if (result.face.face_mask == 1) {
                cv::rectangle(
                        frame,
                        cv::Point(result.face.bbox[0], result.face.bbox[1]),
                        cv::Point(result.face.bbox[2], result.face.bbox[3]),
                        cv::Scalar(20, 20, 200),
                        5 /* line width */);
            } else {
                cv::rectangle(
                        frame,
                        cv::Point(result.face.bbox[0], result.face.bbox[1]),
                        cv::Point(result.face.bbox[2], result.face.bbox[3]),
                        cv::Scalar(187, 236, 93),
                        5 /* line width */);
            }

            char score_str[64];

            if (sim_in_lib > 0.7) {
                snprintf(
                        score_str, 
                        64, 
                        "%.3f %s", 
                        result.face.score,
                        sim_user.c_str());

                cv::putText(
                        frame,
                        score_str,
                        cv::Point(result.face.bbox[0], result.face.bbox[1]),
                        cv::FONT_HERSHEY_SIMPLEX,
                        0.5, /* font scale */
                        cv::Scalar(0, 0, 255), /* font color */
                        1.5 /* font thickness */);

                printf(
                        "\t%s (sim: %.4f)",
                        sim_user.c_str(),
                        sim_in_lib);
            } else {
                snprintf(
                        score_str, 
                        64, 
                        "%.3f", 
                        result.face.score); 

                cv::putText(
                        frame,
                        score_str,
                        cv::Point(result.face.bbox[0], result.face.bbox[1]),
                        cv::FONT_HERSHEY_SIMPLEX,
                        0.5, /* font scale */
                        cv::Scalar(0, 0, 255), /* font color */
                        1.5 /* font thickness */);

                printf("\tUnknown user");
            }
            
            printf(
                    "\t[%d,%d,%d,%d] %.4f\n",
                    result.face.bbox[0],
                    result.face.bbox[1],
                    result.face.bbox[2],
                    result.face.bbox[3],
                    result.face.score);
        }

        snprintf(
                file_name, 
                sizeof(file_name), 
                "%s/%s", 
                OUTPUT_IMAGES_PATH,
                user_name.c_str());

        cv::imwrite(file_name, frame); 
    }

    return 0;
}

static void get_files(
        const char* folder_path, 
        std::vector<std::string>* names) {
    struct dirent* de = NULL;
    DIR* dir = NULL;

    dir = opendir(folder_path);
    if (dir == NULL) {
        printf("Failed to open folder. [path='%s']\n", folder_path);
        return;
    }
    
    while ((de = readdir(dir))) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;
        names->push_back(de->d_name);
    }
    
    closedir(dir);
}

