&#160;&#160; 实现myshell项目总结报告
***

1、参考资料:  

- 《鸟哥的私房菜》
- linux c编程实战

2、我实现的功能:   
- 支持单个命令,如ls等。
- 支持输入输出重定向，即"<"和">"
- 支持管道
- 支持shell的内建命令cd
- Ctrl + c不能中断此shell
-可以查询历史命令，即history
- 界面美观

3、设计思路:
- 主函数,使用while循环然后调用其他函数
- 首先是打印提示符的函数，主要包括用户名，主机名，当前目录，提示符"$"。
- 然后是获取用户输入的函数，并将其保存到buf里，这里要注意遇到空格和换行符的情况。
- 接下来就是解析buf中的命令的,并将其保存到arglist中，如"ls -al /etc",则ls保存到arglist[0],-al保存到arglist[1],/etc保存到arglist[2]。
- 最后就是执行命令，在这里要对应命令分别在当前目录，/bin和/usr/bin目录查找要执行的程序。

4、实现过程中遇到的问题:     
- 刚开始带管道符的运行总是无结果，要么就是不识别管道后面是我的命令
- 有时运行某个命令，shell就会退出.
  
解决方法，对于第二个,主要检查exit(0)，return等这些有关退出的调用，第一个，我是试探性的修改，对自己修改的地方做上标记，以便改回。

