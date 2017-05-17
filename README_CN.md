* 简介
  terminal是一个支持memcache协议的key-value数据库, 可以在线挂载多个静态数据存储, 很适合用于推荐引擎存储离线计算的候选集及非实时更新的大批量数据。 它不支持写操作, 只能用于进行静态数据存储, 但是它可以批量的动态载入线下生成的数据文件，这样可以减少更新大批量数据写入对系统造成的影响, 最大程度的降低写放大对磁盘的损耗。
  terminal结合使用内存和硬盘, 内存只用来存储索引文件, 硬盘则用来存储真正的数据文件. 很大程度上减少对机器内存的消耗。

  terminal的每个库由数据文件和索引文件组成, 数据文件命名为dat, 索引文件命名为idx,  索引文件由一个个key-pos对组成. 其中key就是key-value结构中需要查询的key(1.0版本现在只支持整数类型的key, 后续会支持字符串类型的key), 而pos则包含两部分信息, 它的前40位表示value在dat文件中的偏离值off, 后20位表示value的长度length, 通过off和length来共同定位dat文件中的value

* 使用方法
** 源码编译
*** 依赖的第三方源码包
    - libevent1.4
*** 开始编译
    1. git clone https://github.com/desion/terminal.git
    2. 编译
       #+BEGIN_EXAMPLE
       cd terminal
       make
       #+END_EXAMPLE
** 使用
   1. 启动
      #+BEGIN_SRC sh
      -p <num>   TCP port to listen on (default: 9898)
      -l <ip_addr>  interface to listen on, default is INDRR_ANY
      -c <num>      max simultaneous connections (default: 1024)
      -d            daemon
      -h            help
      -v            verbose (print errors/warnings)
      -t <num>      number of worker threads to use, default 4
      -T <num>      timeout in millisecond, 0 for none, default 0

      ./terminal -p 9898 -c 1024 -d -t 8
      #+END_SRC
   2. 挂载数据
      挂载数据只需要将相应的idx和dat文件放在一个目录，然后执行下面的命令通知服务加载

      #+BEGIN_SRC sh
      echo -ne "open <data_dir>\r\n" | nc 127.0.0.1 9898
      #+END_SRC
      
      或者可以通过append命令向相应的库追加新的数据，如果同以往的数据有重叠，那么新版本的数据会替换旧版本的数据
      #+BEGIN_SRC sh
      echo -ne "append <data_dir>\r\n" | nc 127.0.0.1 9898
      #+END_SRC

   3. 卸载数据
      #+BEGIN_SRC sh
      echo -ne "close <label>\r\n" | nc 127.0.0.1 9898
      #+END_SRC
   4. 卸载制定版本的数据
      #+BEGIN_SRC sh
      echo -ne "close <label> <version>\r\n" | nc 127.0.0.1 9898
      #+END_SRC

** 访问示例
   启动后即可通过stats命令查看服务状态
   #+BEGIN_EXAMPLE
   echo -ne "stats\r\n" | nc 127.0.0.1 9898
   #+END_EXAMPLE
   查询某个key的value(get label-key)
   #+BEGIN_EXAMPLE
   echo -ne "get 1-123456\r\n" | nc 127.0.0.1 9898
   #+END_EXAMPLE
   可以通过info命令查看当前挂载库的状态
   #+BEGIN_EXAMPLE
   echo -ne "info\r\n" | nc 127.0.0.1 9898
   #+END_EXAMPLE
** 生成数据
    #+BEGIN_EXAMPLE
    1. 有一个原始的数据文件dat, 每行都是key-value结构, 用:分隔, key必须为整数
    2. 如果数据文件的key是无序的, 可以先将数据文件中的key按数字进行排序
    3. 使用编译生成的index_create程序生成索引文件
    ./index_create -i <dat> -o <idx> -l <label>
    <dat> 为要生成索引的数据文件，idx是生成索引文件的路径，label是库的标识
    #+END_EXAMPLE
* 支持命令
  - info

    查看库是否挂载成功, 显示每个库的信息, 打开时间, 当前处理的请求量, 库里面有多少条记录

  - stats

    查看lushan本身的状态, 主要是通信部分的信息(例如: 当前等待处理队列里有多少请求, 有多少请求在等待队列里超时了). 这些信息, 有利于知道服务是否稳定, 是否性能满足要求

  - get

    取得一个或多个key的value

  - open reopen

    动态挂载库
  
  - append

    在已有库的情况下追加新的数据

  - stats reset(慎用)

    重置统计信息

  - close

    关闭客户端连接
