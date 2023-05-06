# Lab 2总结
本节学习了《xv6:a simple, Unix-like teaching operating system》第二章内容

## 2. Operating system organization

操作系统必须实现的三个特性：

- multiplexing：操作系统必须同时支持多种活动，操作系统必须在各个进程之间time-share计算机的资源。比如进程数量比硬件CPU核心数还多，但是操作系统必须让所有进程都有机会能执行
- isolation：操作系统必须为进程之间保持独立性。一个进程崩溃了，其他进程应该不受这个崩溃的进程的影响
- interaction：进程之间不应该完全孤立，有时候需要进行一些有目的的交互

Xv6运行在一个多核 RISC-V 微型处理器上，RISC-V 是一个64位的CPU，同时xv6是通过“LP64”C来写的，这意味着，long(L)和pointer(P)在C语言中是64位的，而int是32位的




### 2.1 Abstracting physical resources

我们为什么需要操作系统：操作系统能够对硬件资源进行抽象，为用户和程序提供相应的服务接口，避免程序和用户直接接触硬件资源

- 提供便捷的服务：我们无需关心硬件的特性，以及底层是如何实现的，例如查找一个文件，这时候我们并不需要直到整个文件系统是如何实现的，也不需要关心如何找到硬盘中的物理地址，以及通过文件描述符来进行通信，我们可以直接使用文件描述符来通信，把他当作一个整数来使用就好，而不需要关心它的内部
- 保证进程之间的独立性：如果程序直接接触硬件，比如CPU，当某个程序需要使用CPU时，也就意味着其他程序需要同时放弃CPU，这种情况在各个程序都相互信任彼此，并且程序中没有bug的时候是好方法。但是程序之间往往并不信任，也存在各自的bug。这时候我们通过操作系统来分配cpu，并且保存和读取相关的寄存器数据，由于是操作系统来分配，各个程序也不需要关心时间问题。还能够保证安全，例如不允许程序直接操作敏感资源，而是通过抽象硬件资源来提供服务。




### 2.2 User mode, supervisor mode, and system calls

独立性需要程序和操作系统之间的强硬边界来保证，这样一个进程出现错误时不会影响其他进程和操作系统本身，并且操作系统能够清理失败的进程。为了能够实现独立性，因该保证：

- 进程不能擅自修改甚至读取操作系统的数据结构和指令
- 一个进程不能访问其他进程的内存

CPU提供了硬件支持来保证独立性，比如 RISC-V 有三种状态：

- machine mode：此状态下的指令具有最高的权限，CPU以 machine mode启动，machine mode主要用来配置电脑，之后切换到supervisor mode
- supervisor mode：在该状态下，CPU能够执行特权指令，比如恢复/禁用中断，对储存页表的寄存器进行读写操作等。运行在supervisor mode也成为运行在 kernel space。运行在内核空间的软件叫做内核。
- user mode：运行普通程序，也成为在user space中运行。如果CPU想运行内核函数（比如系统调用），那么必须先进入supervisor mode，进入的方法是使用```ecall```指令，只有内核才能控制进入supervisor mode的进入点.



### 2.3 Kernel organization

根据操作系统的哪些部分运行在supervisor mode来划分，内核可以分为：
Monolithic kernel: 整个操作系统都在kernel中，所有的系统调用都运行在supervisor mode下。

- 优点：方便，设计者无需决定哪些部门不需要全部的硬件特权；便于操作系统的不同部分进行交互；
- 缺点：操作系统不同部分之间的接口很复杂，容易发生错误。并且在monolithic kernel下，错误发生在supervisor mode下，可能会导致整个操作系统崩溃
    Microkernel：减少在supervisor mode下运行的代码，保证kernel的安全，大部分操作系统代码都在user mode下执行。
- 优点：在微内核下，内核接口只包含少量的底层功能，比如启动应用，传递消息等，这使得内核非常简洁。作为进程运行的操作系统服务叫做servers。微内核下，大部分操作系统都属于用户级别的servers

xv6是一个monilithic kernel



### 2.4 Code: xv6 organization



### 2.5 Process overview

隔离或者说独立性的单元是进程。抽象进程能够阻止一个进程监听或者破坏其他进程的内存,CPU,文件描述符等，还能够阻止进程破坏内核本身。

