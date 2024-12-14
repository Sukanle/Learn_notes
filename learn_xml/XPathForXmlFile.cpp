#include <fmt/color.h>
#include <fmt/format.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#define WARING_FMT(msg)                                                                            \
    fmt::print(fg(fmt::color::yellow), "Warning: ");                                               \
    fmt::print(msg)
#define ERROR_FMT(msg)                                                                             \
    fmt::print(fg(fmt::color::red), "Error: ");                                                    \
    fmt::print(msg)

xmlXPathObject* get_nodeset(xmlDoc* doc, const xmlChar* szXpath);

int main(int argc, char* argv[])
{
    if (argc <= 1) {
        fmt::print("Usage: {} docname\n", argv[0]);
        return -1;
    }

    char* szDocName = argv[1];
    xmlDoc* doc = xmlReadFile(szDocName, "UTF-8", XML_PARSE_RECOVER);

    if (doc == nullptr) {
        ERROR_FMT("Document not parsed successfully.\n");
        return -1;
    }

    xmlChar* szXpath = BAD_CAST("/root/node2[@attribute='yes']");
    xmlXPathObject* app_result = get_nodeset(doc, szXpath);

    if (app_result == nullptr) {
        ERROR_FMT("app_result is null.\n");
        return -1;
    }

    xmlChar* szValue = nullptr;
    xmlNode* curNode = nullptr;
    if (app_result) {
        xmlNodeSet* nodeset = app_result->nodesetval;
        for (int i = 0; i < nodeset->nodeNr; i++) {
            curNode = nodeset->nodeTab[i];
            if (curNode != nullptr) {
                szValue = xmlGetProp(curNode, BAD_CAST("attribute"));
                if (szValue != nullptr) {
                    fmt::print("attribute = {:s}\n", szValue);
                    xmlFree(szValue);
                }

                szValue = xmlNodeGetContent(curNode);
                if (szValue != nullptr) {
                    fmt::print("content = {:s}\n", szValue);
                    xmlFree(szValue);
                }
            }
        }
        xmlXPathFreeObject(app_result);
    }
    xmlFreeDoc(doc);
    return 0;
}

xmlXPathObject* get_nodeset(xmlDoc* doc, const xmlChar* szXpath)
{
    xmlXPathContext* context;
    xmlXPathObject* result;

    context = xmlXPathNewContext(doc);
    if (context == nullptr) {
        ERROR_FMT("context is null.\n");
        return nullptr;
    }

    result = xmlXPathEvalExpression(szXpath, context);
    xmlXPathFreeContext(context);
    if (result == nullptr) {
        ERROR_FMT("xmlXPathEvalExpression return nullptr\n");
        return nullptr;
    }

    if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        xmlXPathFreeObject(result);
        ERROR_FMT("nodeset is empty\n");
        return nullptr;
    }

    return result;
}
