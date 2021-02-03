#ifndef CPPSTRUCT_H
#define CPPSTRUCT_H

#include <QStringList>
#include <QMap>

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
    CUSTOM_TABLE_WIDGET,     /*!< 自定义表格样式 */
    CUSTOM_DROP_BUTTON,     /*!< 自定义下拉按钮 */
    CUSTOM_KEYBOARD_RFIELD,     /*!< 自定义输入框 */
    CUSTOM_BIDIRECTIONAL_SLIDER,     /*!< 自定义双向滚动条 */
    CUSTOM_SIGNAL_SLIDING,     /*!< 自定义单滑块 */
    CUSTOM_FLOATING_WINDOW,     /*!< 自定义悬浮窗 */
    CUSTOM_FOLDING_CONTROL,    /*!< 自定义折叠控件 */
    CUSTOM_SWITCH_BUTTON,    /*!< 自定义开关按钮 */
    CUSTOM_DROPDOWN_BOX,    /*!< 自定义开关按钮 */
    CUSTOM_BUTTON_GROUP,    /*!< 自定义按钮组 */
    CUSTOM_SWITCH_PULLDOWN_BUTTON,    /*!< 自定义带有下拉框的开关按钮 */
    CUSTOM_SELECTOR_MENU,   /*!< 选择器菜单 */
    CUSTOM_LABEL,   /*!< 自定义label */
    CUSTOM_BUTTON_ICON,   /*!< 带有图标的按钮 */
    CUSTOM_DOUBLEFOLDING  /*!< 双层折叠控件 */

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

/////////////////// 自定义输入框/////////////////////////////

/**
 * @brief 各部分基础属性
 */
struct Location{
    Location():m_width(0),m_height(0),m_top(0),m_left(0){}
    int m_width;
    int m_height;
    int m_top;
    int m_left;
};

/**
 * @brief 部分控件包含图标
 */
struct IconInfo{
    Location m_location;
    QString m_srcIcon;
    QString m_ID;
};

/**
 * @brief 各部分基础属性
 */
struct BassAttribute {
	QString m_ID;
	QString m_text;
	QString m_srcImage;
	Location m_location;
    IconInfo m_iconInfo;
};

/**
 * @brief按钮属性
 */
struct BUTTON{
    BUTTON():deleteBtn(false){}
   BassAttribute m_baseAtrribute;
   Location m_location;
   bool deleteBtn;
};

/**
 * @brief 触发输入框部分
 */
struct TriggerInputBox{
   BassAttribute m_baseAtrribute;
   Location m_location;
};

/**
 * @brief TITLE
 */
struct TITLE{
   BassAttribute m_baseAtrribute;
   bool m_tytle;
   Location m_location;
};
/**
 * @brief 虚拟键盘按键区
 */
struct THEKEYS{
   BassAttribute m_baseAtrribute;
   QList<BUTTON> m_keyBtns;
   Location m_location;
};
/**
 * @brief 尾部确定取消按钮组
 */
struct TAIL{
   BassAttribute m_baseAtrribute;
   QList<BUTTON> m_quitBtns;
   Location m_location;
};
/**
 * @brief 虚拟键盘部分
 */
struct VirtualKeyboard{
    BassAttribute m_baseAtrribute;
    THEKEYS m_keyGroup;
    TITLE m_title;
    TAIL m_tailGroup;
    Location m_location;
    QString m_ID;
};

/*!
 * @brief 自定义输入框
 */
struct KeyBoardInputBoxData : public AbstractCppCodeData{
    KeyBoardInputBoxData():AbstractCppCodeData(CUSTOM_KEYBOARD_RFIELD){}
    ~KeyBoardInputBoxData(){}

    TriggerInputBox m_triggerInputBox;//触发编辑框
    VirtualKeyboard m_virtualKeyGroup;//虚拟键盘
    QString m_inputBoxId;

};


////////////////////////////自定义下拉框按钮///////////////////////////////////

struct TextInfo{
   QString m_text;
   QString m_color;
   QString m_font;
};