5、结果展示：  
- cd:
```
ltt@myshell-pc:~$ cd liuxiao
ltt@myshell-pc:~/liuxiao$ cd one
ltt@myshell-pc:~/liuxiao/one$ cd llltt
ltt@myshell-pc:~/liuxiao/one/llltt$ cd ..
ltt@myshell-pc:~/liuxiao/one$ cd ~ 
ltt@myshell-pc:~$ cd /
ltt@myshell-pc:/$ cd /bin
ltt@myshell-pc:/bin$ cd
ltt@myshell-pc:~$ 
```
- 单个命令:  
```
ltt@myshell-pc:~$ ls
a.c	 examples.desktop  jdk1.8.0_172  opt	   snap       Videos   vimrc.zip     公共的    视频  下载		  桌面
ACM	 extract	   keshe	 Pictures  Templates  vim      wget-log      刘甜      收藏  线程日常练习
beifen	 f怪异问题.c	   liuxiao	 Projects  TTMSO      vim_old  编程	     模板      图片  项目myshell及其文档
Desktop  git		   oncerun.c	 Public    TTMSyan    vimplus  第三周算法题  日常练习  文档  音乐
ltt@myshell-pc:~$ find a.c
a.c
```
- 输出重定向:   
```
ltt@myshell-pc:~$ ls -l / > a
ltt@myshell-pc:~$ cat a
总用量 1750096
drwxr-xr-x   2 root root       4096 8月   3 19:21 bin
drwxr-xr-x   4 root root       4096 8月   3 19:23 boot
drwxrwxr-x   2 root root       4096 5月  14 22:22 cdrom
drwx------   2 root root       4096 6月   8 23:48 Desktop
drwxr-xr-x  20 root root       4640 8月   6 13:58 dev
drwxr-xr-x 137 root root      12288 8月   6 14:29 etc
drwxr-xr-x   3 root root       4096 6月  19 18:50 home
lrwxrwxrwx   1 root root         33 7月  25 10:31 initrd.img -> boot/initrd.img-4.15.0-22-generic
lrwxrwxrwx   1 root root         33 7月  25 10:31 initrd.img.old -> boot/initrd.img-4.15.0-29-generic
drwxr-xr-x  22 root root       4096 6月   9 00:36 lib
drwxr-xr-x   2 root root       4096 6月   9 00:37 lib32
drwxr-xr-x   2 root root       4096 4月  27 02:18 lib64
drwxr-xr-x   2 root root       4096 7月  25 11:17 libx32
drwx------   2 root root      16384 5月  14 22:19 lost+found
drwxr-xr-x   3 root root       4096 5月  14 22:29 media
drwxr-xr-x   4 root root       4096 6月   5 21:04 mnt
drwxr-xr-x   6 root root       4096 6月  19 17:58 opt
dr-xr-xr-x 359 root root          0 8月   6 13:57 proc
drwx------   7 root root       4096 7月  31 10:13 root
drwxr-xr-x  27 root root        820 8月   6 20:47 run
drwxr-xr-x   2 root root      12288 8月   3 19:22 sbin
drwxr-xr-x  10 root root       4096 8月   3 11:13 snap
drwxr-xr-x   3 root root       4096 6月   2 20:31 srv
-rw-------   1 root root 1791969280 5月  14 22:19 swapfile
dr-xr-xr-x  13 root root          0 8月   6 13:57 sys
drwxrwxrwt  16 root root      12288 8月   6 19:24 tmp
drwxr-xr-x  15 root root       4096 7月  25 11:17 usr
drwxr-xr-x  14 root root       4096 4月  27 02:30 var
lrwxrwxrwx   1 root root         30 7月  25 10:31 vmlinuz -> boot/vmlinuz-4.15.0-22-generic
lrwxrwxrwx   1 root root         30 7月  25 10:31 vmlinuz.old -> boot/vmlinuz-4.15.0-29-generic
``` 
- 输入重定向 
```
ltt@myshell-pc:~$ wc -c < a
1873
```  
- 管道命令 
```
ltt@myshell-pc:~$ ls -al / | wc -c
1980
```
```
ltt@myshell-pc:~$ ping baidu.com &
[process id: 17966]
ltt@myshell-pc:~$ PING baidu.com (220.181.57.216) 56(84) bytes of data.
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=1 ttl=51 time=20.8 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=2 ttl=51 time=22.9 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=3 ttl=51 time=24.1 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=4 ttl=51 time=22.1 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=5 ttl=51 time=21.2 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=6 ttl=51 time=20.8 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=7 ttl=51 time=20.7 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=8 ttl=51 time=20.5 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=9 ttl=51 time=20.8 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=10 ttl=51 time=20.6 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=11 ttl=51 time=20.8 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=12 ttl=51 time=22.6 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=13 ttl=51 time=23.1 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=14 ttl=51 time=21.7 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=15 ttl=51 time=20.7 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=16 ttl=51 time=26.5 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=17 ttl=51 time=21.7 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=18 ttl=51 time=21.1 ms
64 bytes from 220.181.57.216 (220.181.57.216): icmp_seq=19 ttl=51 time=22.1 ms
ls
a		  jdk1.8.0_172	Templates  编程		 文档
a.c		  keshe		TTMSO	   第三周算法题  下载
ACM		  liuxiao	TTMSyan    公共的	 线程日常练习
beifen		  oncerun.c	Videos	   刘甜		 项目myshell及其文档
Desktop		  opt		vim	   模板		 音乐
examples.desktop  Pictures	vim_old    日常练习	 桌面
extract		  Projects	vimplus    视频
f怪异问题.c	  Public	vimrc.zip  收藏
git		  snap		wget-log   图片
```
``` 
ltt      17788  0.0  0.0  17624  2528 pts/2    S+   20:52   0:00 ./ltmyshell
root     17846  0.0  0.0      0     0 ?        D    20:53   0:00 [kworker/u8:0]
ltt      17879  3.0  3.5 787996 126544 tty2    Sl+  20:55   0:07 /opt/google/chr
root     17887  0.0  0.0      0     0 ?        I    20:55   0:00 [kworker/0:2]
ltt      17896  0.0  1.3 681036 49504 tty2     Sl+  20:55   0:00 /opt/google/chr
root     17916  0.0  0.0      0     0 ?        I    20:55   0:00 [kworker/3:1]
ltt      17936  0.0  0.1  30236  5644 pts/1    Ss   20:57   0:00 bash
root     17948  0.0  0.0      0     0 ?        I    20:58   0:00 [kworker/2:1]
root     17961  0.0  0.0      0     0 ?        I    20:58   0:00 [kworker/1:0]
ltt      17966  0.0  0.0  36132  3064 pts/2    S+   20:59   0:00 ping baidu.com
ltt      17967  0.0  0.1  46780  3692 pts/1    R+   20:59   0:00 ps -aux
ltt@ltt-X555QG:~$ kill 17966
``` 
- 查看历史命令  
```
ltt@myshell-pc:~$ history
1  cd
2  wc -c < a
3  ls -al / | wc -c
4  ping baidu.com
5  ls
6  ping baidu.com &
7  ls
8  history
```
6、总结  
这一次的项目，牵扯到许多我从来没了解过的东西，刚开始了解他们画了挺长时间。通过这次做项目，发现，自己平时还是，学到什么，看什么，主动性不强，希望自己能够有所改善。
