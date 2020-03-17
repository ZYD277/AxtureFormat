#ifndef HTMLSTRUCT_H
#define HTMLSTRUCT_H

#include <QString>
#include <QStringList>
#include <QSharedPointer>

namespace Html{

struct NodeHtml{
    //文档结构
    const QString HTML = "html";
    const QString HEAD = "head";
    const QString BODY = "body";
    const QString TITLE = "title";
    const QString META = "meta";
    const QString LINK = "link";
    const QString SCRIPT = "script";

    //元素
    const QString DIV = "div";
    const QString INPUT = "input";
    const QString P = "p";
    const QString SPAN = "span";
    const QString IFRAME = "iframe";

    //属性
    const QString ID = "id";
    const QString CLASS = "class";
    const QString STYLE = "style";
    const QString TYPE = "type";
    const QString SRC = "src";
    const QString HREF = "href";
    const QString VALUE = "value";
    const QString CHECKED = "checked";
    const QString DISABLED = "disabled";
    const QString READONLY = "readonly";
    const QString MAX_LEN = "maxlength";
};

/*!
 * @brief 控件类型
 */
enum NodeType{
    RINVALID,
    RCONTAINER,             /*!< 容器 */
    RGROUP,                 /*!< 分组 */
    RBUTTON,                /*!< 按钮 */
    RDYNAMIC_PANEL,         /*!< StackWidget */
    RDYNAMIC_PANEL_PAGE,    /*!< StackWidget页 */
    RTEXT_FIELD,            /*!< 单行文本框 */
    RRADIO_BUTTON,          /*!< 单选框 */
    RTABLE,                 /*!< 表格 */
    RTABLE_CELL,            /*!< 表格单元格 */
    RIMAGE,                 /*!< 图片 */
    RLABEL,                 /*!< 文本显示 */
    RLINE,                  /*!< 水平/竖直线条*/
    RLIST_BOX,              /*!< 列表框 */
    RTEXT_AREA,             /*!< 文本框 */
    RCHECKBOX,              /*!< 复选框 */
    RDROPLIST,              /*!< 下拉框 */
    RBOX,                   /*!< 矩形 */
    RINLINE_FRAME,          /*!< 内联窗口 */
    RTREE                   /*!< 树形控件*/
};

/*!
 * @brief head中link节点
 */
struct DomHead{
    QString m_title;
    QStringList m_links;
    QStringList m_scripts;
};

/*!
 * @brief 控件基础通用属性
 */
struct BaseData{
    BaseData():m_bChecked(false),m_bDisabled(false),m_bReadOnly(false),m_bLeftToRight(true){}
    virtual ~BaseData(){}

    QString m_text;         /*!< 控件显示内容 */
    QString m_toolTip;      /*!< 工具提示信息，鼠标移入显示 */
    QString m_inlineStyle;  /*!< 内嵌的样式信息 */
    QString m_srcImage;     /*!< 背景图片信息*/
    bool m_bChecked;        /*!< 是否默认选中：checkbox、radiobutton等需要选择的有效 */
    bool m_bDisabled;
    bool m_bReadOnly;       /*!< 只读 */
    bool m_bLeftToRight;    /*!< 布局默认从左至右 */
};

/*!
 * @brief 图像数据
 */
struct ImageData : public BaseData{
    ~ImageData(){}

    QString m_src;          /*!< 图片链接:img节点特有 */
};

/*!
 * @brief 动态面板数据
 */
struct PanelData : public BaseData{
    ~PanelData(){}

    QString m_data_label;   /*!< 数据标签:Dynamic Panel等标签 */
};

/*!
 * @brief 表格数据
 */
struct TableData : public BaseData{
    ~TableData(){}

    int m_rows;
    int m_colums;
    QStringList m_items;
    QString m_itemId;
};

struct TreeItemData : public BaseData{
    TreeItemData():m_parentItem(nullptr){}
    ~TreeItemData(){
        m_parentItem = nullptr;

        foreach(auto pp,m_childItems)
            delete pp;
    }

    TreeItemData * m_parentItem;                /*!< 父节点 */
    QList<TreeItemData *>   m_childItems;       /*!< 子节点 */
    QString m_childItemId;                  /*!< 子节点id用于tree的样式*/
};

/*!
 * @brief tree控件数据
 */
struct TreeData : public BaseData{
    ~TreeData(){}

    QStringList m_dataCoIt;
    QStringList m_colums;
    QStringList m_items;

    QString m_treeItems;
};

/*!
 * @brief 分组数据
 */
struct GroupData : public BaseData{
    GroupData():m_left(0),m_top(0),m_width(0),m_height(0){}
    ~GroupData(){}

    int m_left;
    int m_top;
    int m_width;
    int m_height;
};

/*!
 * @brief 单行文本编辑框
 */
struct TextFieldData : public BaseData{
    TextFieldData():m_maxLength(32767){}

    QString m_type;         /*!< 文本类型：password、text等 */
    QString m_placeHolders; /*!< 文本占位符，生成在js中，无法提取 */
    int m_maxLength;        /*!< 文本最大长度 */
};

/*!
 * @brief 单列表或下拉框列表
 */
struct ListData : public BaseData{
    ListData(){}

    QStringList m_itemList;     /*!< 选项集合 */
    QString m_selectedValue;    /*!< 默认选择项 */
};

/*!
 * @brief 元素节点属性
 * @details 元素节点可存在样式属性、节点类型属性、子节点属性、父亲节点属性
 */
struct DomNode{
    DomNode(NodeType type):m_type(type),m_parent(nullptr),m_data(nullptr){}
    DomNode():DomNode(RINVALID){}

    ~DomNode(){
        m_parent = nullptr;

        foreach(DomNode * node,m_childs){
            delete node;
        }

        if(m_data)
            delete m_data;
    }

    NodeType m_type;        /*!< 元素类型 */

    QString m_id;           /*!< id属性 */
    QString m_class;        /*!< class属性 */
    QString m_style;        /*!< 控件中设置的样式属性 */

    QString m_treeText;

    BaseData * m_data;      /*!< 元素所具备的数据信息 */

    DomNode * m_parent;     /*!< 父节点,body节点parent为nullptr */
    QList<DomNode *> m_childs;     /*!< 子节点集合 */
};

/*!
 * @brief html文档节点结构
 * @details html文档解析后生成树形结构对象
 */
struct DomHtml{
    DomHead * head;         /*!< head节点 */
    DomNode * body;         /*!< body节点 */
};

} //namespace Html

typedef QSharedPointer<Html::DomHtml> DomHtmlPtr;

#endif // HTMLSTRUCT_H
