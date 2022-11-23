//
// Created by alvinzheng on 2019/11/29
//

#ifndef __FACE_RECOG_SDK_RETRIEVE_H__
#define __FACE_RECOG_SDK_RETRIEVE_H__

#ifdef _MSC_VER
#ifdef FACE_RECOG_SDK_EXPORTS
#define FACE_RECOG_API __declspec(dllexport)
#else
#define FACE_RECOG_API 
#endif
#else
#define FACE_RECOG_API __attribute__ ((visibility("default")))
#endif


#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <mutex>

#define ERROR_RETRIEVE_LIB_ID_EXIST -100
#define ERROR_RETRIEVE_LIB_ID_NOT_EXIST -101
#define ERROR_RETRIEVE_FEAT_ID_NOT_EXIST -102
#define ERROR_RETRIEVE_FEATURES_AND_IDS_UNMATCHED -103
#define ERROR_RETRIEVE_FEATURES_SIZE_OVERFLOW -104
#define ERROR_RETRIEVE_EMPTY_LIB_IDS -105
#define ERROR_RETRIEVE_FEAT_ID_EXIST -106
#define ERROR_RETRIEVE_INPUT_ARGUMENTS_WRONG -107


namespace facerecogsdk
{
    struct RetrievedFace {
        std::string feat_id;
        std::string lib_id;
        float score;
    };

    typedef std::map<std::string, std::shared_ptr<float>> FaceLib;

    class FaceRetrieve {
    public:
        
        FACE_RECOG_API FaceRetrieve(const int feat_length);
        FACE_RECOG_API ~FaceRetrieve();

        FACE_RECOG_API static std::string version();
        FACE_RECOG_API int feat_length();

        FACE_RECOG_API std::map<std::string, std::shared_ptr<FaceLib> > all_libs();
        FACE_RECOG_API float Compare(
                const float* feat0, 
                const float* feat1, 
                const bool is_centile);
        FACE_RECOG_API int Retrieve(
                const std::string* lib_ids, 
                const size_t lib_num, 
                const size_t top_n, 
                const float* feat, 
                const float threshold, 
                std::vector<RetrievedFace>& retrieved_face);
        FACE_RECOG_API int CreateLib(
                const float* feats, 
                const std::string* feat_ids, 
                const size_t feat_num, 
                const std::string lib_id);
        FACE_RECOG_API int InsertFeatures(
                const float* feats, 
                const std::string* feat_ids, 
                const size_t feat_num, 
                const std::string lib_id);
        FACE_RECOG_API int ClearLib(const std::string lib_id);
        FACE_RECOG_API int ClearAllLibs();
        FACE_RECOG_API size_t GetLibCount();
        FACE_RECOG_API int DeleteFeatures(
                const std::string* feat_ids, 
                const size_t feat_num, 
                const std::string lib_id);
        FACE_RECOG_API int UpdateFeature(
                const float* feat, 
                const std::string feat_id, 
                std::string lib_id);
        FACE_RECOG_API int GetFeatureCount(const std::string lib_id);
        FACE_RECOG_API int QueryFeature(
                const std::string feat_id, 
                const std::string lib_id, 
                float* feat);
        FACE_RECOG_API bool IsFeatureIdExist(
                const std::string feat_id, 
                const std::string lib_id);

    private:
        FaceRetrieve(const FaceRetrieve&) = delete;
        const FaceRetrieve& operator=(const FaceRetrieve&) = delete;

    private:
        static std::string version_;

        int feat_length_;
        std::map<std::string, std::shared_ptr<FaceLib> > all_libs_;

        std::mutex mutex_;
    };
} // end of namespace facerecogsdk

#endif  // __FACE_RECOG_SDK_RETRIEVE_H__

