## iptables相关概念

iptables是运行在用户空间的应用软件，通过控制Linux内核netfilter模块，来管理网络数据包的处理和转发。它通过定义规则并交给内核中的netfilter即网络过滤器来读取，从而实现iptables的功能。它的作用有：

- ip数据包的条件过滤（防火墙，流量统计）
- 重定向SNAT,DNAT(内外网访问)
- 透明代理REDIRECT（代理服务器）

来看一个简单iptables的命令：

> iptables -t filter -A INPUT -i *eth0 -p tcp -s* X.X.X.X -j DROP

它表示在**filter表**的**INPUT链**中增加一条**规则**，规则的条件是：

- 数据包从指定网卡eth0进来
- 数据包是采用tcp协议传输
- 数据包的源地址为X.X.X.X

规则的动作是：满足上面条件的数据包直接进行丢弃。这里就涉及到**表，链，规则**的几个核心概念。理解iptables，核心是要先理解**四表五链和规则**的概念。表由链组成，链容纳各种规则，如下图所示：![内建表和内建链](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\讲一下iptables\内建表和内建链.png)

### 四表

iptables内部(kernel的netfilter模块)维护四个Table，分别是filter, nat, mangle, raw, 他们分别对应不同的功能。![tables](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\讲一下iptables\tables.png)

如上图所示，四种内建规则表，优先级 raw > mangle > nat > filter。

| 规则表     | 描述                                                         |
| :--------- | ------------------------------------------------------------ |
| **filter** | 1. 主要用于对数据包进行过滤<br />2. 表内包含三个链，INPUT，OUTPUT, FORWARD |
| **nat**    | 1. 主要用于修改数据包的ip地址，端口等信息(网络地址转换)<br />2. 表内包含三个链，PREROUTING, POSTROUTING, OUTOUT; |
| **raw**    | 1. 主要用于确定是否对该数据包进行状态跟踪<br />2. 表内包含两个链，OUTPUT, PREROUTING; |
| **mangle** | 1. 主要用于修改数据包的TOS,TTL以及数据包设置mark标记<br />2. 表内包含五个链，PREROUTING,POSTROUTING,INPUT,OUTPUT,FORWARD |

### 五链

iptables中的表是用于容纳规则链，iptables命令中设置数据过滤或处理数据包的策略就做规则，将多个规则合成一个链，叫做规则链。规则链依据处理数据包的位置不同分类如下：

| 规则链          | 描述                                              |
| --------------- | ------------------------------------------------- |
| **PREROUTING**  | 在进行路由判断之前所要进行的规则(DNAT/REDIRECT)   |
| **INPUT**       | 处理入站的数据包                                  |
| **OUTPUT**      | 处理出站的数据包                                  |
| **FORWARD**     | 处理转发的数据包                                  |
| **POSTROUTING** | 在进行路由判断之后所要进行的规则(SNAT/MASQUERADE) |

规则链的现后顺序：

- 入站： PREROUTING -> INPUT
- 出站： OUTPUT -> POSTROUTING
- 转发： PREROUTING -> FORWARD -> POSTROUTING

下图就是数据包在表和规则链中的发送顺序。

![iptables数据流向图](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\讲一下iptables\iptables数据流向图.png)

这里要注意一下**PREROUTING和POSTROUTING**的区别

通常内网到外网是post，外网到内网是pre，但是外和内只是个相对概念，在一定条件下是可以转换的。落实到网卡上，**对于每个网卡数据流入的时候必然经过pre，数量流出必然经过post**。最要紧的是我们要记住PREROUTING是“路由规则”之前的动作，POSTROUTING是“路由规则”之后的动作！

**源地址发送数据--> {PREROUTING-->路由规则-->POSTROUTING} -->目的地址接收到数据。**所以这就是为什么我们打标签是在mangle表的PREROUTING链。

### 规则

规则(rules)是iptables过滤数据包的具体方法和策略。