struct BaseInfo{
    Location m_location;
    TextInfo m_textInfo;
    QString m_srcImg;
    QString m_style;
    QString m_dataLabel;
    QString m_className;
    QString m_ID;
};

/**
 * @brief 按钮信息
 */
struct ButtonInfo{
    ButtonInfo():deleteBtn(false){}
    BaseInfo m_baseInfo;
    bool deleteBtn;//用于标记按钮组组中的删除按钮
};

/**
 * @brief 选项触发按钮
 */
struct OptionTriggerButton{
    QString m_ID;
    Location m_location;
    BaseInfo m_iconInfo;
    BaseInfo m_buttonInfo;
};

/**
 * @brief 边框
 */
struct TheBorder{
    BaseInfo m_baseInfo;
};

struct TitleInfo{
    BaseInfo m_baseInfo;
};
/**
 * @brief 按钮组
 */
struct ButtonGroup{
    Location m_location;
    QString m_ID;
    QList<ButtonInfo> m_buttons;
};

/**
 * @brief 选项弹出框
 */
struct OptionPopUp
{
    QString m_ID;
    Location m_location;
    TheBorder m_border;
    TitleInfo m_titleInfo;
    ButtonGroup m_optionBtnGroup;
    ButtonGroup m_exitBtnGroup;
};

/*!
 * @brief 自定义下拉框按钮
 */
struct DropDownButtonData : public AbstractCppCodeData{
    DropDownButtonData():AbstractCppCodeData(CUSTOM_DROP_BUTTON){}
    ~DropDownButtonData(){}

    OptionTriggerButton m_triggerBtnOpen;//选项触发按钮打开状态
    OptionTriggerButton m_triggerBtnClose;//选项触发关闭状态
    OptionPopUp m_optionPopUp;//选项弹出框
    QString m_ID;

};

/// //////////////自定义双向滚动条/////////////////////////////////

/**
 * @brief 文字提示
 */
struct TheTooltip{
    QString m_ID;
    Location m_location;
    BaseInfo m_textBox;  /*!< 文本框 */
};

/**
 * @brief 滚动条
 */
struct DScrollBar{
    Location m_location;
    QString m_id;
    QList<BaseInfo> m_srcImgs;
};

struct BidirectionalSlider : public AbstractCppCodeData{
    BidirectionalSlider():AbstractCppCodeData(CUSTOM_BIDIRECTIONAL_SLIDER){}
    ~BidirectionalSlider(){}

    QList<BaseInfo> m_rectangularsInfo;
    Location m_location;
    DScrollBar m_leftScrollBar;  /*!< 左滚动条 */
    TheTooltip m_leftToolTip;        /*!< 左文字提示 */
    TheTooltip m_rightToolTip;        /*!< 左文字提示 */
    DScrollBar m_rightScrollBar;     /*!< 右滚动条 */
    QString m_ID;
};
////////////////单滑块//////////////////////////

struct InputBoxInfo{
    QString m_ID;
    Location m_location;
    QList<BaseInfo> m_inputBoxInfos;  /*!< 输入框 */
};
/**
 * @brief 自定义单滑块
 */
struct SingleSlidingBlockData : public AbstractCppCodeData{
    SingleSlidingBlockData():AbstractCppCodeData(CUSTOM_SIGNAL_SLIDING){}
    ~SingleSlidingBlockData(){}

    QList<BaseInfo> m_rectangularsInfo; /*!< 内部组成部分矩形信息 */
    DScrollBar m_leftScrollBar;  /*!< 左滑块 */
    TheTooltip m_letfToolTip;   /*!< 左文字提示 */
    InputBoxInfo m_inputBox;    /*!< 文字输入框 */
    Location m_location;
    QString m_ID;
};
////////////////////自定义悬浮窗/////////////////////////////////
/**
 * @brief 开关按钮
 */
struct SwitchButton{
    BaseInfo m_openState;    /*!< 打开状态 */
    BaseInfo m_closeState;    /*!< 关闭状态 */
    QString m_id;
    Location m_location;
};
/**
 * @brief 主窗体部分：由开关按钮组成
 */
