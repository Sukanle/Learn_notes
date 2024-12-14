#include <libxml/parser.h>
#include <libxml/tree.h>
#include <fmt/format.h>
#include <fmt/color.h>

#define PRINT_ERROR(meg)\
    fmt::print(stderr,fg(fmt::color::red),"Error: ");\
    fmt::print(stderr,meg)

int main(int argc,char *argv[]){
    if(argc<2){
        fmt::print("Usage: {:s} docname\n",argv[0]);
        return 0;
    }

    char *docname=argv[1];

    xmlDoc *doc=xmlReadFile(docname,"UTF-8",XML_PARSE_RECOVER);

    if(doc==nullptr){
        PRINT_ERROR("Document not parsed successfully.\n");
        return -1;
    }
    xmlNode *curNode=xmlDocGetRootElement(doc);

    if(curNode==nullptr){
        PRINT_ERROR("empty document\n");
        xmlFreeDoc(doc);
        return -1;
    }

    curNode = curNode->xmlChildrenNode;
    while(curNode!=nullptr){
        if(!xmlStrcmp(curNode->name,BAD_CAST("node1"))){
            xmlNode *tempNode=curNode->next;
            xmlUnlinkNode(curNode);
            xmlFreeNode(curNode);
            curNode=tempNode;
            continue;
        }

        if(!xmlStrcmp(curNode->name,BAD_CAST("node2")))
            xmlSetProp(curNode,BAD_CAST("attribute"),BAD_CAST("no"));

        if(!xmlStrcmp(curNode->name,BAD_CAST("node3")))
            xmlNodeSetContent(curNode,BAD_CAST("content changed"));

        if(!xmlStrcmp(curNode->name,BAD_CAST("node4")))
            xmlSetProp(curNode,BAD_CAST("newAttr"),BAD_CAST("YES"));

        if(!xmlStrcmp(curNode->name,BAD_CAST("son")))
            xmlNewTextChild(curNode,nullptr,BAD_CAST("newGrandson"),BAD_CAST("newGrandson content"));

        curNode=curNode->next;
    }

    int nRel=xmlSaveFile("ChangedXml.xml",doc);
    if(nRel!=-1)
        fmt::print("一个xml文件被创建，其中包含了{:d}个节点\n",nRel);

    xmlFreeDoc(doc);

    return 0;
}
