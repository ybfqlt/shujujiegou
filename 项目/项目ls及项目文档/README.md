### “ls”项目总结报告  
***
>项目名称：ls -a -l -R及其任意组组合  
>姓名：刘甜  
>起止时间：2018年7月26日至2018年7月29日

***
1.参考资料：linuxc编程实战  

2.项目要求：  
- s基础功能  
- -a -l -R三个参数,并能随意组合
- 在所有目录下都能跑  

3.完成的功能：  
- ls -a:列出文件下所有的文件，包括以“.“开头的隐藏文件（linux下文件隐藏文件是以.开头的，如果存在..代表存在着父目录）。  
- ls -l：列出长数据串，显示出文件的属性与权限等数据信息.
- ls -R:将目录下所有的子目录的文件都列出来，相当于我们编程中的“递归”实现.
- 它们的随意组合
  
4.设计思路：-a和-l主要是参考书上的操作,首先就是解析参数，对于参数的解析.然后是对于目录的解析，这里与参数是类似的，这里主要是将目录的参数与-a, -l, -R能够区分开来，这里用“-”来区分 。接下来是对目录的解析，主要用到opendir()函数来打开目录，然后用readdir读目录，获取目录下的信息，再使用strcpy()将目录名复制到一个保存完整路径的二维数组中，然后使用strcat()函数把得到的读到的文件名称连接在其后，这样就得到了一个文件的完整路径，再下来就是显示工作，‘-a’和无参数区别只是隐藏文件，所以，使用if判断其是否以‘.’开头，若是，在‘-a’时打印，其他则不打印。对于-l，则需要将文件的所有信息进行输出，这里只要把之前用lstat获取的文件信息被保存的地址传入然后依次打印即可。
最后就是-R参数的输出，对于-R参数的也是利用以上的函数，使用递归调用，还有就是递归的时候要注意路径名中的‘/’。  
5.设计中遇到的问题：  
- 在-l的输出那块，输出时间那，ctime()获取到的时间字符串，有自动换行，开始没了解到，最后才发现。
- -R在使用递归调用时，需要更大的空间去保存完整路径名，然而我的比较小，难以跑完主目录，但是改成动态二维数组，运行一半就会出现“已杀死”的提示。

***
   
