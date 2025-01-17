web_server
===============
Linux下C++轻量级Web服务器，参考自热门项目TinyWebServer

* 使用 **线程池 + 非阻塞socket + epoll(ET和LT均实现) + 事件处理(Reactor和模拟Proactor均实现)** 的并发模型
* 使用**状态机**解析HTTP请求报文，支持解析**GET和POST**请求
* 访问服务器数据库实现web端用户**注册、登录**功能，可以请求服务器**图片和视频文件**
* 实现**同步/异步日志系统**，记录服务器运行状态
* 实现**时间堆/时间轮**定时器,默认使用时间堆
* 经Webbench压力测试可以实现**上万的并发连接**数据交换

理论参考
-------------
《linux高性能网络编程》 游双著

压力测试
-------------
```cpp
cd /test_pressure/webbench-1.5

make clean

make & make install

webbench -c 10000 -t 5 http:127.0.0.1:9006/
```
报错可参考处理：https://blog.csdn.net/qq_44857215/article/details/128788582


测试结果
-------------
在关闭日志后，使用Webbench对服务器进行压力测试，对listenfd和connfd分别采用ET和LT模式，默认使用时间堆

> * 时间堆，Proactor，LT + LT，29997 QPS
![alt text](root/result-1.png)

> * 时间堆，Proactor，LT + ET，31206 QPS
![alt text](root/result-2.png)

> * 时间堆，Proactor，ET + LT，26315 QPS
![alt text](root/result-3.png)

> * 时间堆，Proactor，ET + ET，29758 QPS
![alt text](root/result-4.png)

> * 时间轮，Proactor，LT + LT，28318 QPS
![alt text](root/result-5.png)

> * 时间轮，Proactor，LT + ET，29289 QPS
![alt text](root/result-6.png)

> * 时间轮，Proactor，ET + LT，27116 QPS
![alt text](root/result-7.png)

> * 时间轮，Proactor，ET + ET，29792 QPS
![alt text](root/result-8.png)


> * 并发连接总数：10000
> * 访问服务器时间：5s
> * 所有访问均成功

快速运行
------------
* 服务器测试环境
	* WSL2子系统中的Ubuntu版本22.04
	* MySQL版本8.0.37
    * CPU：11th Gen Intel(R) Core(TM) i7-11800 @ 2.30GHz
* 浏览器测试环境
	* Windows、Linux均可
	* Chrome/edge

* 测试前确认已安装MySQL数据库

    ```C++
    // 建立yourdb库
    create database yourdb;

    // 创建user表
    USE yourdb;
    CREATE TABLE user(
        username char(50) NULL,
        passwd char(50) NULL
    )ENGINE=InnoDB;

    // 添加数据
    INSERT INTO user(username, passwd) VALUES('name', 'passwd');
    ```

* 修改main.cpp中的数据库初始化信息

    ```C++
    //数据库登录名,密码,库名
    string user = "root";
    string passwd = "111111";
    string databasename = "yourdb";
    ```

* build

    ```C++
    sh ./build.sh
    ```

* 启动server

    ```C++
    ./server
    ```

* 浏览器端

    ```C++
    ip:9006
    ```

个性化运行
------

```C++
./server [-p port] [-l LOGWrite] [-m TRIGMode] [-o OPT_LINGER] [-s sql_num] [-t thread_num] [-c close_log] [-a actor_model]
```

温馨提示:以上参数不是非必须，不用全部使用，根据个人情况搭配选用即可.

* -p，自定义端口号
	* 默认9006
* -l，选择日志写入方式，默认同步写入
	* 0，同步写入
	* 1，异步写入
* -m，listenfd和connfd的模式组合，默认使用LT + LT
	* 0，表示使用LT + LT
	* 1，表示使用LT + ET
    * 2，表示使用ET + LT
    * 3，表示使用ET + ET
* -o，优雅关闭连接，默认不使用
	* 0，不使用
	* 1，使用
* -s，数据库连接数量
	* 默认为8
* -t，线程数量
	* 默认为8
* -c，关闭日志，默认打开
	* 0，打开日志
	* 1，关闭日志
* -a，选择反应堆模型，默认Proactor
	* 0，Proactor模型
	* 1，Reactor模型

测试示例命令与含义

```C++
./server -p 9007 -l 1 -m 0 -o 1 -s 10 -t 10 -c 1 -a 1
```

- [x] 端口9007
- [x] 异步写入日志
- [x] 使用LT + LT组合
- [x] 使用优雅关闭连接
- [x] 数据库连接池内有10条连接
- [x] 线程池内有10条线程
- [x] 关闭日志
- [x] Reactor反应堆模型