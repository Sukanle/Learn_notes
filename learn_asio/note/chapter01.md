# `Asio`基本流程

## 基础知识
网络编程的基本流程如下：
* `server`:
    1. `socket` --> 创建`socket`对象
    2. `bind`   --> 绑定本机`ip`+`port`
    3. `listen` --> 监听`port`，是否有`connect`请求
    4. `accpet` --> 在创建一个`socket`对象给其收发消息。原因是现实中`server`是面对多个`client`，那么为了区分各个`client`，则每个`server`都需再分配一个`socket`对象进行收发消息。
    5. `read`、`write` ——> 就是收发消息了。
* `client`:
    1. `socket`  --> 创建`socket`对象
    2. `connect` --> 根据`client`的`ip`+`port`，发起`connect`请求
    5. `write`、`read` ——> 建立`connect`后，就可以发收消息了
* 图示如下：
    ![clinet_and_server](./png/1540562-20190417002428451-62583604.jpg "clinet_and_server")

## Asio基本流程：
正如`Unix`哲学一样，`Asio`里面也是使用文件来进行网络连接的，因此，程序中想使用`Asio`，至少需要一个`io_context`[^1]

[^1]:早期的`Asio`版本中，`io_context`又名为`io_service`,仅只是将名称更改以便于更贴近功能语义，使用方法还是一样的。
