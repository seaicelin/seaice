最近收到一个某地区拨打电话失败的概率性问题，据测试部门的同事反馈，发生的概率为 1/10。即拨打10次电话，基本上就会出现一次，出现问题后，再次拨打又好了，没啥规律，没有必现路径，真是奇怪了。每次遇到这样的问题，作为终端开发者，想到的就是网络问题。。。

 好了，话不多说，基本上每次遇到这样的问题，首先想到的就是对比其他型号的情况，以确认是否共性或者网络问题。

- 前提条件:同个地方，同样的卡
- 请测试我厂的其他型号是否也有这样的问题，如果我厂的其他型号也有这个问题，那么是共性问题。
-  如果我厂的机子都有问题，那么也请测试友商的机子，是否存在同样的问题，以判断是否网络问题。

 经过几次邀请测试，我厂其他型号不存在这个问题，友商的机子测试也没发现有问题，这下尴尬了。

为了寻找问题发生的可能原因，我按照下面的思路打开tcpdump进行分析：

> 1. NR SA网络下，拨打电话后是否能够正常EPS Fallback。
> 2.  EPS Fallback到LTE后，正常和异常情况下，小区信息是否一样。
> 3. 对比正常和异常的SIP信令是否一样。
> 4. 通过tcpdump调查SIP信令是否有发出去。
> 5. 当前手机信号情况如何，是否信号差。

通过仔细对比，发现其中第1，2，3，5点，正常和异常的log都基本一致，未发现可疑的地方。唯独第4点，从tcpdump看到SIP信令已经发给了Modem，但是一直在重复确认和虚假重传。涉及到这种重传的问题，首先想到的就是找Modem的同事确认信令是否成功发给基站了。

Modem同事分析后淡定的表示，问题发生的时间点手机信号强度正常，信令发给了基站并且基站回了ACK，所以信令已经正常发出去了。最后给的结论是终端测未发现异常，建议调查网络侧的IMS服务器。

结果意料之中，要网络侧去查，就必须邀请运营商一起联调，但是测试的结果又是其他型号正常，只有这个型号不正常。这要简单粗暴的说是网络问题也说不过去。。。

再次仔细确认log，确认没有可疑的地方后，只能把问题给了运营商接口人去分析看看，毕竟人家是邀请运营商联调的人，各种疑难杂症可能都见过。

得到的回复是：**正常和异常log中，MSS大小不一样，估计这个会导致问题，需要调查为什么会不一样？**

MSS？心想MSS变小不是通过的概率更大吗？怎么会导致问题呢？
既然MSS可能有问题，那就整理一下MSS相关的知识以及分析tcpdump时，自己心中的一些疑问。

## MSS是什么，怎么配置的，两个手机的MSS为什么会不一样？

MSS是TCP协议的一个概念，表示最大分段大小。也就是除去IP和TCP头部之后，一个网络包TCP数据部分的最大长度。TCP协议三次握手阶段会协商MSS值，MSS的值决定了每个TCP报文数据段的最大长度。这里还设计MTU的概念，MTU是一个网络包的最大长度，以太网中一般为1500字节，网络包超过MTU大小需要在网络层进行分片。![MSS](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\一个mss大小引发的思考\MSS.png)

那么这两个又有什么关系呢？

如果网络包MSS大小超过MTU，那么一个数据包就会在IP层进行分片，每一片都小于MTU。分片后由目标主机的IP层进行重组，再交给TCP层。但是如果在传输过程中，有一个IP分片丢失了，那么整个数据包都得重传，这样效率非常低。

为了提高传输效率，TCP协议在建立连接的时候通常需要协商双方的MSS，使得数据包在发给IP层之前已经在TCP层按照MSS大小来分段，使得分段之后形成的数据包不会大于MTU，这样就不会再IP层分片了。如果传输过程中发生丢包，进行重发时也是以MSS为单位，不用重传所有的分段，大大提高了传输效率。

MSS的大小取协商时较小的值，从下图可以看出，手机MSS是1240，Server是1312，最终TCP分段MSS是1240。协商后，可以看到REGISTER信令被分为两个包：6426,  6427。

![MSS协商](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\一个mss大小引发的思考\MSS协商.png)

知道了MSS是什么后，那么手机MSS=1240又是怎么来的呢？
终端在建立IMS PDN时，网络会回复MTU大小，这个问题当时网络的回复是MTU=1300，那么默认情况下MSS就是按照下面的公式计算：

> MSS = MTU(1300) - TCP header(20) - IPV6 header(40) = 1240。

为什么两个手机的MSS不一样呢？

那是因为另一台手机与网络三次握手时，网络回复的MSS小了，从下图看到网络回应MSS是1120，所以后续TCP报文是按照1120来进行分片，可以看包1357，1358。

![mss二次协商](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\一个mss大小引发的思考\mss二次协商.png)

奇怪的是，有时候建立IMS PDN网络回复的MTU是1432，但是在三次握手时SYN带出去的MSS却是1300。理论上的MSS大小应该是

> MSS = MTU(1432) - TCP header(20) - IPV6 header(40) = 1372。

