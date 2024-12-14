#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <fmt/format.h>

int main(){
    xmlDoc *doc=xmlNewDoc(BAD_CAST("1.0"));
    xmlNode *root=xmlNewNode(nullptr,BAD_CAST("root"));

    xmlDocSetRootElement(doc,root);

    xmlNewTextChild(root,nullptr,BAD_CAST("node1"),BAD_CAST("content of node1"));
    xmlNewTextChild(root,nullptr,BAD_CAST("node2"),BAD_CAST("content of node2"));
    xmlNewTextChild(root,nullptr,BAD_CAST("node3"),BAD_CAST("content of node3"));

    xmlNode *node=xmlNewNode(nullptr,BAD_CAST("node2"));
    xmlNode *content=xmlNewText(BAD_CAST("NODE CONTENT"));
    xmlAddChild(root,node);
    xmlAddChild(node,content);
    xmlNewProp(node,BAD_CAST("attribute"),BAD_CAST("yes"));

    node=xmlNewNode(nullptr,BAD_CAST("son"));
    xmlAddChild(root,node);
    xmlNode *grandson=xmlNewNode(nullptr,BAD_CAST("grandson"));
    xmlAddChild(node,grandson);
    xmlAddChild(grandson,xmlNewText(BAD_CAST("This is a grandson node")));

    // int nRel=xmlSaveFile("CreatedXML.xml",doc);
    int nRel=xmlSaveFormatFile("CreatedXML.xml",doc,1);
    if(nRel!=-1)
        fmt::print("一个xml文档被创建，写入{}个字节\n",nRel);
    xmlFreeDoc(doc);

    return 0;
}
