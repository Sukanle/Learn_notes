# `Makefile`基础

## 1.1 `Makefile`的概念
Makefile是一种构建工具，它可以自动化执行任务，比如编译源代码、打包、运行测试等。Makefile是一种文本文件，其中包含一系列规则，每个规则定义了一个任务的依赖关系和执行命令。

## 1.2 `Makefile`的基本语法
* 一个规则由三部分组成：目标、依赖和命令。<br>
  例如，要生成`KB.txt`文件，它依赖于`Out.txt`和`Man.txt`两个目标,规则如下：
  ```make
  KB.txt: Out.txt Man.txt
    cat Out.txt Man.txt > KB.txt
  ```
  > 注意：<br>
  > **命令前必须是一个Tab键**，不能是空格。<br>
  > #号后面的内容是注释，`Makefile`中的注释是以#号开头的。
  实际上，`Makefile`中的命令是`shell`命令，所以你可以在`Makefile`中使用`shell`命令，比如`cat`、`cp`、`mv`等，而编译指令`gcc`、`javac`等也可以在`Makefile`中使用，只要是`shell`命令都可以。<br>
  类似的，我们写出生成`牢大.txt`文件的规则：
  ```make
  牢大.txt: KB.txt 坠机.txt
    cat KB.txt 坠机.txt > 牢大.txt
  ```
  由于`make`执行时，会从第一条规则，所有我们把规则`牢大.txt`放到前面。完整的`Makefile`如下：
  ```make
  牢大.txt: KB.txt 坠机.txt
    cat KB.txt 坠机.txt > 牢大.txt
  KB.txt: Out.txt Man.txt
    cat Out.txt Man.txt > KB.txt
  ```
  在当前目录创建`Man.txt`、`Out.txt`和`坠机.txt`，输入一些内容，执行`make`：
  ```sh
  $ make
  cat Out.txt Man.txt > KB.txt
  cat KB.txt 坠机.txt > 牢大.txt
  ```
  `make`默认执行第一个规则，也就是创建`牢大.txt`文件,但是由于其依赖的`KB.txt`文件不存在，所以会先执行第二个规则，生成`KB.txt`文件，然后再执行第一个规则，生成`牢大.txt`文件。
  > 即如果你修改了依赖文件但是让该依赖文件的修改时间比目标文件的修改时间早，那么`make`不会执行规则中的命令。
* 通配符<br>
  `Makefile`支持通配符，比如`*`、`?`、`[]`等，可以用来匹配文件名。例如，要生成所有的`.txt`文件，可以使用通配符：
  ```make
  all: *.txt
  ```
  `make`会自动匹配当前目录下的所有`.txt`文件，执行规则中的命令。
  > 通配符：`*`表示匹配任意个字符，`?`表示匹配一个字符，`[]`表示匹配括号中的任意一个字符。
* 伪目标<br>
  有时候我们需要定义一些不是真正的文件的目标，比如`clean`目标，用于清理生成的文件。这种目标称为伪目标，可以在`Makefile`中定义：
  ```make
  clean:
    rm -f *.txt
  ```
  执行`make clean`命令，会删除当前目录下的所有`.txt`文件。
  但是如果当前目录下有一个名为`clean`的文件，那么`make clean`命令会执行该文件，而不是执行`clean`目标。为了避免这种情况，我们可以在`clean`目标前加上`.PHONY`伪目标：
  ```make
  .PHONY: clean
  clean:
    rm -f *.txt
  ```
* 控制打印信息<br>
  默认情况下，make会打印出它执行的每一条命令。如果我们不想打印某一条命令，可以在命令前加上`@`,表示不打印命令：
  ```make
  @echo "Hello, Makefile"
  echo "Hello, Makefile"
  ```
  执行结果如下：
  ```sh
  $ make
  Hello, Makefile
  echo "Hello, Makefile"
  Hello, Makefile
  ```
* 忽略错误<br>
  默认情况下，如果某条命令执行失败，`make`会停止执行后续命令。如果我们不想让`make`停止执行，可以在命令前加上`-`，表示忽略错误：
  ```make
  -rm -f *.txt
  ```
  如果当前目录下没有`.txt`文件，执行`make`命令不会报错。

  > 在下一章，我们将会通过一个简单的`C++`来初步的把Makefile文件编写出来，并成功地运行：[构建C/C++项目](chapter02.md)
  > [返回目录](README.md)
