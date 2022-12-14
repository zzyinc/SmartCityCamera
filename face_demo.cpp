#include <array>
#include <cstdio>
#include <fstream>
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

#define COLOR_RED cv::Scalar(20, 20, 255)
#define COLOR_GREEN cv::Scalar(187, 236, 93)
#define COLOR_YELLOW cv::Scalar(20, 255, 255)

// 人脸检测质量阈值
#define FACE_QUALITY_THRESHOLD 70
// 人脸相似度阈值
#define FACE_RECOG_SCORE_THRESHOLD 87

static void get_files(
        const char* dir, 
        std::vector<std::string>* names);

static std::string get_uid_from_fn(const std::string& fn);

int main(int argc, char **argv) {
    FaceSDK face_sdk(LICENSE_PATH);
    int res = 0;

    // 读取人脸大头照目录
    std::vector<std::string> user_names;
    get_files(USER_IMAGES_PATH, &user_names);

    printf("Scan user dir... [file_nums=%zu]\n", user_names.size());

    std::map<std::string, std::unique_ptr<float[]> > user_face;
 
    for (auto &user_name : user_names) {
        char file_name[128];
        snprintf(
                file_name, 
                sizeof(file_name), 
                "%s/%s",
                USER_IMAGES_PATH,
                user_name.c_str());

        // OpenCV 默认载入 BGR 格式 
        std::ifstream fin(file_name);
        fin.seekg(0, fin.end);
        int file_size = fin.tellg();
        fin.seekg(0, fin.beg);

        std::unique_ptr<unsigned char[]> file_buffer(new unsigned char[file_size]);
        fin.read((char*)file_buffer.get(), file_size);

        int face_num = 0;
        if (0 != (res = face_sdk.loadEncodedImage(
                    file_buffer.get(),
                    file_size,
                    true,           // bigger_face_mode,
                    &face_num))) {
            printf(
                    "\tFailed to load image. [file_name='%s' res=%d]\n",
                    file_name,
                    res);

        }
       
        if (1 != face_num) {
            printf(
                    "Invalid face quantity. [file_name='%s' face_num=%d]\n",
                    file_name,
                    face_num);
            continue;
        }

        FaceDetectResult face_detect[1];
        face_sdk.getAllFaces(face_detect);

        if (face_detect[0].quality < FACE_QUALITY_THRESHOLD) {
            printf(
                    "Face quality is too low. [file_name='%s' face_quality=%d]\n",
                    file_name,
                    face_detect[0].quality);
            continue;
        }

        std::unique_ptr<float[]> feature(new float[FRS_FEATURE_LENGTH]);
        if (0 != (res = face_sdk.getFaceFeature(0, feature.get()))) {
            printf(
                    "\tFailed to get face feature. [file_name='%s' res=%d]\n",
                    file_name,
                    res);
            continue;
        }

        user_face[user_name] = std::move(feature); 
        printf(
                "Loaded user. [user_name='%s' face_quality=%d]\n", 
                user_name.c_str(),
                face_detect[0].quality);
    }

    // 读取比对大图
    std::vector<std::string> image_names;
    get_files(TEST_IMAGES_PATH, &image_names);

    printf("Scan image dir... [file_num=%zu]\n", image_names.size());

    int recall_file = 0;
    int recall = 0;
    int precision = 0;

    for (auto &cmp_name : image_names) {
        char file_name[128];
        snprintf(
                file_name, 
                sizeof(file_name), 
                "%s/%s",
                TEST_IMAGES_PATH,
                cmp_name.c_str());

        bool recall_in_file = false;
        double t = clock(); 

        cv::Mat frame = cv::imread(file_name);
        int face_num = 0;
        if (0 != (res = face_sdk.loadRawImage(
                    frame.data,
                    frame.cols,
                    frame.rows,
                    false,          // bigger_face_mode,
                    &face_num))) {
            printf(
                    "\tFailed to load image. [file_name='%s' res=%d]\n",
                    file_name,
                    res);

        }
 
        printf(
                "Found %d users in image. [file_name='%s'(%d,%d)]\n",
                face_num,
                file_name,
                frame.cols,
                frame.rows);
      
        if (0 < face_num) {
            FaceDetectResult face_detect[face_num];
            face_sdk.getAllFaces(face_detect);

            for (int i = 0; i < face_num; ++i) {
                // 在原图中标记人脸区域
                if (face_detect[i].face_mask == 1) {
                    cv::rectangle(
                            frame,
                            cv::Point(face_detect[i].bbox[0], face_detect[i].bbox[1]),
                            cv::Point(face_detect[i].bbox[2], face_detect[i].bbox[3]),
                            COLOR_RED,
                            5 /* line width */);
                } else {
                    cv::rectangle(
                            frame,
                            cv::Point(face_detect[i].bbox[0], face_detect[i].bbox[1]),
                            cv::Point(face_detect[i].bbox[2], face_detect[i].bbox[3]),
                            COLOR_GREEN,
                            5 /* line width */);
                }

                // 若人脸质量合格，则提取人脸特征，并在底库中检索人脸
                int max_sim_score = 0;
                std::string sim_user;

                if (face_detect[i].quality >= FACE_QUALITY_THRESHOLD) {
                    std::unique_ptr<float[]> feature(new float[FRS_FEATURE_LENGTH]);
                    if (0 != (res = face_sdk.getFaceFeature(i, feature.get()))) {
                        printf(
                                "\tFailed to get face feature. [file_name='%s' res=%d]\n",
                                file_name,
                                res);
                        continue;
                    }

                    for (auto it = user_face.begin(); it != user_face.end(); ++it) {
                        int sim = face_sdk.faceSimilarity(feature.get(), it->second.get());
                        if (sim > max_sim_score) {
                            max_sim_score = sim;
                            sim_user = it->first;
                        }
                    }

                    cv::rectangle(
                            frame,
                            cv::Point(
                                    face_detect[i].bbox[0] + 5, 
                                    face_detect[i].bbox[1] + 5),
                            cv::Point(
                                    face_detect[i].bbox[2] - 5, 
                                    face_detect[i].bbox[3] - 5),
                            COLOR_YELLOW,
                            5 /* line width */);
                }

                char score_str[64];

                if (max_sim_score >= FACE_RECOG_SCORE_THRESHOLD) {
                    snprintf(
                            score_str, 
                            64, 
                            "%d %s(sim:%d)", 
                            face_detect[i].quality,
                            sim_user.c_str(),
                            max_sim_score);

                    cv::putText(
                            frame,
                            score_str,
                            cv::Point(face_detect[i].bbox[0], face_detect[i].bbox[1]),
                            cv::FONT_HERSHEY_SIMPLEX,
                            1.0, /* font scale */
                            COLOR_RED,
                            2.0 /* font thickness */);

                    printf(
                            "\t%s (sim:%d)",
                            sim_user.c_str(),
                            max_sim_score);

                    // 判断一下文件名，统计召回的准确率
                    recall_in_file = true;
                    recall += 1;
                    if (get_uid_from_fn(sim_user) == get_uid_from_fn(cmp_name)) {
                        precision += 1;
                    }
                } else {
                    snprintf(
                            score_str, 
                            64, 
                            "%d", 
                            face_detect[i].quality); 

                    cv::putText(
                            frame,
                            score_str,
                            cv::Point(face_detect[i].bbox[0], face_detect[i].bbox[1]),
                            cv::FONT_HERSHEY_SIMPLEX,
                            0.5, /* font scale */
                            COLOR_GREEN,
                            1.5 /* font thickness */);

                    printf("\tUnknown user");
                }
                
                printf(
                        "\t[%d,%d,%d,%d] quality:%d %s\n",
                        face_detect[i].bbox[0],
                        face_detect[i].bbox[1],
                        face_detect[i].bbox[2],
                        face_detect[i].bbox[3],
                        face_detect[i].quality,
                        face_detect[i].quality < FACE_QUALITY_THRESHOLD ? "LOW" : "");
            }
        }

        printf("cost %.4f ms.\n", (clock()-t)/1000.0);

        snprintf(
                file_name, 
                sizeof(file_name), 
                "%s/%s", 
                OUTPUT_IMAGES_PATH,
                cmp_name.c_str());

        cv::imwrite(file_name, frame);

        if (recall_in_file) {
            recall_file += 1;
        }
    }

    printf(
            "Recall: %.2f Precision: %.2f\n",
            float(recall_file) / image_names.size() * 100,
            float(precision) / recall * 100);

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

static std::string get_uid_from_fn(const std::string& fn) {
    size_t current = fn.find_first_of('-');
    return fn.substr(0, current);
}

