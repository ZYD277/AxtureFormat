/*!
 *  @brief     qss文件输出接口
 *  @details   1.支持手动设置导出目录
 *             2.支持设置普通样式和动态样式
 *  @attention 动态样式会将其转换成qt的动态样式，如QLineEdit[urgent=true]格式，动态样式需要在代码中手动的调用setProperty("urgent",true);
 *             然后再手动的对其调用unpolish()和polish()方法，达到样式更新效果。
 *  @author    wey
 *  @version   1.0
 *  @date      2020.02.10
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef QSSOUTPUT_H
#define QSSOUTPUT_H

#include <QString>
#include <QStringList>
#include "../../css/cssstruct.h"

namespace RQt{

class QSSOutput
{
public:
    QSSOutput();

    void setCommonStyle(const CSS::CssMap & globalCss,const CSS::CssMap & pageCss);
    void setDynamicStyle();
    bool save(QString fullPath);

private:
    CSS::CssMap m_globalCss;
    CSS::CssMap m_pageCss;
};

} //namespace RQt

#endif // QSSOUTPUT_H
