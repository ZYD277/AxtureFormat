#ifndef CPPSTRUCT_H
#define CPPSTRUCT_H

#include <QStringList>

namespace CXX{

/*!
 * @brief 代码模板类型
 * @details 根据每种定制化控件都需要产生特定的代码，定义了代码生成类型
 */
enum CodeType{
    MODEL_SWITCH,           /*!< 模式切换按钮控件 */
    TW_SWITCH,              /*!< 台位切换按钮控件 */
    PAGE_SWITCH,            /*!< 页面切换按钮控件 */
    PLAY_CONTROL,           /*!< 回放控制按钮控件 */
    MUTEX_BUTTON,           /*!< 互斥按钮控件 */
    CUSTOM_TABLE_WIDGET     /*!< 自定义表格样式 */
};

/*!
 * @brief 生成代码所需的基本信息基类
 */
struct AbstractCppCodeData{
    AbstractCppCodeData(CodeType type):m_type(type){}
    virtual ~AbstractCppCodeData(){}

    CodeType m_type;            /*!< 子类类型 */
};

typedef QList<AbstractCppCodeData *> CppCodeDatas;

/*!
 * @brief 模式切换控件代码数据区
 */
struct ModelSwitchCodeData : public AbstractCppCodeData{
    ModelSwitchCodeData():AbstractCppCodeData(MODEL_SWITCH){}
    ~ModelSwitchCodeData(){}

    QStringList m_modelIds;     /*!< 各个触发按钮的Id信息集合 */
};

/*!
 * @brief 模式切换控件代码数据区
 */
struct PlayControlCodeData : public AbstractCppCodeData{
    PlayControlCodeData():AbstractCppCodeData(PLAY_CONTROL){}
    ~PlayControlCodeData(){}

    QString m_stackedWidgetId;  /*!< 动态面板ID，作为按钮切换的容器 */
    QStringList m_modelIds;     /*!< 各个触发按钮的Id信息集合 */
};

/*!
 * @brief 页面切换代码区
 */
struct PageSwitchCodeData : public AbstractCppCodeData{
    PageSwitchCodeData():AbstractCppCodeData(PAGE_SWITCH){}
    ~PageSwitchCodeData(){}

    QStringList m_pageIds;      /*!< 页码按钮ID */
    QString m_leftPage;         /*!< 左、右翻页码 */
    QString m_rightPage;
};

/*!
 * @brief 台位切换控件代码数据区
 */
struct TWSwitchCodeData : public AbstractCppCodeData{
    TWSwitchCodeData():AbstractCppCodeData(TW_SWITCH){}
    ~TWSwitchCodeData(){}

    QList<QPair<QString, QString> > m_buttIds;      /*!< 各个台位触发按钮的Id信息集合:key:id,value:索引编号 */
    QString m_buttContainerId;  /*!< 台位切换按钮外层容器id */

    QString m_twContainerId;    /*!< 台位切换容器ID */
    QString m_twPopButtId;      /*!< 台位弹出按钮id */
};

/*!
 * @brief 互斥按钮控件代码数据区
 */
struct MutexButtonCodeData : public AbstractCppCodeData{
    MutexButtonCodeData():AbstractCppCodeData(MUTEX_BUTTON){}
    ~MutexButtonCodeData(){}

    QStringList m_buttIds;     /*!< 互斥按钮的Id信息集合 */
};

/*!
 * @brief 表格样式表代码数据区
 */
struct TableStyleCodeData : public AbstractCppCodeData{
    TableStyleCodeData():AbstractCppCodeData(CUSTOM_TABLE_WIDGET){}
    ~TableStyleCodeData(){}

    QString m_tableId;          /*!< 表格的id */
    QString m_tableStyle;       /*!< 表格样式 */
    QString m_horizontalStyle;  /*!< 水平表头样式 */
    QString m_verticalStyle;    /*!< 竖直表头样式 */
};


}


#endif // CPPSTRUCT_H
