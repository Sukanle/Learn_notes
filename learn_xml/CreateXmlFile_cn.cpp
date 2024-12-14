#include <libxml/parser.h>
#include <libxml/tree.h>

#include "wxn_codeConv.h"

int main()
{
	//定义文档和节点指针
	xmlDocPtr doc = xmlNewDoc(BAD_CAST"1.0");
	xmlNodePtr root_node = xmlNewNode(NULL,BAD_CAST"root");

	//设置根节点
	xmlDocSetRootElement(doc,root_node);

	//一个中文字符串转换为UTF-8字符串，然后写入
	char* szOut = GD2312_to_UTF8("节点1的内容");

	//在根节点中直接创建节点
	xmlNewTextChild(root_node, NULL, BAD_CAST "newNode1", BAD_CAST "newNode1 content");
	xmlNewTextChild(root_node, NULL, BAD_CAST "newNode2", BAD_CAST "newNode2 content");
	xmlNewTextChild(root_node, NULL, BAD_CAST "newNode3", BAD_CAST "newNode3 content");

	xmlNewChild(root_node, NULL, BAD_CAST "node1",BAD_CAST szOut);
	free(szOut);

	//创建一个节点，设置其内容和属性，然后加入根结点
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"node2");
	xmlNodePtr content = xmlNewText(BAD_CAST"NODE CONTENT");
	xmlAddChild(root_node,node);
	xmlAddChild(node,content);
	szOut = GD2312_to_UTF8("属性值");
	xmlNewProp(node,BAD_CAST"attribute",BAD_CAST szOut);
	free(szOut);

	//创建一个中文节点
	szOut = GD2312_to_UTF8("中文节点");
	xmlNewChild(root_node, NULL, BAD_CAST szOut,BAD_CAST "content of chinese node");
	free(szOut);

	//存储xml文档
	int nRel = xmlSaveFormatFileEnc("CreatedXml_cn.xml",doc,"GB2312",1);
	if (nRel != -1)
        fmt::print("一个xml文档被创建,写入{:d}个字节\n",nRel);

	xmlFreeDoc(doc);

	return 0;
}
