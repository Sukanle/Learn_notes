#工厂模式

#工厂方法
* 工厂方法实际就是将类的实例化单独封装为一个方法，这个方法就是工厂方法。
>在`C++`的`STL`中，大多数的容器都有一个对应的工厂方法，一般以`make_`开头，比如`std::make_pair`、`std::make_shared`、`std::make_unique`等等，可见工厂方法在实际开发中是非常常见的。
* 先看一个简单的场景：<br>
有一个工厂老板，他有很多生产线，比如`Truck`、`Car`、`Bike`等等，每个生产线都有自己的生产方法，但是作为老板的他并不关心生产的细节，即他只关心生产出的产品，而不关心生产的细节，这时候就可以使用工厂方法来实现，即老板告知工厂负责人需要什么产品，工厂负责人再去调用对应的生产线来生产产品。
```cpp
#include <iostream>
#include <memory>

enum ProductType{
  Truck,
  Car,
  Bike
};
class Factory {
public:
  Factory() = default;
  ~Factory() = default;
  auto create(ProductType type){
    switch(type){
      case Truck:
        return new Truck();
      case Car:
        return new Car();
      case Bike:
        return new Bike();
      default:
        return nullptr;
      }
  }
};

class Truck {
public:
  Truck(){produce();}
  ~Truck() = default;
  void produce(){
    std::cout << "Produce Truck" << std::endl;
  }
};

class Car {
public:
  Car(){produce();}
  ~Car() = default;
  void produce(){
    std::cout << "Produce Car" << std::endl;
  }
};

class Bike {
public:
  Bike(){produce();}
  ~Bike() = default;
  void produce(){
    std::cout << "Produce Bike" << std::endl;
  }
};
```
* 上面的代码中，`Factory`类是工厂负责人，`Truck`、`Car`、`Bike`是具体的生产线，`create`方法就是工厂方法，通过传入不同的产品类型，来调用对应的生产线来生产产品，虽然这个例子很简单，但是却很好的说明了工厂方法的作用。
* 但是上面的代码有一个问题，每次老板想要添加一个新的生产线，都需要修改`Factory`类，这样就违反了开闭原则，所以可以使用抽象工厂方法来解决这个问题。

#抽象工厂方法
