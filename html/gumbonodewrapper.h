/*!
 *  @brief     封装gumbo的node节点,提供便捷访问操作
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.03
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef GUMBONODEWRAPPER_H
#define GUMBONODEWRAPPER_H

#include <functional>
#include <QString>
#include <QList>
#include <gumbo.h>
#include "htmlstruct.h"

namespace Html{

class GumboNodeWrapper;
typedef QList<GumboNodeWrapper> GumboNodeWrapperList;

class GumboNodeWrapper{
public:
    GumboNodeWrapper();
    GumboNodeWrapper(GumboNode * node);

    /*!< 子节点操作 */
    GumboNodeWrapper elementByTagName(QString tagName);
    GumboNodeWrapperList elementsByTagName(QString tagName);
    GumboNodeWrapperList children();

    GumboNodeWrapper firstChild();
    GumboNodeWrapper secondChild();
    GumboNodeWrapper lastChild();

    typedef QPair<QString,QString> AttributePair;
    typedef QList<AttributePair> AttributeList;

    /*!< 属性操作 */
    AttributeList attributes();
    QString attribute(QString attrName);
    QString id();
    QString clazz();

    bool valid() const{return m_bValid;}

    QString tagName() const;
    QString text() const;

    enum NodeType{
        WInvalid,
        WDoc,
        WElement,
        WTextNode,
        WComment,
        WSpace
    };

    NodeType type() const;

private:
    GumboTag getTagByTagName(QString tagName);
    inline bool isValidElement() const;
    inline bool isValidText() const;

    typedef std::function<GumboNodeWrapper(const GumboVector &)> IterCallback;
    GumboNodeWrapper _iteratorChildren(IterCallback callback);

    typedef std::function<void(const GumboVector &,GumboNodeWrapperList&)> ListIterCallback;
    GumboNodeWrapperList _iteratorListChildren(ListIterCallback callback);

private:
    GumboNode * m_node;
    bool m_bValid;
};

} //namespace Html

#endif // GUMBONODEWRAPPER_H