struct MainWidget{
    QList<SwitchButton> m_switchButtons;
    Location m_location;
    QString m_srcImg;
    QString m_id;
};

/**
 * @brief 悬浮按钮，控制界面的显示隐藏
 */
struct FloatButton{
    Location m_location;
    QList<BaseInfo> m_srcImgs;
};

struct FloatingWindow : public AbstractCppCodeData{
    FloatingWindow():AbstractCppCodeData(CUSTOM_FLOATING_WINDOW){}
    ~FloatingWindow(){}

    MainWidget m_mainWidget;
    FloatButton m_floatButton;
    Location m_location;
    QString m_ID;
};

/////////////////////////////////////////////
/// \brief 自定义折叠信息控件
///

/**
 * @brief 折叠信息
 */
struct FoldingInfo{
    Location m_location;
    QString m_ID;
    QList<BaseInfo> m_information;
};

/**
 * @brief 展开信息
 */
struct UnFoldInfo{
    UnFoldInfo(): m_autoSetControl(true){}
    QString m_parentID;//当前展开信息所在的动态面板的id
    Location m_parentLocation;//当前展开信息所在动态面板的位置信息
    Location m_location;
    QString m_ID;
    bool m_autoSetControl;   /*!< 有些需要手动设置控件类型以及布局 */
    QList<BaseInfo> m_information;
};
struct Information{
    FoldingInfo m_foldingInfo;
    UnFoldInfo m_unFoldInfo;
    QString m_ID;
    Location m_location;
};
struct FoldingControls : public AbstractCppCodeData{
    FoldingControls():AbstractCppCodeData(CUSTOM_FOLDING_CONTROL),m_addScrollBar(true){}
    ~FoldingControls(){}

    QList<Information> m_informations;
    Location m_location;
    QString m_ID;
    bool m_addScrollBar;   /*!< 有些控件不需要添加滚动条 */
};

//////////////////自定义开关按钮/////////////////////////////

struct customSwitchButton : public AbstractCppCodeData{
	customSwitchButton():AbstractCppCodeData(CUSTOM_SWITCH_BUTTON){}
    ~customSwitchButton(){}

    QList<BaseInfo> m_textParagraphs;
    QList<BaseInfo> m_lineEdits;
    BaseInfo m_closeState;
    BaseInfo m_openState;
    QString m_ID;
    Location m_location;
};

///////////////////下拉弹框///////////////////////////
struct DropDownBoxData : public AbstractCppCodeData{
    DropDownBoxData():AbstractCppCodeData(CUSTOM_DROPDOWN_BOX){}
    ~DropDownBoxData(){}

    QStringList m_optionIdList; /*!< 弹框选项id */
    QString m_buttonID;            /*!< 按钮id */
    QString m_widgetID;          /*!< 弹框id */
};

/**
 * @brief 自定义带有下拉框的开关按钮
 */

struct SwitchPullDownButtonData : public AbstractCppCodeData{
    SwitchPullDownButtonData():AbstractCppCodeData(CUSTOM_SWITCH_PULLDOWN_BUTTON){}
    ~SwitchPullDownButtonData(){}

    QStringList m_optionIdList; /*!< 弹框选项id */
    QString m_popupWidgetID;    /*!< 弹框id */
    QString m_triggerPopupBtnID;  /*!< 触发弹窗按钮id */
    QString m_openBtnID;        /*!< 开状态按钮id */
    QString m_openPageID;        /*!< 开状态分页面ID */
    QString m_closeBtnID;       /*!< 关状态按钮id */
    QString m_closePageID;        /*!< 关状态分页面ID */
    QString m_stackedWidgetID;  /*!< 动态面板id */
    QString m_icon;             /*!< 按钮图标 */
    QString m_datalabel;

};

/////////////////带有弹框的自定义标签控件//////////////////////、

/**
 * @brief 标签弹窗
 */
struct LabelPopupWindow
{
    QString m_ID;
    Location m_location;
    QList<BaseInfo> m_optionsInfo;   /*!< 弹窗选项 */
};

/**
 * @brief 带有弹框的自定义标签控件
 */

