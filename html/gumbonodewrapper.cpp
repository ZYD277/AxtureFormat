#include "gumbonodewrapper.h"

#include "../head.h"

namespace Html{

GumboNodeWrapper::GumboNodeWrapper():m_bValid(false),m_node(nullptr)
{

}

GumboNodeWrapper::GumboNodeWrapper(GumboNode *node):m_node(node),m_bValid(true){

}

/*!
 * @brief 匹配子节点中第一个符合的标签名
 * @param[in] tagName 标签名
 * @return 若成功返回对应节点包装，否则返回空对象
 */
GumboNodeWrapper GumboNodeWrapper::elementByTagName(QString tagName)
{
    return _iteratorChildren([&](const GumboVector & children)->GumboNodeWrapper{
        for(uint i = 0; i < children.length; i++){
            GumboNode * node = static_cast<GumboNode *>(children.data[i]);
            if(node->type == GUMBO_NODE_ELEMENT){
                if(node->v.element.tag == getTagByTagName(tagName))
                    return GumboNodeWrapper(node);
            }
        }
        return GumboNodeWrapper();
    });
}

/*!
 * @brief 匹配子节点中所有符合的标签名
 * @param[in] tagName 标签名
 * @return 若成功返回对应节点包装，否则返回空对象
 */
GumboNodeWrapperList GumboNodeWrapper::elementsByTagName(QString tagName)
{
    return _iteratorListChildren([&](const GumboVector & children,GumboNodeWrapperList & output){
        for(uint i = 0; i < children.length; i++){
            GumboNode * node = static_cast<GumboNode *>(children.data[i]);
            if(node->type == GUMBO_NODE_ELEMENT){
                if(node->v.element.tag == getTagByTagName(tagName))
                    output.push_back(GumboNodeWrapper(node));
            }
        }
    });
}

/*!
 * @brief 获取当前节点下直接子节点
 */
GumboNodeWrapperList GumboNodeWrapper::children()
{
    return _iteratorListChildren([&](const GumboVector & children,GumboNodeWrapperList & output){
        for(uint i = 0; i < children.length; i++){
            GumboNode * node = static_cast<GumboNode *>(children.data[i]);
            if(node->type == GUMBO_NODE_ELEMENT){
                output.push_back(GumboNodeWrapper(node));
            }
        }
    });
}

/*!
 * @brief 获取第一个元素子节点
 * @attention 若不存在子节点则返回空
 */
GumboNodeWrapper GumboNodeWrapper::firstChild()
{
    return _iteratorChildren([](const GumboVector & children)->GumboNodeWrapper{
        for(uint i = 0; i < children.length; i++){
            GumboNode * node = static_cast<GumboNode *>(children.data[i]);
            if(node->type == GUMBO_NODE_ELEMENT || node->type == GUMBO_NODE_TEXT){
                return GumboNodeWrapper(node);
            }
        }
        return GumboNodeWrapper();
    });
}

/*!
 * @brief 获取第二个元素子节点
 * @attention 若不存在子节点则返回空
 */
GumboNodeWrapper GumboNodeWrapper::secondChild()
{
    return _iteratorChildren([](const GumboVector & children)->GumboNodeWrapper{
        int count = 0;
        for(uint i = 0; i < children.length; i++){
            GumboNode * node = static_cast<GumboNode *>(children.data[i]);
            if(node->type == GUMBO_NODE_ELEMENT && ++count == 2){
                return GumboNodeWrapper(node);
            }
        }
        return GumboNodeWrapper();
    });
}

GumboNodeWrapper GumboNodeWrapper::lastChild()
{
    return _iteratorChildren([](const GumboVector & children)->GumboNodeWrapper{
        for(uint i = children.length - 1; i >= 0; i++){
            GumboNode * node = static_cast<GumboNode *>(children.data[i]);
            if(node->type == GUMBO_NODE_ELEMENT){
                return GumboNodeWrapper(node);
            }
        }
        return GumboNodeWrapper();
    });
}

GumboNodeWrapper::AttributeList GumboNodeWrapper::attributes()
{
    AttributeList attrs;
    do{
        if(!isValidElement()) break;

        GumboVector attrVector = m_node->v.element.attributes;
        for(uint i = 0; i < attrVector.length; i++){
            GumboAttribute* attr = static_cast<GumboAttribute*>(attrVector.data[i]);
            attrs.push_back(qMakePair(attr->name,attr->value));
        }
    }while(0);

    return attrs;
}

QString GumboNodeWrapper::attribute(QString attrName)
{
    QString result;
    do{
        attrName = attrName.trimmed();
        if(!isValidElement() || attrName.size() == 0) break;

        attrName = attrName.toLower();

        GumboVector attrVector = m_node->v.element.attributes;
        for(uint i = 0; i < attrVector.length; i++){
            GumboAttribute* attr = static_cast<GumboAttribute*>(attrVector.data[i]);
            if(strcmp(attr->name,attrName.toLocal8Bit().data()) == 0)
                return QString(attr->value);
        }
    }while(0);

    return result;
}

bool GumboNodeWrapper::hasAttribute(QString attrName)
{
    Check_Return(!isValidElement() || attrName.trimmed().size() == 0,false);

    GumboVector attrVector = m_node->v.element.attributes;
    for(uint i = 0; i < attrVector.length; i++){
        GumboAttribute* attr = static_cast<GumboAttribute*>(attrVector.data[i]);
        if(strcmp(attr->name,attrName.toLocal8Bit().data()) == 0)
            return true;
    }
    return false;
}

QString GumboNodeWrapper::id()
{
    return attribute(m_nodeHtml.ID);
}

QString GumboNodeWrapper::clazz()
{
    return attribute(m_nodeHtml.CLASS);
}

QString GumboNodeWrapper::style()
{
    return attribute(m_nodeHtml.STYLE);
}

QString GumboNodeWrapper::tagName()
{
    if(isValidElement() && m_node->type == GUMBO_NODE_ELEMENT){
        switch(m_node->v.element.tag){
            case GUMBO_TAG_HTML:return m_nodeHtml.HTML;break;
            case GUMBO_TAG_HEAD:return m_nodeHtml.HEAD;break;
            case GUMBO_TAG_TITLE:return m_nodeHtml.TITLE;break;
            case GUMBO_TAG_LINK:return m_nodeHtml.LINK;break;
            case GUMBO_TAG_META:return m_nodeHtml.META;break;
            case GUMBO_TAG_BODY:return m_nodeHtml.BODY;break;
            case GUMBO_TAG_DIV:return m_nodeHtml.DIV;break;
            case GUMBO_TAG_P:return m_nodeHtml.P;break;
            case GUMBO_TAG_SPAN:return m_nodeHtml.SPAN;break;
            default:break;
        }
    }
    return "";
}

/*!
 * @brief 获取节点文本
 * @attention 只在类型为GUMBO_NODE_TEXT时才能返回
 * @return 节点文本
 */
QString GumboNodeWrapper::text()
{
    if(isValidText()){
        return QString(m_node->v.text.text);
    }
    return "";
}

GumboNodeWrapper::NodeType GumboNodeWrapper::type() const
{
    if(m_bValid && m_node){
        switch(m_node->type){
            case GUMBO_NODE_ELEMENT:return WElement;break;
            case GUMBO_NODE_CDATA:
            case GUMBO_NODE_TEXT:return WTextNode;break;
            case GUMBO_NODE_COMMENT:return WComment;break;
            case GUMBO_NODE_DOCUMENT:return WDoc;break;
            case GUMBO_NODE_WHITESPACE:return WSpace;break;
            default:break;
        }
    }
    return WInvalid;
}

/*!
 * @brief 根据标签名获取标签类型
 * @param[in] tagName 标签名
 * @return 若存在对应的名称的类型，则返回，否则返回未知类型
 */
GumboTag GumboNodeWrapper::getTagByTagName(QString tagName)
{
    QString lower = tagName.toLower();
    if(lower.compare(m_nodeHtml.HEAD) == 0){
        return GUMBO_TAG_HEAD;
    }else if(lower.compare(m_nodeHtml.TITLE) == 0){
        return GUMBO_TAG_TITLE;
    }else if(lower.compare(m_nodeHtml.LINK) == 0){
        return GUMBO_TAG_LINK;
    }else if(lower.compare(m_nodeHtml.META) == 0){
        return GUMBO_TAG_META;
    }else if(lower.compare(m_nodeHtml.BODY) == 0){
        return GUMBO_TAG_BODY;
    }else if(lower.compare(m_nodeHtml.DIV) == 0){
        return GUMBO_TAG_DIV;
    }else if(lower.compare(m_nodeHtml.P) == 0){
        return GUMBO_TAG_P;
    }else if(lower.compare(m_nodeHtml.SPAN) == 0){
        return GUMBO_TAG_SPAN;
    }

    return GUMBO_TAG_UNKNOWN;
}

bool GumboNodeWrapper::isValidElement()
{
    if(!m_bValid || m_node->type != GUMBO_NODE_ELEMENT)
        return false;
    return true;
}

bool GumboNodeWrapper::isValidText()
{
    if(!m_bValid || m_node->type != GUMBO_NODE_TEXT)
        return false;
    return true;
}

GumboNodeWrapper GumboNodeWrapper::_iteratorChildren(GumboNodeWrapper::IterCallback callback)
{
    do{
        if(!isValidElement() || callback == nullptr) break;

        GumboVector childs = m_node->v.element.children;
        if(childs.length == 0) break;

        return callback(childs);

    }while(0);

    return GumboNodeWrapper();
}

GumboNodeWrapperList GumboNodeWrapper::_iteratorListChildren(GumboNodeWrapper::ListIterCallback callback)
{
    GumboNodeWrapperList nlist;
    do{
        if(!isValidElement() || callback == nullptr) break;

        GumboVector childs = m_node->v.element.children;
        if(childs.length == 0) break;

        callback(childs,nlist);
    }while(0);

    return nlist;
}

} //namespace Html
