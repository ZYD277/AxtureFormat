/*!
 *  @brief     使用gumbo库解析html，生成控件树
 *  @details   google的gumbo库可以很好的解决html解析问题，规避了通过xml方式解析失败的问题
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.03
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef GUMBOPARSEMETHOD_H
#define GUMBOPARSEMETHOD_H

#include <QSharedPointer>
#include <gumbo.h>
#include "../util/fileutils.h"
#include "../head.h"
#include "htmlstruct.h"
#include "gumbonodewrapper.h"

namespace Html{

class GumboParseMethod : public RTextParseMethod
{
public:
    GumboParseMethod();
    ~GumboParseMethod();

    bool startParse(RTextFile * file) override;

    DomHtmlPtr getParsedResult(){return m_htmlResultPtr;}

private:
    void skipBomHead(RTextFile * file);
    bool parseFile(RTextFile * file);

    void parseBody(GumboNodeWrapper &bodyNode);
    void parseDiv(GumboNodeWrapper &divNode, DomNode *parentNode);

    NodeType getNodeType(GumboNodeWrapper &element, GumboNodeWrapper parentElement);

    void printBody(DomNode *node);
    inline void establishRelation(DomNode *parentNode,DomNode *childNode);

    //解析特定控件
    void parseNodeData(GumboNodeWrapper &element, NodeType type, DomNode *node);
    void parseButtonNodeData(GumboNodeWrapper &element, DomNode *node);
    void parseRadioButtonNodeData(GumboNodeWrapper &element, DomNode *node);
    void parseTextFieldNodeData(GumboNodeWrapper &element, DomNode *node);
    void parseImageNodeData(GumboNodeWrapper &element, DomNode *node);
    void parseTableNodeData(GumboNodeWrapper &element, DomNode *node);
    void parseTableCellNodeData(GumboNodeWrapper &element, DomNode *parentNode);

    void parseGroupNodeData(GumboNodeWrapper &element, DomNode *node);
    void parseLabelNodeData(GumboNodeWrapper &element, DomNode *node);

private:
    DomHtmlPtr m_htmlResultPtr;

    QString m_errorMsg;
    GumboOutput * m_gumboParser;
};

} //namespace Html

#endif // GUMBOPARSEMETHOD_H
