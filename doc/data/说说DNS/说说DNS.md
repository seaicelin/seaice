在手机的某些仪表测试场景，实网的网址通常是无法访问的，这使得手机在访问网址时会不停的重传，导致测试过程出现异常。那么想要阻止手机对网址进行访问，该怎么办呢？可以利用访问网址时，需要先进行DNS查询的原理，对DNS查询进行屏蔽，使得手机无法正常访问网址。可以采用iptables对DNS数据包进行过滤：

```
//新建一条子链，作用是过滤出DNS数据包并拒绝查询
iptables -t filter -N reject_dns_query
//添加规则，丢弃所有目标端口为53的udp数据包
iptablse -A reject_dns_query -p udp -dport 53 -j drop
//添加规则，丢弃所有目标端口为53的tcp数据包
iptables -A reject_dns_query -p tcp -dport 53 -j drop
//在OUTPUT链中，插入子链reject_dns_query
iptables -I OUTPUT -j reject_dns_query
```

执行完上面的命令，就会对发往端口为53的数据包进行丢弃，无法进行DNS查询解析了。在这里通过屏蔽DNS，我们来学习其中涉及到的知识点。

## DNS的概念

DNS是Domain Network System的缩写，主要作用是在域名和IP地址之间建立一种映射，也就是根据域名找出IP地址。可以把DNS的功能想象成一个电话本，通过电话本把人名映射到相应的电话号码。在DNS中，人名就是域名，电话号码就是IP地址。域名系统的管理由DNS服务器来完成，全球范围内的DNS服务器共同构成了一份分布式的域名-IP数据库。

程序使用DNS来查询地址一般分为两个步骤：

1. 程序向DNS服务器发起请求，获取域名对用的IP地址。
2. DNS服务器根据解析规则得到域名的IP地址，返回给客户端。

在DNS中，每一条域名和IP的对应关系称为一条记录，客户端一般会缓存这个记录以备后续使用，这个记录一般会有一个有效时间。

一个普通的网址 "www.baidu.com" 的完整域名其实是 "www.baidu.com."。因为根域名 . 对所有域名是一样的，通常都是省略。他的对应层次如下所示：

```
www.baidu.com.
主机名.次级域名.顶级域名.根域名
```

## DNS的查询方式

分为两类，一类是递归查询，一类是迭代查询。

### 递归查询

递归查询是一种DNS服务器的查询模式，在该模式下DNS服务器接收到客户机请求，必须使用一个准确的查询结果回复客户机。如果DNS服务器本地没有存储查询DNS信息，那么本地服务器就会成为DNS中的一台客户机，并向上级域名服务器发出查询请求，这种过程将持续到找到具有相关信息的域名服务器为止，然后将返回的查询结果提交给客户机。
过程中如果没有找到查询结果，重复递归上述操作直至根域名服务器，根域名服务器收到DNS请求后，把所查询得到的所请求的DNS域名中发送给顶级域名服务器，让顶级域名服务器去往下级域名服务器请求查找，如果找到了就原路返回。某域名服务器-->...->顶级域名服务器-->根域名服务器-->下一级域名服务器-->...-->本地域名服务器-->客户机。如果没有找到就报错，表示无法查询到相关信息。

### 迭代查询

DNS服务器另外一种查询方式为迭代查询，DNS服务器会向客户机提供其他能够解析查询请求的DNS服务器地址，当客户机发送查询请求时，DNS服务器并不直接回复查询结果，而是告诉客户机另一台DNS 服务器地址，客户机再向这台DNS服务器提交请求，依次循环直到返回查询的结果为止。

这两种查询中，递归是优于迭代的，因为迭代只有客户机和域名服务器之间的操作，而递归是除了客户机和域名服务器之间的操作，还涉及域名服务器之间的操作。

## DNS报文格式

DNS的请求和响应报文基本一致。其实事务ID，标志，问题计数，回答资源记录数，权威名称服务器计数，附加资源记录这6个字段是DNS的报文首部，共12个字节。

![DNS报文格式](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\说说DNS\DNS报文格式.png)

整个DNS报文格式分为：

1. 基础结构部分
2. 问题部分
3. 资源记录部分

### 基础结构部分

我们来分析一个实际的DNS请求报文，来看他的基础部分，下图红色框标记的便是基础部分。

![基础部分](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\说说DNS\基础部分.png)

该部分的字段含义如下