- rules包含一个条件和一个目标。
- 如果条件满足，就执行**目标中的规则或者特定动作**；
- 如果不满足，就判断下一条rule。

目标动作： ACCEPT, DROP, REJECT, LOG, SNAT, DNAT, MARK, RETURN, MASQUERADE。

| 目标           | 动作                               |
| -------------- | ---------------------------------- |
| **ACCEPT**     | 允许数据包通过                     |
| **DROP**       | 直接丢弃数据包，不给任何回应信息   |
| **REJECT**     | 拦阻该数据包，并传达封包通知对方   |
| **REDIRECT**   | 将数据包重新导向另一个端口（PNAT） |
| **LOG**        | 保存数据包信息记录                 |
| **SNAT**       | 改写数据包的来源IP                 |
| **DNAT**       | 改写数据包的目的IP                 |
| **MARK**       | 将数据包打上标记                   |
| **RETURN**     | 结束目前规则链中的过滤程序         |
| **MASQUERADE** | 改写数据包源IP为网卡IP             |

现在来看看ACCEPT,DROP,REJECT,RETURN的执行流程。

#### ACCEPT

匹配到目标为ACCEPT的rule时的执行顺序，将数据包放行并且不在对比同一链其他规则，直接跳往下一个规则链。

![accept](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\讲一下iptables\accept.png)

#### DROP

匹配到目标为DROP的rule时的执行顺序，丢弃数据包不予处理，并且不再对比其他规则，直接中断数据过滤程序。

![DROP](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\讲一下iptables\DROP.png)

#### REJECT

匹配到目标为REJECT的rule时的执行顺序，阻拦数据包并传送数据包通知对方，不再对比其他规则，直接中断过滤程序。返回的数据包有：ICMP port-unreachable, ICMP echo-reply, tcp-reset。

![REJECT](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\讲一下iptables\REJECT.png)

#### RETURN

匹配到目标为RETURN的rule时的执行顺序，结束在目前规则链中的过滤程序，返回主规则链继续过滤。

![RETURN](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\讲一下iptables\RETURN.png)

#### SNAT

匹配到Target为SNAT的rule时的执行顺序(同ACCEPT), 改写数据包源IP为某特定IP或IP范围，可以指定port范围，处理完动作后，将直接跳往下一个规则链。

#### DNAT

匹配到Target为DNAT的rule时的执行顺序(同ACCEPT), 改写数据包目的IP为某特定IP或IP范围，可以指定port范围，处理完动作后，将直接跳往下一个规则链。(同ACCEPT)

#### MARK

匹配到Target为MARK的rule时的执行顺序, MARK将数据包标上标记数，以便作为后续过滤条件的判断依据，处理完此动作后，将会继续比对其它规则。

#### LOG

匹配到Target为LOG的rule时的执行顺序，将数据包记录在日志中，处理完此动作后，将会继续比对其它规则。

#### MASQUERADE

匹配到Target为MASQUERADE的rule时的执行顺序，动态改写数据包来源IP为网络接口IP，可以指定port对应的范围，处理完此动作后，直接跳往下一个规则链.

## iptables命令语法

### 命令格式

**iptables  [-t 表名]  命令选项 ［链名］ ［条件匹配］ ［-j 目标动作或跳转］**

说明：表名、链名用于指定命令所操作的表和链，命令选项用于指定iptables的执行方式（比如：插入规则、增加规则、删除规则、查看规则等；条件匹配用于指定对符合什么样条件的数据包进行处理；目标动作或跳转用于指定数据包的处理方式，比如允许通过、拒绝、丢弃、跳转（Jump）给其它链处理。

#### 表名

表选项用于指定命令应用于哪个iptables內建表。如果不指定，默认是filter表。

#### 命令选项

命令选项用于指定iptables的执行方式，包括插入规则，删除规则和添加规则，如下表所示：

