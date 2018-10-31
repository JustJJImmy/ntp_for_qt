

@[TOC](目录)

# NTP 协议简介
NTP，Network timestamp protocol，网络时间协议。
NTP client 简单来说，就是以网络上某个节点上的时间为基准，校正本机时间。
NTP server 即提供本机时间戳给 client 校准的服务器。
NTP 协议是基于 UDP 的。

# 原理
NTP 的实现是 C/S 结构的，client 向 server 发送时间校准请求，server 返回校准后时间。
既然是网络访问，那必然会有网络延时问题，这就必然导致 server 返回的时间戳不准。
如何解决呢？
我们来分别看一下 client 和 server 的时间线：

```
server --------------------------------------------  ts
                       t2          t3

                t1                        t4
client --------------------------------------------  tc
```

如上图，定义：
 - ts 为 server 的时间戳
 - tc 为 client  的时间戳
 - ts、tc 在 NTP 协商之前互不干扰，设 ts、tc 之间的误差 设为 T
 - t1 为 client 向 server 发起 NTP 请求的时间戳，这个时间戳在 tc 上
 - t2 为 server 收到 client 的 NTP 请求的时间戳，这个时间戳在 ts 上
 - t3 为 server 向 client 发起 NTP 响应的时间戳，这个时间戳在 ts 上
 - t4 为 client 收到 server 的 NTP 响应的时间戳，这个时间戳在 tc 上
 - d 为 client 与 server 之间的网络延时


下面，我们来计算 T。
根据以上的变量，我们可得出如下方程组：

 1. t2 = t1 + d + T。 即，t2 的数值，是 t1 加上网络延时 d、再加上 ts 、tc之间的误差 T
 2. t4 = t3 +d + (-T)。即，t4 的数值，是 t3 加上网络延时 d、再加上 tc、ts 之间的误差 -T

其中， t1、t2、t3、t4 都是确定的数值，即可解出方程租:
```
t2 - t4 = t1 + T - t3 - (-T) 
```
变换后可得如下算式：
```
T = ((t2 - t1) + (t3 - t4) ) /2
```
这样，有了 ts 和 tc 之间的时间戳误差，client 就可以调整了:
```
new tc = old tc + T
```



# 协议内容
![在这里插入图片描述](https://img-blog.csdnimg.cn/20181101040101674.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3UwMTE1NDY3NjY=,size_16,color_FFFFFF,t_70)
	
	
	
NTP报文格式如上图所示，它的字段含义参考如下:

 1. LI 闰秒标识器，占用2个bit。0 即可。
 2. VN 版本号，占用3个bits，表示NTP的版本号，现在为3
 3. Mode 模式，占用3个bits，表示模式。 3 表示 client， 2 表示 server
 4. stratum（层），占用8个bits。不清楚怎么用
 5. Poll 测试间隔，占用8个bits，表示连续信息之间的最大间隔。不清楚怎么用
 6. Precision 精度，占用8个bits，，表示本地时钟精度。 不清楚怎么用
 7. Root Delay根时延，占用8个bits，表示在主参考源之间往返的总共时延。 不清楚怎么用
 8. Root Dispersion根离散，占用8个bits，表示在主参考源有关的名义错误。 不清楚怎么用
 9. Reference Identifier参考时钟标识符，占用8个bits，用来标识特殊的参考源。不同的 NTP server 改字段不一样
 10.  参考时间戳，64bits时间戳，本地时钟被修改的最新时间。一般由 server 端填写，表示 server 上次同步时间戳的时间点
 11. 原始时间戳，客户端发送的时间，64bits。即公式中的 t1 。client 请求时必须填写，server 端响应时、回写请求包里的该字段
 12. 接受时间戳，服务端接受到的时间，64bits。即公式中的 t2 。server 端填写
 13. 传送时间戳，服务端送出应答的时间，64bits。即公式中的 t3。server 端填写
 14. 认证符（可选项）


注意：t4 为client 收到响应的时间戳，只有收到了才会产生，因此 t4 不体现在协议报文里。



# 实现
网络延时 d，计算时假定了请求和响应的延时相等，但真实网络环境非常复杂，UDP 本身可靠度也不够高，可能请求包很快就到服务器、而响应包在网络上饶了一大圈才到，这就会产生误差。
这种误差很难彻底消除，但还是有一些手段可以减少误差。
以前做物理实验，测量物体的长度、重量等，一般要求测量多次，最后取平均值。
本实现里思路也是发起多次 ntp 请求，最后取平均误差。
但考虑到计算机的计算环境，略有优化：

 1. 取 10 个不同地域的共有 NTP server，分别获取时间戳误差
 2. NTP 请求成功数少于 3 个 认为 失败
 3. NTP 请求成功数大于等于 3 个 且 小于 8 个，则求和计算平均误差
 4. NTP 请求成功数等于 8 个时，按位计算平均误差
 5. NTP 请求成功数等于 9 个时，去掉一个最大值后，按位计算平均误差
 6. NTP 请求成功数等于 10 个时，去掉一个最大值、去掉一个最小值后，按位计算平均误差

# 代码
https://github.com/JustJJImmy/ntp_for_qt.git