- **事务ID(Transaction ID):** 一个标识，请求和对应的响应报文的ID值一样。
- **标志(Flags):** 报文的标志字段，其中c, f, h是在响应报文中生效，所以查询报文wireshark没有显示出来。
  1. **Response:** 区分是请求还是响应，请求值是0，响应值是1。
  2. **Opcode:** 操作码，0是标准查询，1是反向查询，2表示服务器状态请求。
  3. **Authoritative:** 授权应答，只在响应报文中。1表示权威服务器，0表示不是。
  4. **Truncated(TC):** 表示是否被截断。1表示响应超过512字节并且被截断，只返回前512字节。
  5. **Recursion desired:** 期望递归。涉及到DNS的查询方式。
  6. **Recursion Available:** 可用递归，只出现在响应报文中。1表示服务器支持递归查询。
  7. **Z:** 保留字段，请求和响应报文中，值必须为0。
  8. **rcode:** 返回码字段，表示响应的差错状态。0是没有错误，1是报文格式错误，2是域名服务器失败，3是名字错误，4是查询类型不支持，5是拒绝。
- **Questions(问题计数)：**DNS查询请求数目。
- **(Answer RRs)回答资源计数：**DNS响应的数目。
- **Authority RRs(权威名称服务器计数)：** 权威名称服务器的数目。
- **Additional RRs(附加资源记录数)：**额外的记录数目，权威名称服务器对应IP地址的数目。

### 问题部分

接着继续来分析一个上面DNS的请求报文，看它的问题部分，下图红色框标记的便是问题部分。

![问题部分](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\说说DNS\问题部分.png)

问题部分是报文格式中的查询问题区域部分，表示DNS查询请求的问题。每个字段的含义如下

- **Name:** 要查询的域名。
- **Type:** DNS查询请求的资源类型。通常是为A类型，表示要获取域名对应的IP地址。
- **Class:** 地址类型，通常为互联网地址，值为1.

### 资源记录部分

DNS报文格式中最后的三个字段，回答问题区域字段和权威名称服务器域名字段以及附加信息字段。这三个字段个格式如下所示：![资源记录格式](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\说说DNS\资源记录格式.png)

我们依据上面的格式来分析DNS报文的响应报文，如下所示：

![资源记录部分](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\说说DNS\资源记录部分.png)

首先，Answer区域，表示www.baidu.com的别名是www.a.shifen.com， www.a.shifen.com的IP地址是14.215.177.38和14.215.177.39。这两个地址也就是www.baidu.com的IP地址。其次，权威名称服务器区域显示的是a.shifen.com域名对应的权威名称服务器名称，即ns1.a.shifen.com总共5个。最后附加信息区域显示的是5个权威名称服务器名称对应的IP地址。

Answer RRs，Authority RRs, Additional RRs 的值记录着每个区域对应的记录数量，这几个区域中的每条记录格式都是一样的。下面是每个字段的含义：

- **Name:** DNS请求的域名。
- **Type:** 资源记录类型。
  1. A类型：是IP解析，直接将域名指向某个IP。
  2. CNAME类型：也叫别名类型，也是一种指向关系。www.baidu.com -> www.a.shifen.com -> 14.215.177.38。为什么需要别名呢？(有什么好处)。
  3. NS类型：表示要获取该域名的权威名称服务器名称。
- **Class:** 地址类型。
- **Time to live:** 以秒为单位，表示资源记录的生命周期，一般是缓存当前资源记录的生命周期。
- **Data length:** 资源数据的长度。
- **Address:** 资源记录的数据。

## DNS报文是使用UDP还是TCP？

DNS同时占用TCP和UDP的端口号53，DNS的查询报文或者相应报文超过512字节时，使用TCP协议，其他则使用UDP协议。客户端向DNS服务器查询域名时，一般返回的内容都不超过512字节，用UDP即可，它不用进行三次握手，这样DNS服务器的负载更低，相应更快。在RFC1035规范中，有这段描述规定了512字节。

>Messages carried by UDP are restricted to 512 bytes (not counting the IP or UDP headers). Longer messages are truncated and the TC bit is set in the header.

在Internet上的标准MTU是576字节，UDP协议应用在被设计时，数据包大小会被限制在Internet标准MTU以下。DNS报文内容长度则被限制在512字节以下。DNS报文中有一个TC位，当DNS发出一个查询请求，返回的响应TC位被设置为1，说明该DNS响应长度超过512字节，后面响应有缺失，此时需要利用TCP进行重传。

## 手机中的DNS

在手机attach到网络，建立数据链路时，网络会返回手机的IP地址和DNS地址。对于软件开发者来说，常用的域名解析socket api有两个：

```
getaddrinfo:根据指定host或service名获取到对应的IP地址。
getnameinfo:根据指定的IP地址获取到对应的host或service名称。
```

Android中，DNS查找操作集中在netd守护进程中，以便进行系统级缓存。应用通过getaddrinfo调用查询时，会通过UNIX套接字发送到/dev/socket/dnsproxyd，再到netd进程，该进程会解析请求并再次调用getaddrinfo用于发出DNS查找请求，同时它会缓存结果以供其他应用使用。DNS 解析器实现主要包含在 bionic/libc/dns/ 中，部分包含system/netd/server/dns 中。

参考文档

https://source.android.com/devices/architecture/modular-system/dns-resolver?hl=zh-cn