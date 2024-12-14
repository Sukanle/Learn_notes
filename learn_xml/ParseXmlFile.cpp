#include <fmt/color.h>
#include <fmt/format.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#define PRINT_ERROR(msg)                                                                           \
    fmt::print(stderr, fg(fmt::color::red), "Error: ");                                            \
    fmt::print(stderr, msg)

int main(int argc, char* argv[])
{
    xmlDoc* doc = nullptr;
    xmlNode* curNode;
    xmlChar* szKey;

    if (argc <= 1) {
        fmt::print("Usage: {} docname\n", argv[0]);
        return -1;
    }

    char* szDocName = argv[1];
    doc = xmlReadFile(szDocName, "UTF-8", XML_PARSE_RECOVER);
    if (nullptr == doc) {
        PRINT_ERROR("Document not parsed successfully.\n");
        return -1;
    }

    curNode = xmlDocGetRootElement(doc);

    if (nullptr == curNode) {
        PRINT_ERROR("empty document\n");
        xmlFreeDoc(doc);
        return -1;
    }

    if (xmlStrcmp(curNode->name, BAD_CAST("root"))) {
        PRINT_ERROR("document of the wrong type, root node != root\n");
        xmlFreeDoc(doc);
        return -1;
    }

    curNode = curNode->xmlChildrenNode;
    xmlNode* propNodeptr = curNode;
    while (curNode != nullptr) {
        if (!xmlStrcmp(curNode->name, (const xmlChar*)("node1"))) {
            szKey = xmlNodeGetContent(curNode);
            fmt::print("newNode1: {:s}\n", szKey);
            xmlFree(szKey);
        }
        if (xmlHasProp(curNode, BAD_CAST("attribute")))
            propNodeptr = curNode;
        curNode = curNode->next;
    }

    xmlAttr* attrptr = propNodeptr->properties;
    while (attrptr != nullptr) {
        if (!xmlStrcmp(attrptr->name, BAD_CAST("attribute"))) {
            xmlChar* szAttr = xmlGetProp(propNodeptr, BAD_CAST("attribute"));
            fmt::print("attribute: {:s}\n", szAttr);
            xmlFree(szAttr);
        }
        attrptr = attrptr->next;
    }

    xmlFreeDoc(doc);

    return 0;
}