为了实现进程之间的独立，进程这种抽象提供一种机制让程序认为自己拥有一个独立的机器：
进程提供给程序一个私有的内存系统，or地址空间，其他的进程不能对其进行读或者写

- Xv6使用页表来给每个进程分配地址空间（虚拟地址）。之后页表将虚拟地址（RISC-V指令集操作的）翻译成物理地址（CPU芯片发送给主存的地址）
- Xv6为每个进程维护了一个独立的页表
    进程还提供给程序看上去是自己独属的CPU来执行程序的指令

RISC-V中的指针是64位的，硬件在查找页表中的虚拟地址的时候仅仅使用其中的低39位，并且xv6仅仅使用这39位中的38位。因此最大的地址是$$2^38 - 1 = 0x3fffffffff$$。地址空间从低地址到高地址分别是指令，全局变量，栈，最后是堆。地址空间最上面两个页：trampoline和trapframe

进程有很多状态，都保存在结构体`struct proc`中，其中最重要的是进程的页表，进程的内核栈以及进程的运行状态。

每个进程都有线程（thread），线程能够被暂停和继续。内核通过暂停当前正在运行的线程并且唤醒其他进程的线程来实现在进程之间的切换。每个进程有两个独立的栈，用户栈和内核栈。当处于用户态时，只使用用户栈，内核栈是空的；当处于内核态时，只使用内核栈，但是用户栈中保存了数据。即便用户栈损坏了，也不会影响内核栈。

进程通过使用`ecall`指令进入内核态，完成系统调用，之后使用`sret`返回用户态。进入内核态后，之前运行的用户指令被暂停，返回用户态之后继续执行之前的用户指令。



### 2.6 Code:starting xv6, the first process and system call

当RISC-V启动时，先运行一个存储于ROM中的boot loader来家在xv6 kernel到内存中，接着CPU在`_entry`处开始运行xv6。boot loader将xv6内核加载到0x80000000的物理地址处，因为前面有I/O设备。

在`_entry`中设置了一个初始stack来运行C代码，`stack0`来让xv6运行`kernel/start.c`。函数`start`现在machine mode下做一些配置，然后通过`mret`进入supervisor mode，并且设置返回地址为`main`来使program counter切换到`kernel/main.c`

`main`初始化几个设备和子系统，然后通过调用`userinit`来创建第一个进程，改进程执行了一个`initcode.S`的汇编程序，这个汇编程序进入内核并调用`exec`这个系统调用来执行`/init`进程。

一旦内核完成了`exec`，它会返回用户空间的`/init`进程，`/init`创建一个新的控制台设备，并且以文件描述符0,1,2打开，之后在控制台中启动shell，至此，系统启动了。



## Lab 2:system calls

### 1. trace

`trace <tracing_mask> <command>`，要求当调用了给定的tracing_mask对应的system call时，打印调用该system call的进程PID，system call的名称，system call的返回值。

该lab需要传递记录一下变量，在`kernel/sysproc.c`的`sys_trace()`中使用`argint`来获取mask，然后存储在进程的`struct proc`中。为此，我们需要在`struct proc`的定义中增加一项


### 2. sysinfo

`int sysinfo(struct sysinfo* addr);`
该系统调用获取系统当前空闲的内存以及不处于unused状态的进程数量，并且将结果储存在addr中

我们在`kernel/sysproc.c`中的`sys_sysinfo()`中获取系统当前空闲内存以及满足要求的进程数量，之后使用`copyout()`函数将结果返回用户空间(数据从内核空间传入用户空间)

为了统计当前空闲内存，我们需要查看`kernel/kalloc.c`文件，里面对物理内存按page大小进行了分配，并把所有的空闲页连成了一个链表，链表头就是结构体`kmem`中的`freelist`变量。因此为了统计空闲的物理内存，我们只需要遍历`freelist`查看有多少空闲的页，然后乘以页的大小也就是空闲物理内存。

- 这里想到了内部碎片和外部碎片，有些页可能没有用完，但是属于是内部碎片用不了了

系统的所有进程，都放在了`kernel/proc.c`文件中的`proc`数组中，该数组储存的是`struct proc`元素，因此我们只需要遍历该数组，访问每个元素也就是进程的状态即可。
