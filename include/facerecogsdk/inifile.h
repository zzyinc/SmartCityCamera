#ifndef __FACE_RECOG_SDK_INIFILE_H__
#define __FACE_RECOG_SDK_INIFILE_H__

#include <vector>
#include <algorithm>
#include <string>
#include <string.h>
#if __ANDROID_API__ >= 9
#include <android/asset_manager.h>
#endif // __ANDROID_API__ >= 9

#include "common.h"

using std::string;
using std::vector;

#define RET_OK 0
// 没有找到匹配的]
#define ERR_UNMATCHED_BRACKETS 2
// 段为空
#define ERR_SECTION_EMPTY 3
// 段名已经存在
#define ERR_SECTION_ALREADY_EXISTS 4
// 解析key value失败
#define ERR_PARSE_KEY_VALUE_FAILED 5
// 打开文件失败
#define ERR_OPEN_FILE_FAILED 6
// 内存不足
#define ERR_NO_ENOUGH_MEMORY 7
// 没有找到对应的key
#define ERR_NOT_FOUND_KEY 8
// 没有找到对应的section
#define ERR_NOT_FOUND_SECTION 9

namespace facerecogsdk {
    const char delim[] = "\n";
    struct IniItem {
        string key;
        string value;
        string comment;  // 每个键的注释，都是指该行上方的内容
        string rightComment;
    };

    struct IniSection {
        typedef vector<IniItem>::iterator IniItem_it;  // 定义一个迭代器，即指向键元素的指针
        typedef vector<IniItem>::const_iterator IniItem_const_it;  // 定义一个迭代器，即指向键元素的指针

        IniItem_it begin() {
            return items.begin();  // 段结构体的begin元素指向items的头指针
        }

        IniItem_it end() {
            return items.end();  // 段结构体的begin元素指向items的尾指针
        }

        IniItem_const_it begin() const {
            return items.begin();  // 段结构体的begin元素指向items的头指针
        }

        IniItem_const_it end() const {
            return items.end();  // 段结构体的begin元素指向items的尾指针
        }

        string name;
        string comment;  // 每个段的注释，都是指该行上方的内容
        string rightComment;
        vector<IniItem> items;  // 键值项数组，一个段可以有多个键值，所有用vector来储存
    };

    class IniFile
    {
    public:
        FACE_RECOG_API IniFile();
        FACE_RECOG_API ~IniFile() {
            release();
        }

    public:
        /* 打开并解析一个名为fname的INI文件 */
        FACE_RECOG_API int Load(const string &fname);
#if __ANDROID_API__ >= 9
        FACE_RECOG_API int Load(AAssetManager* mgr, const string &fname);
#endif // __ANDROID_API__ >= 9
        /* 将内容保存到当前文件 */
        FACE_RECOG_API int Save();
        /* 将内容另存到一个名为fname的文件 */
        FACE_RECOG_API int SaveAs(const string &fname);

        /* 获取section段第一个键为key的string值，成功返回0，否则返回错误码 */
        FACE_RECOG_API int GetStringValue(const string &section, const string &key, string *value) const;
        /* 获取section段第一个键为key的int值，成功返回0，否则返回错误码 */
        FACE_RECOG_API int GetIntValue(const string &section, const string &key, int *value) const;
        /* 获取section段第一个键为key的double值，成功返回0，否则返回错误码 */
        FACE_RECOG_API int GetDoubleValue(const string &section, const string &key, double *value) const;
        /* 获取section段第一个键为key的bool值，成功返回0，否则返回错误码 */
        FACE_RECOG_API int GetBoolValue(const string &section, const string &key, bool *value) const;
        /* 获取注释，如果key=""则获取段注释 */
        FACE_RECOG_API int GetComment(const string &section, const string &key, string *comment) const;
        /* 获取行尾注释，如果key=""则获取段的行尾注释 */
        FACE_RECOG_API int GetRightComment(const string &section, const string &key, string *rightComment) const;

        /* 获取section段第一个键为key的string值，成功返回获取的值，否则返回默认值 */
        FACE_RECOG_API void GetStringValueOrDefault(const string &section, const string &key, string *value, const string &defaultValue) const;
        /* 获取section段第一个键为key的int值，成功返回获取的值，否则返回默认值 */
        FACE_RECOG_API void GetIntValueOrDefault(const string &section, const string &key, int *value, int defaultValue) const;
        /* 获取section段第一个键为key的double值，成功返回获取的值，否则返回默认值 */
        FACE_RECOG_API void GetDoubleValueOrDefault(const string &section, const string &key, double *value, double defaultValue) const;
        /* 获取section段第一个键为key的bool值，成功返回获取的值，否则返回默认值 */
        FACE_RECOG_API void GetBoolValueOrDefault(const string &section, const string &key, bool *value, bool defaultValue) const;

