#ifndef _SCHEMA_MIN_INCLUSIVE_HPP_
#define _SCHEMA_MIN_INCLUSIVE_HPP_

#include "SchemaCommon.hpp"

class CMinInclusive : public CXSDNode
{
public:

    virtual ~CMinInclusive()
    {
    }

    static CMinInclusive* load(CXSDNodeBase* pParentNode, const IPropertyTree *pSchemaRoot, const char* xpath);

    virtual void dump(std::ostream& cout, unsigned int offset = 0) const;

    virtual void getDocumentation(StringBuffer &strDoc) const
    {
        assert(!"Not Implemented");
    }

    virtual void getDojoJS(StringBuffer &strJS) const
    {
        assert(!"Not Implemented");
    }

    void getQML(StringBuffer &strQML, int idx = -1) const
    {
        assert(!"Not Implemented");
    }

    virtual const char* getXML(const char* /*pComponent*/)
    {
        assert(!"Not Implemented");
    }

    virtual void populateEnvXPath(StringBuffer strXPath, unsigned int index = 1)
    {
        assert(!"Not Implemented");
    }

    virtual void loadXMLFromEnvXml(const IPropertyTree *pEnvTree)
    {
        assert(!"Not Implemented");
    }

    GETTERSETTER(Value)
    GETTERSETTERINT(MinInclusive)

private:

    CMinInclusive() : CXSDNode::CXSDNode(NULL, XSD_MIN_INCLUSIVE), m_nMinInclusive(-1)
    {
    }

};

#endif // _SCHEMA_MIN_INCLUSIVE_HPP_
