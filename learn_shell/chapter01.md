# 第一章：认识shell脚本

## Shell脚本简介

`shell`脚本可以被脚本语言解释器执行，也可以被`shell`解释器执行。

## shell脚本解释器

1. *指定`shell`脚本解释器*
2. *在`Linux`系统中，`bash`是默认的`shell`解释器。在`shell`脚本的第一行，可以使用`#!`来指定解释器。例如，指定`bash`解释器如下所示：*
```sh
#!/bin/bash
```
* 当然也可以使用其他脚本解释器来执行`shell`脚本，例如，指定`python`解释器如下所示：
```sh
#!/usr/bin/python3
```

1. *查看系统默认shell解释器*
你也可以尝试在终端使用`echo` `$SHELL`命令来查看当前系统的默认`shell`解释器。
在`Linux`系统中，一般默认的`shell`解释器是`bash`，通常在`/bin/bash`目录下。

> **注意：在`Linux`系统中，`bash`是默认的`shell`解释器，但不同的`Linux`系统可能会有不同的默认`shell`解释器。**

## shell脚本文件

1. *`shell`脚本文件的创建*
`shell`脚本的文件扩展名通常是`.sh`，实际上对于脚本的后缀名并没有严格规定，但为了便于其他程序员理解，通常规定`.sh`后缀名为`shell`脚本文件；
例如，一个名为`test.sh`的`shell`脚本文件的内容如下所示：
```sh
#!/bin/bash // 指定bash解释器
... // shell脚本的代码
```
2. *`shell`脚本文件的执行*
    1. 编写好`shell`脚本文件后，该如何执行呢？在终端中，可以使用以下命令来执行`shell`脚本文件：
    ```sh
    bash test.sh
    <shell解释器路径> <shell脚本文件路径>
    ```
    2. 当然，也可以给`shell`脚本文件添加可执行权限，然后直接执行`shell`脚本文件，就像执行其他可执行文件一样。例如，给`test.sh`添加可执行权限并执行如下所示：
    ```sh
    chmod +x test.sh
    chmod +x <shell脚本文件路径> // 添加文件权限，使其可执行，可能需要root权限
    ./test.sh
    ./<shell脚本文件路径> // 直接执行shell脚本文件
    ```

> 恭喜,你现在成功认识了`shell`脚本  
> 若您使用的是`vim`文本编辑器，可以将光标定位到后面文件名的任意位置上，直接敲键盘 `gf` 进入[第二章](chapter02.md)。