        /* 获取section段所有键为key的值,并将值赋到values的vector中 */
        FACE_RECOG_API int GetValues(const string &section, const string &key, vector<string> *values) const;

        /* 获取section列表,并返回section个数 */
        FACE_RECOG_API int GetSections(vector<string> *sections) const;
        /* 获取section个数，至少存在一个空section */
        FACE_RECOG_API int GetSectionNum() const;
        /* 是否存在某个section */
        FACE_RECOG_API bool HasSection(const string &section) const;
        /* 获取指定section的所有ken=value信息 */
        FACE_RECOG_API const IniSection* getSection(const string &section = "") const;
        FACE_RECOG_API IniSection* getMutableSection(const string &section = "");

        /* 是否存在某个key */
        FACE_RECOG_API bool HasKey(const string &section, const string &key) const;

        /* 设置字符串值 */
        FACE_RECOG_API int SetStringValue(const string &section, const string &key, const string &value);
        /* 设置整形值 */
        FACE_RECOG_API int SetIntValue(const string &section, const string &key, int value);
        /* 设置浮点数值 */
        FACE_RECOG_API int SetDoubleValue(const string &section, const string &key, double value);
        /* 设置布尔值 */
        FACE_RECOG_API int SetBoolValue(const string &section, const string &key, bool value);
        /* 设置注释，如果key=""则设置段注释 */
        FACE_RECOG_API int SetComment(const string &section, const string &key, const string &comment);
        /* 设置行尾注释，如果key=""则设置段的行尾注释 */
        FACE_RECOG_API int SetRightComment(const string &section, const string &key, const string &rightComment);

        /* 删除段 */
        FACE_RECOG_API void DeleteSection(const string &section);
        /* 删除特定段的特定参数 */
        FACE_RECOG_API void DeleteKey(const string &section, const string &key);
        /*设置注释分隔符，默认为"#" */
        FACE_RECOG_API void SetCommentDelimiter(const string &delimiter);

        FACE_RECOG_API const string &GetErrMsg();
        FACE_RECOG_API void release();
    private:
        /* 获取section段所有键为key的值,并将值赋到values的vector中,,将注释赋到comments的vector中 */
        int GetValues(const string &section, const string &key, vector<string> *value, vector<string> *comments) const;

        /* 同时设置值和注释 */
        int setValue(const string &section, const string &key, const string &value, const string &comment = "");

        /* 去掉str前面的c字符 */
        static void trimleft(string &str, char c = ' ');
        /* 去掉str后面的c字符 */
        static void trimright(string &str, char c = ' ');
        /* 去掉str前面和后面的空格符,Tab符等空白符 */
        static void trim(string &str);
        /* 将字符串str按分割符delim分割成多个子串 */
    private:
        int UpdateSection(const string &cleanLine, const string &comment,
            const string &rightComment, IniSection **section);
        int AddKeyValuePair(const string &cleanLine, const string &comment,
            const string &rightComment, IniSection *section);


        bool split(const string &str, const string &sep, string *left, string *right);
        bool IsCommentLine(const string &str);
        bool parse(const string &content, string *key, string *value);
        // for debug
        void print();

    private:
        /* 获取section段第一个键为key的值,并将值赋到value中 */
        int getValue(const string &section, const string &key, string *value) const;
        /* 获取section段第一个键为key的值,并将值赋到value中,将注释赋到comment中 */
        int getValue(const string &section, const string &key, string *value, string *comment) const;

        bool StringCmpIgnoreCase(const string &str1, const string &str2) const;
        bool StartWith(const string &str, const string &prefix) const;

    private:
        typedef vector<IniSection *>::iterator IniSection_it;
        typedef vector<IniSection *>::const_iterator IniSection_const_it;

        vector<IniSection *> sections_vt;  // 用于存储段集合的vector容器
        string iniFilePath;
        string commentDelimiter;
    };

} // end of namespace facerecogsdk

#endif  // __FACE_RECOG_SDK_INIFILE_H__
