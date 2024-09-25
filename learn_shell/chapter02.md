# Shell变量

## *定义变量*
* 跟绝大数编程语言一样，`shell`编程中，`变量`主要用于储存数据值，如：
```shell
shell_name="bash"
```
* 注意，不同与ss他常用的编程语言（如`C/C++`），`shell`的变量名与等号之间不允许存在空格，并且还有一些其他的规则：
    * **只能包含字母、数字和下划线：** 变量名可以包含字母（大小敏感，如同`C/C++`）、数字和下划线`_`，不能包含其他特殊字符；
    * **不能以数字开头：** 变量名不能以数字开头、但可以包含数字；
    * **避免使用shell关键字** 如同其他编程语言一样，不要使用`Shell关键字`（例如`if`、`then`、`else`、`fi`、`for`、`while`等）；
    * **避免使用特殊符号** 尽量避免在变量名中使用特殊符号，因为可能无法被识别或与`shell`自身的语法冲突；
    * **避免使用控股** 不同与其他语言，`shell`语法中，空格常用于分隔命令和参数；
    * **使用大写字母表示常量** 相当于`C/C++`中使用宏来定义常量一样，`shell`使用全大写来表示常量只是习惯上的表示方法，并没有语法这样规定；
    * **以下是shell变量名的演示：**
         ```shell
        #正确的shell变量命名
        PI="3.14"
        BASH_PATH="/bin/bash"
        _var="123"
        var0="123"

        #错误的shell变量命名
        #避免使用关键字作为变量名
        if="what"
        #避免使用$等特殊符号
        var$=42
        ?var=24
        #避免变量名使用空格
        var man="value"
        #避免等号两边出现空格
        var_ = "val"
        ```

## *使用变量*
* 使用一个已定义过的变量，只要在变量名前加上`$`符号即可，例如：
    ```
    bash_path="/bin/bash"
    echo $bash_path
    echo ${bash_path}
    ```
    >变量名外的花括号是可选的，加花括号是为了帮助解释器识别变量的边界，比如下面这种情况：
    ```
    for skill in Ada Coffe Action Java; do
    echo "I am good at ${skill}Script"
    done
    ```
    >如果不给`skill`变量加花括号，写成`echo "I am good at $skillScript"`，解释器就会把`$skillScript`当成一个变量（其值为空），代码执行结果就不是我们期望的样子了。
* 推荐给所有变量加上花括号，这是个好的编程习惯。

* 已定义的变量，可以被重新定义，如：
    ```
    your_name="tom"
    echo $your_name
    your_name="alibaba"
    echo $your_name
    ```
    >这样写是合法的，但注意，第二次赋值的时候不能写`$your_name="alibaba"`，使用变量的时候才加美元符`$`。

## *只读变量*
* 使用`readonly`命令可以将变量定义为只读变量，只读变量的值不能被改变。
* 下面的例子尝试更改只读变量，结果报错：
     ```
    #!/bin/bash

    myUrl="https://www.google.com"
    readonly myUrl
    myUrl="https://www.runoob.com"
    ```
    >运行该脚本后，结果如下：  
    >/bin/sh: NAME: This variable is read only.

## *删除变量*
* 使用`unset`命令可以删除变量。语法:
    ```
    unset variable_name
    ```
    >变量被删除后不能再次使用。unset 命令不能删除只读变量。
    >```
    >#!/bin/sh
    >
    >myUrl="https://www.runoob.com"
    >unset myUrl
    >echo $myUrl
    >```
    >以上实例执行将没有任何输出。

## *变量类型*
`Shell`支持多种不同类型的变量，其中一些主要的类型包括：
1. **字符串变量：** 在 `Shell`中，变量通常被视为字符串。  
    >你可以使用单引号 ' 或双引号 " 来定义字符串，例如：
    >```
    >my_string='Hello, World!'
    >或者
    >my_string="Hello, World!"
    >```
2. **整数变量：** 在一些`Shell`中，你可以使用 `declare` 或 `typeset` 命令来声明整数变量。
    >这样的变量只包含整数值，例如：
    >```
    >declare -i my_integer=42
    >```
3. **数组变量：** `Shell` 也支持数组，允许你在一个变量中存储多个值。
    >数组可以是整数索引数组或关联数组，以下是一个简单的整数索引数组的例子：
    >```
    >my_array=(1 2 3 4 5)
    >```
    >或者关联数组：
    >```
    >declare -A my_array
    >my_array["color"]=blue
    >my_array["office number"]=42
    >```
4. 环境变量：`Shell` 也包含了一些环境变量，这些变量由`Shell`创建并维护，包含了一些有用的信息。  
    >例如，`$HOME` 变量包含了当前用户的家目录的路径，`$PWD` 变量包含了当前工作目录的路径。
    >```
    >echo $HOME
    >echo $PWD
    >```
5. 特殊变量：`Shell` 也包含了一些特殊变量，这些变量由`Shell`创建并维护，包含了一些有用的信息。  
    >例如，`$0` 变量包含了当前脚本的文件名，`$#` 变量包含了传递给脚本的参数的个数。
    >```
    >echo $0
    >echo $#
    >```

> 你现在应该对`Shell`中的变量有了一个基本的了解，接下来将着重讲解一下`Shell`字符串。[第三章](chapter03.md)
