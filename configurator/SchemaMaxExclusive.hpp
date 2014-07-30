#ifndef _SCHEMA_MAX_EXCLUSIVE_HPP_
#define _SCHEMA_MAX_EXCLUSIVE_HPP_

#include "SchemaCommon.hpp"

class CMaxExclusive : public CXSDNode
{
    GETTERSETTER(Value)
    GETTERSETTERINT(MaxExclusive)

public:

    virtual ~CMaxExclusive()
    {
    }

    static CMaxExclusive* load(CXSDNodeBase* pParentNode, const IPropertyTree *pSchemaRoot, const char* xpath);

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

protected:

private:

    CMaxExclusive() : CXSDNode::CXSDNode(NULL, XSD_MAX_EXCLUSIVE), m_nMaxExclusive(-1)
    {
    }
};

#endif // _SCHEMA_MAX_EXCLUSIVE_HPP_
