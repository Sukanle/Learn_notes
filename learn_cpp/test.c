#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MAX_CONTACTS 100
#define MAX_NAME_LENGTH 50
#define MAX_WORKPLACE_LENGTH 50
#define MAX_PHONE_LENGTH 20
#define MAX_EMAIL_LENGTH 50


typedef struct
{
char name[MAX_NAME_LENGTH];
char workplace[MAX_WORKPLACE_LENGTH];
char phone[MAX_PHONE_LENGTH];
char email[MAX_EMAIL_LENGTH];
}contact;

contact contacts[MAX_CONTACTS];
int numContacts=0;

//添加联系人
void addContact()
{
	if(numContacts<MAX_CONTACTS)
	{
		printf("请输入联系人姓名：");
		scanf("%s",contacts[numContacts].name);
		printf("请输入联系人工作单位：");
		scanf("%s",contacts[numContacts].workplace);
		printf("请输入联系人电话：");
		scanf("%s",contacts[numContacts].phone);
		printf("请输入联系人电子邮件：");
		scanf("%s",contacts[numContacts].email);
		numContacts++;
        return;
	}
	printf("通讯录已满，无法添加更多联系人！\n");
}

//按姓名查找联系人
void searchbyname()
{
	char name[MAX_NAME_LENGTH];
	printf("请输入要查找的联系人姓名：");
	scanf("%s",name);
	for(int i=0;i<numContacts;i++)
		if(strcmp(contacts[i].name,name)==0)
		{
			printf("姓名：%s\n",contacts[i].name);
			printf("工作单位：%s\n",contacts[i].workplace);
			printf("电话：%s\n",contacts[i].phone);
			printf("电子邮箱：%s\n",contacts[i].email);
            return;
		}
    printf("未找到该联系人!\n");
}
//按电话号码查找联系人
void searchbyphone()
{
	char phone[MAX_PHONE_LENGTH];
	printf("请输入要查找的联系人电话号码：");
	scanf("%s",phone);
	for(int i=0;i<numContacts;i++)
		if(strcmp(contacts[i].phone,phone)==0)
		{
			printf("姓名：%s\n",contacts[i].name);
			printf("工作单位：%s\n",contacts[i].workplace);
			printf("电话：%s\n",contacts[i].phone);
			printf("电子邮箱：%s\n",contacts[i].email);
            return;
		}
    printf("未找到该联系人!\n");
}

//修改联系人信息
void modifyContact()
{
	char name[MAX_NAME_LENGTH];
	printf("请输入要修改的联系人姓名：");
	scanf("%s",name);
	for(int i=0;i<numContacts;i++)
		if(strcmp(contacts[i].name,name)==0)
		{
			printf("请输入新的工作单位：");
			scanf("%s",contacts[i].workplace);
			printf("请输入新的电话：");
			scanf("%s",contacts[i].phone);
		    printf("请输入新的电子邮箱：");
			scanf("%s",contacts[i].email);
			printf("修改成功！\n");
            return;
		}
    printf("未找到该联系人！\n");
}

//删除联系人
void deleteContact()
{
	char name[MAX_NAME_LENGTH];
	printf("请输入要删除的联系人姓名：");
	scanf("%s",name);
	for(int i=0;i<numContacts;i++)
		if (strcmp(contacts[i].name,name)==0)
		{
			for(;i<numContacts-1;i++)
			{
				strcpy(contacts[i].name,contacts[i+1].name);
				strcpy(contacts[i].workplace,contacts[i+1].workplace);
				strcpy(contacts[i].phone,contacts[i+1].phone);
				strcpy(contacts[i].email,contacts[i+1].email);
			}
            numContacts--;
            printf("联系人已删除！\n");
            return;
		}
    printf("未找到该联系人！\n");
}

//显示所有联系人
void displayContacts()
{
	for(int i=0;i<numContacts;i++)
	{
		printf("姓名：%s\n",contacts[i].name);
		printf("工作单位：%s\n",contacts[i].workplace);
		printf("电话：%s\n",contacts[i].phone);
		printf("电子邮箱：%s\n",contacts[i].email);
		printf("\n");
	}
}

void CLEAR_PAUSE()
{
#ifdef _WIN32
    system("pause");
    system("cls");
#else
    system("read -n 1 -p 'Press any key to continue...'");
    system("clear");
#endif
}

int main()
{
	int choice;

	do
	{
        printf("\n1.添加联系人\n");
        printf("2.按姓名查找联系人\n");
        printf("3.按电话号码查找联系人\n");
        printf("4.修改联系人信息\n");
        printf("5.删除联系人\n");
        printf("6.显示所有联系人\n");
        printf("7.退出\n");
        printf("请选择：");
        scanf("%d",&choice);


        switch(choice)
        {
            case 1:
                addContact();
                CLEAR_PAUSE();
                break;
            case 2:
                searchbyname();
                CLEAR_PAUSE();
                break;
            case 3:
                searchbyphone();
                CLEAR_PAUSE();
                break;
            case 4:
                modifyContact();
                CLEAR_PAUSE();
                break;
            case 5:
                deleteContact();
                CLEAR_PAUSE();
                break;
            case 6:
                displayContacts();
                CLEAR_PAUSE();
                break;
            case 7:
                printf("退出程序！\n");
                CLEAR_PAUSE();
                break;
            default:
                printf("无效的选择，请重新输入！\n");
                CLEAR_PAUSE();
        }
    }while(choice!=7);

    return 0;
}