6.结果展示：  
-a:
```
ltt@ltt-X555QG:~/liuxiao/one$ ./my_lslt -a
.              ..             lianxidaima    llltt          my_lslt        my_lslt.c      日常练习   

```
-l:
```
ltt@ltt-X555QG:~/liuxiao/one$ ./my_lslt -l
drwxr-xr-x   2 ltt  ltt   4096 Mon Jul 30 21:52:27 2018 lianxidaima
drwxr-xr-x   4 ltt  ltt   4096 Mon Jul 30 21:53:31 2018 llltt
-rwxr-xr-x   1 ltt  ltt  17656 Mon Jul 30 21:54:08 2018 my_lslt
-rw-r--r--   1 ltt  ltt  11134 Mon Jul 30 07:59:47 2018 my_lslt.c
drwxr-xr-x   2 ltt  ltt   4096 Thu Jul 26 19:31:39 2018 日常练习

```
-R:
```
ltt@ltt-X555QG:~/liuxiao/one$ ./my_lslt -R
lianxidaima    llltt          my_lslt        my_lslt.c      日常练习   
./lianxidaima:
a.out                          
cdvc                           example_62.c                   example_63.c                   
example_64                     example_65                     example_681.test               
example_682.test               fcntl_access                   fcntl_access.c                 
fcntl_lock                     fcntl_lock.c                   hello                          
hello.c                        ll                             ll.c                           
mls                            mls.c                          my                             
my_cdvc                        my_cdvc.c                      my_chmod                       
my_chmod.c                     my_chmod1.c                    my_chmod2                      
my_chmod2.c                    my_chmod3                      my_chmod3.c                    
my_creat                       my_creat.c                     my_ls                          
my_ls.c                        my_ls1.c                       my_ls2.c                       
my_lslt                        my_mv                          my_mv.c                        
my_rwl                         my_rwl.c                       rwl                            
show_files                     show_files.c                   test_umask                     
test_umask.c                   unlink_temp                    unlink_temp.c                  


./llltt:

刘甜第0周                  刘甜第1周练习题         
./llltt/刘甜第0周:
a.c                       b.c                       
c.c                       d.c                       e.c                       f.c                       
g.c                       h.c                       


./llltt/刘甜第1周练习题:
a1.c                                
a2.c                                a3.c                                e.c                                 
h.c                                 i.c                                 j.c                                 
k.c                                 p.c                                 u.c                                 
v.c                                 z.c                                 




./日常练习:
example_63.c                  
fcntl_access.c                fcntl_lock.c                  hello.c                       
my_cdvc.c                     my_chmod.c                    my_chmod2.c                   
my_creat.c                    my_mv.c                       my_rwl.c                      
show_files.c                  test_umask.c                  unlink_temp.c                 

```
-al:
```
ltt@ltt-X555QG:~/liuxiao/one$ ./my_lslt -al
drwxr-xr-x   5 ltt  ltt   4096 Mon Jul 30 21:54:08 2018 .
drwxr-xr-x   8 ltt  ltt   4096 Mon Jul 30 19:51:05 2018 ..
drwxr-xr-x   2 ltt  ltt   4096 Mon Jul 30 21:52:27 2018 lianxidaima
drwxr-xr-x   4 ltt  ltt   4096 Mon Jul 30 21:53:31 2018 llltt
-rwxr-xr-x   1 ltt  ltt  17656 Mon Jul 30 21:54:08 2018 my_lslt
-rw-r--r--   1 ltt  ltt  11134 Mon Jul 30 07:59:47 2018 my_lslt.c
drwxr-xr-x   2 ltt  ltt   4096 Thu Jul 26 19:31:39 2018 日常练习

```
-aR:
```
ltt@ltt-X555QG:~/liuxiao/one$ ./my_lslt -aR
.               ..               lianxidaima    llltt          my_lslt        my_lslt.c      日常练习   
./lianxidaima:

.                               ..                               a.out                          
cdvc                           example_62.c                   example_63.c                   
example_64                     example_65                     example_681.test               
example_682.test               fcntl_access                   fcntl_access.c                 
fcntl_lock                     fcntl_lock.c                   hello                          
hello.c                        ll                             ll.c                           
mls                            mls.c                          my                             
my_cdvc                        my_cdvc.c                      my_chmod                       
my_chmod.c                     my_chmod1.c                    my_chmod2                      
my_chmod2.c                    my_chmod3                      my_chmod3.c                    
my_creat                       my_creat.c                     my_ls                          
my_ls.c                        my_ls1.c                       my_ls2.c                       
my_lslt                        my_mv                          my_mv.c                        
my_rwl                         my_rwl.c                       rwl                            
show_files                     show_files.c                   test_umask                     
test_umask.c                   unlink_temp                    unlink_temp.c                  


./llltt:

.                               ..                               刘甜第0周                  
刘甜第1周练习题         
./llltt/刘甜第0周:
.                          ..                          a.c                       
b.c                       c.c                       d.c                       e.c                       
f.c                       g.c                       h.c                       


./llltt/刘甜第1周练习题:
.                                    
..                                    a1.c                                a2.c                                
a3.c                                e.c                                 h.c                                 
i.c                                 j.c                                 k.c                                 
p.c                                 u.c                                 v.c                                 
z.c                                 




./日常练习:
.                              ..                              
example_63.c                  fcntl_access.c                fcntl_lock.c                  
hello.c                       my_cdvc.c                     my_chmod.c                    
my_chmod2.c                   my_creat.c                    my_mv.c                       
my_rwl.c                      show_files.c                  test_umask.c                  
unlink_temp.c                 

```
-alR:
```
ltt@ltt-X555QG:~/liuxiao/one$ ./my_lslt -alR
drwxr-xr-x   5 ltt  ltt   4096 Mon Jul 30 21:54:08 2018 .
drwxr-xr-x   8 ltt  ltt   4096 Mon Jul 30 19:51:05 2018 ..
drwxr-xr-x   2 ltt  ltt   4096 Mon Jul 30 21:52:27 2018 lianxidaima
drwxr-xr-x   4 ltt  ltt   4096 Mon Jul 30 21:53:31 2018 llltt
-rwxr-xr-x   1 ltt  ltt  17656 Mon Jul 30 21:54:08 2018 my_lslt
-rw-r--r--   1 ltt  ltt  11134 Mon Jul 30 07:59:47 2018 my_lslt.c
drwxr-xr-x   2 ltt  ltt   4096 Thu Jul 26 19:31:39 2018 日常练习

./lianxidaima:
drwxr-xr-x   5 ltt  ltt   4096 Mon Jul 30 21:54:08 2018 .
drwxr-xr-x   8 ltt  ltt   4096 Mon Jul 30 19:51:05 2018 ..
-rwxr-xr-x   1 ltt  ltt  17656 Sun Jul 29 21:50:39 2018 a.out
-rwxr-xr-x   1 ltt  ltt   8640 Fri Jul 27 16:12:19 2018 cdvc
-rw-------   1 ltt  ltt      0 Tue Jul 24 11:43:02 2018 example_62.c
-rwx------   1 ltt  ltt     12 Tue Jul 24 15:21:10 2018 example_63.c
-rwx------   1 ltt  ltt      0 Tue Jul 24 17:19:39 2018 example_64
-rwx------   1 ltt  ltt     10 Tue Jul 24 20:49:49 2018 example_65
-rwxrwxrwx   1 ltt  ltt      0 Wed Jul 25 08:30:14 2018 example_681.test
-rwxrwx---   1 ltt  ltt      0 Wed Jul 25 08:30:14 2018 example_682.test
-rwxr-xr-x   1 ltt  ltt   8616 Tue Jul 24 17:19:28 2018 fcntl_access
-rw-r--r--   1 ltt  ltt   1214 Tue Jul 24 17:19:26 2018 fcntl_access.c
-rwxr-xr-x   1 ltt  ltt  13128 Tue Jul 24 20:49:34 2018 fcntl_lock
-rw-r--r--   1 ltt  ltt   2646 Tue Jul 24 20:40:35 2018 fcntl_lock.c
-rwxr-xr-x   1 ltt  ltt   8296 Tue Jul 24 10:15:37 2018 hello
-rw-r--r--   1 ltt  ltt    273 Tue Jul 24 10:17:51 2018 hello.c
-rwxr-xr-x   1 ltt  ltt  13480 Sat Jul 28 21:00:12 2018 ll
-rw-r--r--   1 ltt  ltt   7512 Sat Jul 28 21:45:17 2018 ll.c
-rwxr-xr-x   1 ltt  ltt  17824 Sat Jul 28 19:08:55 2018 mls
-rw-r--r--   1 ltt  ltt  11016 Sat Jul 28 20:02:27 2018 mls.c
-rw-r--r--   1 ltt  ltt      0 Sun Jul 29 13:59:50 2018 my
-rwxr-xr-x   1 ltt  ltt   8640 Wed Jul 25 11:29:34 2018 my_cdvc
-rw-r--r--   1 ltt  ltt    578 Fri Jul 27 16:11:57 2018 my_cdvc.c
-rwxr-xr-x   1 ltt  ltt  12720 Fri Jul 27 10:22:53 2018 my_chmod
-rw-r--r--   1 ltt  ltt   1167 Fri Jul 27 20:43:19 2018 my_chmod.c
-rw-r--r--   1 ltt  ltt    279 Fri Jul 27 10:21:48 2018 my_chmod1.c
-rwxr-xr-x   1 ltt  ltt  12720 Sun Jul 29 14:56:55 2018 my_chmod2
-rw-r--r--   1 ltt  ltt   2105 Sun Jul 29 15:34:35 2018 my_chmod2.c
-rwxr-xr-x   1 ltt  ltt  13000 Sun Jul 29 14:33:54 2018 my_chmod3
-rw-r--r--   1 ltt  ltt   3567 Sun Jul 29 14:36:17 2018 my_chmod3.c
-rwxr-xr-x   1 ltt  ltt   8568 Tue Jul 24 11:43:01 2018 my_creat
-rw-r--r--   1 ltt  ltt    530 Tue Jul 24 11:42:58 2018 my_creat.c
-rwxr-xr-x   1 ltt  ltt  17696 Sun Jul 29 22:36:20 2018 my_ls
-rw-r--r--   1 ltt  ltt    275 Fri Jul 27 20:04:17 2018 my_ls.c
-rw-r--r--   1 ltt  ltt  11207 Sat Jul 28 20:04:02 2018 my_ls1.c
-rw-r--r--   1 ltt  ltt  11207 Sat Jul 28 20:12:28 2018 my_ls2.c
-rwxr-xr-x   1 ltt  ltt  17656 Mon Jul 30 20:22:02 2018 my_lslt
-rwxr-xr-x   1 ltt  ltt   8424 Wed Jul 25 08:50:56 2018 my_mv
-rw-r--r--   1 ltt  ltt    575 Wed Jul 25 08:50:53 2018 my_mv.c
-rwxr-xr-x   1 ltt  ltt  13008 Tue Jul 24 15:21:06 2018 my_rwl
-rw-r--r--   1 ltt  ltt   1778 Tue Jul 24 15:21:05 2018 my_rwl.c
-rwxr-xr-x   1 ltt  ltt  13008 Tue Jul 24 15:10:19 2018 rwl
-rwxr-xr-x   1 ltt  ltt   8552 Sat Jul 28 14:32:33 2018 show_files
-rw-r--r--   1 ltt  ltt    860 Sat Jul 28 14:32:31 2018 show_files.c
-rwxr-xr-x   1 ltt  ltt   8424 Wed Jul 25 08:30:06 2018 test_umask
-rw-r--r--   1 ltt  ltt    455 Wed Jul 25 08:29:39 2018 test_umask.c
-rwxr-xr-x   1 ltt  ltt   8768 Wed Jul 25 09:43:41 2018 unlink_temp
-rw-r--r--   1 ltt  ltt    792 Wed Jul 25 09:44:57 2018 unlink_temp.c



./llltt:
drwxr-xr-x   5 ltt  ltt   4096 Mon Jul 30 21:54:08 2018 .
drwxr-xr-x   8 ltt  ltt   4096 Mon Jul 30 19:51:05 2018 ..
drwxr-xr-x   2 ltt  ltt   4096 Mon Jul 30 09:04:52 2018 刘甜第0周
drwxr-xr-x   2 ltt  ltt   4096 Mon Jul 30 09:04:52 2018 刘甜第1周练习题

./llltt/刘甜第0周:
drwxr-xr-x   5 ltt  ltt   4096 Mon Jul 30 21:54:08 2018 .
drwxr-xr-x   8 ltt  ltt   4096 Mon Jul 30 19:51:05 2018 ..
-rw-r--r--   1 ltt  ltt    129 Wed Jul 25 14:08:35 2018 a.c
-rw-r--r--   1 ltt  ltt    269 Wed Jul 25 14:08:35 2018 b.c
-rw-r--r--   1 ltt  ltt    191 Wed Jul 25 14:08:35 2018 c.c
-rw-r--r--   1 ltt  ltt    277 Wed Jul 25 14:08:35 2018 d.c
-rw-r--r--   1 ltt  ltt    263 Wed Jul 25 14:08:35 2018 e.c
-rw-r--r--   1 ltt  ltt    226 Wed Jul 25 14:08:35 2018 f.c
-rw-r--r--   1 ltt  ltt    149 Wed Jul 25 14:08:35 2018 g.c
-rw-r--r--   1 ltt  ltt    304 Wed Jul 25 14:08:35 2018 h.c



./llltt/刘甜第1周练习题:
drwxr-xr-x   5 ltt  ltt   4096 Mon Jul 30 21:54:08 2018 .
drwxr-xr-x   8 ltt  ltt   4096 Mon Jul 30 19:51:05 2018 ..
-rw-r--r--   1 ltt  ltt    556 Mon Jul 23 22:21:48 2018 a1.c
-rw-r--r--   1 ltt  ltt    686 Mon Jul 23 23:33:40 2018 a2.c
-rw-r--r--   1 ltt  ltt    318 Mon Jul 23 23:39:31 2018 a3.c
-rw-r--r--   1 ltt  ltt    841 Mon Jul 23 23:53:05 2018 e.c
-rw-r--r--   1 ltt  ltt    505 Mon Jul 23 23:43:32 2018 h.c
-rw-r--r--   1 ltt  ltt    556 Mon Jul 23 11:09:15 2018 i.c
-rw-r--r--   1 ltt  ltt    171 Mon Jul 23 11:21:44 2018 j.c
-rw-r--r--   1 ltt  ltt    308 Mon Jul 23 14:12:23 2018 k.c
-rw-r--r--   1 ltt  ltt    687 Tue Jul 24 08:27:07 2018 p.c
-rw-r--r--   1 ltt  ltt    523 Mon Jul 23 16:25:17 2018 u.c
-rw-r--r--   1 ltt  ltt    487 Mon Jul 23 22:11:45 2018 v.c
-rw-r--r--   1 ltt  ltt    357 Mon Jul 23 14:34:10 2018 z.c





./日常练习:
drwxr-xr-x   5 ltt  ltt   4096 Mon Jul 30 21:54:08 2018 .
drwxr-xr-x   8 ltt  ltt   4096 Mon Jul 30 19:51:05 2018 ..
-rwx------   1 ltt  ltt     12 Tue Jul 24 15:21:10 2018 example_63.c
-rw-r--r--   1 ltt  ltt   1214 Tue Jul 24 17:19:26 2018 fcntl_access.c
-rw-r--r--   1 ltt  ltt   2646 Tue Jul 24 20:40:35 2018 fcntl_lock.c
-rw-r--r--   1 ltt  ltt    273 Tue Jul 24 10:17:51 2018 hello.c
-rw-r--r--   1 ltt  ltt    578 Wed Jul 25 11:29:10 2018 my_cdvc.c
-rw-r--r--   1 ltt  ltt   1167 Tue Jul 24 21:19:40 2018 my_chmod.c
-rw-r--r--   1 ltt  ltt   1212 Tue Jul 24 21:45:42 2018 my_chmod2.c
-rw-r--r--   1 ltt  ltt    530 Tue Jul 24 11:42:58 2018 my_creat.c
-rw-r--r--   1 ltt  ltt    575 Wed Jul 25 08:50:53 2018 my_mv.c
-rw-r--r--   1 ltt  ltt   1778 Tue Jul 24 15:21:05 2018 my_rwl.c
-rw-r--r--   1 ltt  ltt    866 Thu Jul 26 10:36:18 2018 show_files.c
-rw-r--r--   1 ltt  ltt    455 Wed Jul 25 08:29:39 2018 test_umask.c
-rw-r--r--   1 ltt  ltt    792 Wed Jul 25 09:44:57 2018 unlink_temp.c


```
7.总结：
在做项目时，开始算是比较懵，感觉提前没有构思，出现错误，就手足无措。所以，以后做项目时，一定要提前构思好，要纵观大局。
