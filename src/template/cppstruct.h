#ifndef CPPSTRUCT_H
#define CPPSTRUCT_H

namespace CXX{

/*!
 * @brief 代码模板类型
 * @details 根据每种定制化控件都需要产生特定的代码，定义了代码生成类型
 */
enum CodeType{
    MODEL_SWITCH,           /*!< 模式切换按钮控件 */
    TW_SWITCH,              /*!< 台位切换按钮控件 */
    PAGE_SWITCH             /*!< 页面切换按钮控件 */
};

}


#endif // CPPSTRUCT_H