struct CustomLabelData : public AbstractCppCodeData{
    CustomLabelData():AbstractCppCodeData(CUSTOM_LABEL){}
    ~CustomLabelData(){}

    LabelPopupWindow m_labelPopupWindow;
    BaseInfo m_defaultInfo;  /*!< 标签默认状态信息 */
    BaseInfo m_mouseEnter;    /*!< 鼠标进入标签状态信息 */
    QString m_ID;        /*!< Label控件ID */
};

/**
 * @brief 自定义选择器下拉菜单
 */

struct TheSelectorMenu : public AbstractCppCodeData{
    TheSelectorMenu():AbstractCppCodeData(CUSTOM_SELECTOR_MENU){}
    ~TheSelectorMenu(){}

    QStringList m_optionIdList; /*!< 弹框选项id */
    QString m_popupWidgetID;    /*!< 弹框id */
    QString m_triggerPopupBtnID;  /*!< 触发弹窗按钮id */
    QString m_defaultBtnID;        /*!< 默认按钮ID */
    QString m_defaultPageID;        /*!< 开状态分页面ID */
    QString m_pressedBtnID;       /*!< 关状态按钮id */
    QString m_pressedPageID;        /*!< 关状态分页面ID */
    QString m_stackedWidgetID;  /*!< 动态面板id */
    QString m_icon;             /*!< 按钮图标 */

};

/**
 * @brief 自定义双层折叠面板代码数据区，用于切换动态面板两个状态
 */

struct DoubleFoldingPanel : public AbstractCppCodeData{
    DoubleFoldingPanel():AbstractCppCodeData(CUSTOM_DOUBLEFOLDING){}
    ~DoubleFoldingPanel(){}

    QString foldingBtnID;        /*!< 折叠按钮id */
    QString foldingPageID;       /*!< 折叠页面ID */
    QString unFlodBtnID;        /*!< 展开按钮id */
    QString unFoldPageID;          /*!< 展开页面ID */
    QString m_stackedWidgetID;  /*!< 动态面板id */

};

/*!
 * @brief  按钮组控制代码
 */
struct ButtonGroupCodeData : public AbstractCppCodeData{
    ButtonGroupCodeData():AbstractCppCodeData(CUSTOM_BUTTON_GROUP){}
    ~ButtonGroupCodeData(){}

    QStringList m_buttIds;     /*!< 互斥按钮的Id信息集合 */
};

/*!
 * @brief  带图标按钮设置数据
 */
struct ButtonWithIcon : public AbstractCppCodeData{
    ButtonWithIcon():AbstractCppCodeData(CUSTOM_BUTTON_ICON){}
    ~ButtonWithIcon(){}

    QString m_srcIcon;
    QString m_ID;
};
/*!
 * @brief 互斥按钮控件代码数据区
 */
struct MutexButtonCodeData : public AbstractCppCodeData{
    MutexButtonCodeData():AbstractCppCodeData(MUTEX_BUTTON){}
    ~MutexButtonCodeData(){}

    QStringList m_buttIds;     /*!< 互斥按钮的Id信息集合 */
};

struct TabRowInfo{
    int m_rowNumbers;    /*!< 行号 */
    int m_height;        /*!< 行高 */
};

struct TabColumnInfo{

    int m_columnNumbers;     /*!< 列号 */
    int m_width;             /*!< 列宽 */
};

/*!
 * @brief 表格样式表代码数据区
 */
struct TableStyleCodeData : public AbstractCppCodeData{
    TableStyleCodeData():AbstractCppCodeData(CUSTOM_TABLE_WIDGET){}
    ~TableStyleCodeData(){}

    QList<TabRowInfo> m_tabRowInfos;
    QList<TabColumnInfo> m_tabColumnInfos;
    QString m_tableId;          /*!< 表格的id */
    QString m_tableStyle;       /*!< 表格样式 */
    QString m_horizontalStyle;  /*!< 水平表头样式 */
    QString m_verticalStyle;    /*!< 竖直表头样式 */
};


}


#endif // CPPSTRUCT_H