为什么不是1372呢？是因为IMS在建立Socket时，上层会去设置**TCP_MAXSEG**选项，大小是1300。

>TCP_MAXSEG这个选项，它设置或接收最大出站 TCP 数据段尺寸。如果这个选项在建立连接前的设置，它将改变发送到另一端初始信息包中的 MSS 值。**这个值大于 MTU 接口值将被忽略而不起作用**。

所以MSS是两者中小的值：

> min(TCP_MAXSEG value,  MTU - TCP header - IPV6 header)

回到前面，建立IMS PDN时网络回复MTU是1300的情况，TCP_MAX_SEG也是设置1300，最后三次握手时SYN发出去的MSS是1240，也就解释的通了。

## 发生问题时，为啥PRACK信令是用UDP进行传输而不是TCP?

 这个是运营商的规范，如下：

![MSS规范](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\一个mss大小引发的思考\MSS规范.png)

 所以在IMS的配置文件中指定的mss大小，它有两个作用：

1. 它是用来选择采用哪种方式传输的判断依据。
2. 用来设置socket的**TCP_MAXSEG**选项，也就是MSS。

## UDP不是不会进行重传的吗？为啥从tcpdump看PRACK能进行重传呢？

用户数据报协议（UDP）是基于IP网络层协议的传输层协议。它提供高效的数据报文传输服务。UDP的缺点不能保证信息的交付，也不能防止信息重复，但UDP的简易性减少了传输协议的开销。UDP是不可靠的协议，它没有TCP的重传机制。

如下图所示，tcpdump中PRACK信令一直在重传。

![PRACK重传](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\一个mss大小引发的思考\PRACK重传.png)

既然UDP协议本身不支持重传，那么这里PRACK进行重传是在上层APP控制的，根据log看出确实是在APP控制重传：

> 10-23 14:24:05.944 2936 3491 D TimerQueue.cxx:50 | Adding timer: Timer E1 tid=6edfedcfed08cb3e ms=2000
> 10-23 14:24:07.947 2936 3491 D TimerQueue.cxx:50 | Adding timer: Timer E1 tid=6edfedcfed08cb3e ms=4000
> 10-23 14:24:11.951 2936 3491 D TimerQueue.cxx:50 | Adding timer: Timer E1 tid=6edfedcfed08cb3e ms=8000
> 10-23 14:24:19.958 2936 3491 D TimerQueue.cxx:50 | Adding timer: Timer E1 tid=6edfedcfed08cb3e ms=16000
> 10-23 14:24:35.970 2936 3491 D TimerQueue.cxx:50 | Adding timer: Timer E1 tid=6edfedcfed08cb3e ms=16000

## TCP重传的类型，发生重传的可能原因有哪些？
在网络环境中，数据包丢失是非常常见的事情。TCP协议有内建的处理机制来保证网络数据的传输正确。在Wireshark中，根据特性有几种TCP重传的分类，下面是几种TCP重传的分类：

- **TCP Retransmission**,这是一种单纯的重传，如果发送端等待接收端发送的ACK超过了TCP所设置的RTO，那么此时发送端便会重传刚发的数据包。
- **TCP Fast Retransmission**,主机在检测到原始数据包丢失后立即重新发送数据包，而不是等待完整的重传计时器到期。通常是发送方从接收方收到3个相同序列号的重复ACK时，就会触发TCP Fast Retransmission. 
- **TCP Spurious Retransmission(虚假重传)**，这标记着发送端发出的一个数据重传，实际上已经被接收者ACK过。由于一些原因，发送方认为一个数据包丢失，所以重新发送。

![TCP虚假重传](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\一个mss大小引发的思考\TCP虚假重传.png)

从图中可以看出发生了虚假重传，手机在包10388 ACK服务器后，服务器似乎没有收到，所以包10393,10399,10406,10413,10429都是重传，都被定义为虚假重传。与之相对应的包10394,10400,10407,10414,10430,都是手机侧进行确认应答。

那么发生虚假重传的可能性有哪些呢？

- 在超时之前手机侧的ACK没有发送到服务器，可能在传输的过程丢包了。
- 服务器的TCP协议栈有问题，导致没有处理接收到的ACK。

那么到这里，通过这个问题的log发散出来，整理了MSS,MTU,TCP,UDP相关的一些知识点，下次碰到类似的问题，就会有一个比较全面的概念，最后这个问题按照上面的分析提供了TR，还需要网络侧一起排查是什么原因导致服务器一直收不到数据包。

参考文档
[https://www.chappell-university.com/post/spurious-retransmissions-a-concern](https://www.chappell-university.com/post/spurious-retransmissions-a-concern)
[http://bbs.chinaunix.net/thread-4155778-1-1.html](http://bbs.chinaunix.net/thread-4155778-1-1.html)
[https://blog.csdn.net/Victordas/article/details/101534276](https://blog.csdn.net/Victordas/article/details/101534276)
[https://linux.die.net/man/7/tcp](https://linux.die.net/man/7/tcp)
图解系统-亮白风格-小林coding-v1.0.pdf