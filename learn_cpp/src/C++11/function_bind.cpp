#include <iostream>
#include <functional>
#include <string>

// 普通函数
void show(int a,const std::string &str)
{
    std::cout <<"show: " << a << " " << str << std::endl;
}
void show0()
{
    std::cout <<"show0\n";
}

// 仿函数
class Func0
{
public:
    void operator()(){
        std::cout <<"FUnc0::operator()()\n";
    }
};

// 类静态函数
class Func1
{
public:
    static void show(int a)
    {
        std::cout <<"Func1::show: " << a << std::endl;
    }
};

// 类成员函数
class Func2
{
public:
    void show(int a)
    {
        std::cout <<"Func2::show: " << a << std::endl;
    }
};

// 类转换函数指针函数
class Func3
{
private:
    using func=void(*)();
    int a;
public:
    operator func()
    {
        return show0;
    }
};

template <typename Fn,typename ...Arg>
auto showT(Fn &&fn,Arg&&... args)
{
    std::cout<<"This is a template function.\n";

    auto f=std::bind(std::forward<Fn>(fn),std::forward<Arg>(args)...);
    f();

    std::cout<<"The template function is done.\n";

    return f;
}
int main()
{
    void(*f0)()=show0;f0();
    auto f1=[](int a){std::cout <<"lambda: " << a << std::endl;};f1(1);
    Func0 ff;Func0 *f2=&ff;(*f2)();
    void(*f3)(int)=Func1::show;f3(2);
    void(Func2::*f4)(int)=&Func2::show;Func2 FF; (FF.*f4)(3);
    Func3 f6;f6();

    printf("\n");

    std::function<void()> func0=show0;func0();
    std::function<void(int)> func1=[](int a){std::cout <<"lambda: " << a << std::endl;};func1(1);
    std::function<void()> func2=ff;func2();
    std::function<void(int)> func3=Func1::show;func3(2);
    std::function<void(Func2&,int)> func4=&Func2::show;func4(FF,3);

    printf("\n");

    std::function<void(int,const std::string &)>fn1=show;fn1(1,"hello");
    std::function<void(int,const std::string &)>fn2=std::bind(show,std::placeholders::_1,std::placeholders::_2);fn2(1,"hello");
    std::function<void(const std::string &,int)> fn3=std::bind(show,std::placeholders::_2,std::placeholders::_1);fn3("hello",1);
    std::function<void(int,const std::string &,bool)> fn4=std::bind(show,std::placeholders::_1,std::placeholders::_2);fn4(1,"hello",true);
    auto fn5=std::bind(&Func2::show, std::placeholders::_1,std::placeholders::_2);fn5(FF,3);

    printf("\n");

    // std::bind提前绑定参数，默认为值传递，如果需要引用传递，需要使用std::ref
    std::string str="hello";
    printf("%s\n",str.c_str());
    std::function<void(int)> fn6=std::bind(show,std::placeholders::_1,str);
    std::function<void(int)> fn6_quote=std::bind(show, std::placeholders::_1,std::ref(str));
    fn6(1);fn6_quote(1);
    printf("\n");
    str="hello World";
    printf("%s\n",str.c_str());
    fn6(1);fn6_quote(1);


    // 模板函数showT,支持绑定任意函数
    showT(show,1,str);
    printf("\n");
    auto f=showT(&Func2::show,&FF,3);

    f();
    return 0;
}
