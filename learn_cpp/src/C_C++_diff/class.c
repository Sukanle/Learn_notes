#define namespace_Man
#include <stdio.h>
#include <string.h>

#ifdef namespace_Man
void print(char* name, int age);
char* getName(char* name);
int* getAge(int age);

typedef struct {
    void (*print)(char*, int);
    char* (*getName)(char*);
    int* (*getAge)(int);
} Manfn;
typedef struct {
    char name[20];
    int age;
    // void (*print)(char *,int);
    Manfn fn;
} Man;

void ManInit(Man* this) {
    // this->print = print;
    this->fn.print = print;
    this->fn.getName = getName;
    this->fn.getAge = getAge;
}
#endif

Man kb;
int main() {
    strcpy(kb.name, "KB");
    ManInit(&kb);
    kb.age = 20;
    // kb.print = print;
    // kb.print(kb.name,kb.age);
    kb.fn.print(kb.name, kb.age);
}

#ifdef namespace_Man
void print(char* name, int age) {
    printf("Name: %s\n", name);
    printf("Age: %d\n", age);
}

char* getName(char* name) {
    return name;
}

int* getAge(int age) {
    return &age;
}
#endif