![命令选项](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\讲一下iptables\命令选项.png)

#### 条件匹配

指定数据包所具有的特征，包括源地址，目的地址，传输协议和端口号等。

![条件匹配](C:\ubuntu2\shared\Iceserver\vsclient\seaice\android data\讲一下iptables\条件匹配.png)

## iptables使用案例

1.拒绝进来的所有ICMP协议数据包

> iptables -I INPUT -p icmp -j REJECT

2.丢弃从接口(eth1)进入本机的源地址为某个网段地址的数据包

> iptables -A INPUT -i eth1 -s 192.168.0.0/16 -j DROP

3.允许所有来自外部的http连接请求

> iptables -A INPUT -p tcp --dport 80 -m state --state NEW, ESTABLISHED -j ACCEPT
>
> iptables -A OUTPUT -p tcp --sport 80 -m state --state ESTABLISHED -j ACCEPT

4.阻止指定IP地址

> iptables -A INPUT -i eth0 -p tcp -s x.x.x.x -j DROP

5.对局域网内mac地址为00:0C:29:56:A6:A2主机开放其联机

> iptables -A INPUT -m mac --mac-source 00:0C:29:56:A6:A2 -j ACCEPT

6.将80端口的封包传递到8080端口

> iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT --to-ports 8080

## 分析手机中的规则链

在android手机中，手机启动时netd进程会去创建iptables的规则链。我们可以通过adb shell执行iptables命令，来查看有哪些规则链，也可以来分析里面的规则信息。

通过iptables -L INPUT来查看INPUT链里面的规则，可以看到它包含了很多子链。

```
seaice:/ # iptables -L INPUT
iptables -L INPUT
Chain INPUT (policy ACCEPT)//表示默认是ACCEPT
target(目标)            prot(协议)    opt          source(源地址)        destination(目的地址)
bw_INPUT                all          --           anywhere             anywhere
fw_INPUT                all          --           anywhere             anywhere
input_dos               all          --           anywhere             anywhere
limit                   all          --           anywhere             anywhere
bw_VIDEOCALL_IN         all          --           anywhere             anywhere
bw_VIDEOCALL_OUT        all          --           anywhere             anywhere
bw_videocall_box        all          --           anywhere             anywhere
firewall_f              all          --           anywhere             anywhere
```

我们选择其中的input_dos这个子链来继续分析，如下所示：

```
seaice:/ # iptables -L input_dos
iptables -L input_dos
Chain input_dos (1 references)
target    prot   opt   source       destination
ACCEPT    tcp    --    anywhere     anywhere      ctstate NEW limit: avg 50/sec burst 50
DROP      tcp    --    anywhere     anywhere      ctstate NEW
```

input_dos包含两个规则，target是ACCEPT的这个规则，表示每秒可以接收50个新连接并且对前面50个链接不进行限制，直接ACCEPT。假如不满足这个条件，每秒超过50个新连接，就会接着走target是DROP的规则，表示只要是新连接就直接丢弃。这两个规则需要配合使用。可以看出，这个是防范SYN-FLOOD碎片攻击的规则链。那么反推出来它的iptables命令应该是：

> iptables -N input_dos
> iptables -A input_dos -m conntrack --ctstate NEW -m limit --limit 50/s --limit-burst 50 -j ACCEPT
> iptables -A input_dos -m conntrack --ctstate NEW -j DROP
> iptables -A INPUT -j input_dos

其中，iptables使用的-m模块的用法可以参考这个链接：https://www.cnblogs.com/dongzhanyi123/p/12965704.html

## 参考

https://www.cnblogs.com/wspblog/p/4297160.html

https://www.cnblogs.com/dongzhanyi123/p/12965704.html

https://www.cnblogs.com/whych/p/9147900.html
https://www.codeplayer.org/Wiki/Router/Linux%20ip%E5%91%BD%E4%BB%A4%E8%AF%A6%E8%A7%A3.html