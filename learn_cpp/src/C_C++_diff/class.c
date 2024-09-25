#include <stdio.h>
#include <string.h>
void print(char *name,int age);

typedef struct {
	char name[20];
	int age;
	void (*print)(char *,int);
}Man;

Man kb;
int main()
{
	strcpy(kb.name,"KB");
	kb.age = 20;
	kb.print = print;
	kb.print(kb.name,kb.age);
}

void print(char *name, int age)
{
	printf("Name: %s\n", name);
	printf("Age: %d\n", age);
}

