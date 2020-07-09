# (U) cherlene

**Under development**

> Outlaw Country! WOO! - *Cherlene*

(U) `cherlene` is an open-sourced remake of the CIA tool `OutlawCountry` that was disclosed as part of the Vault7 leaks in 2017. I've attempted to keep the functionality similar to what was described in the version 1.0 rev A user manual from June 2015.


## (U) Overview

(U) `cherlene` is a kernel module that provides an unlisted table in `iptables` on a Linux target. With knowledge of the table name, an operator can create rules that take precedence over existing netfilter/iptables rules.

(U) The module will need to be compiled for the target kernel and operating system.


## (U) Prerequisites

(U) The target must have `iptables` available (See *Limitations* below).

(U) The `nf_nat` module must be loaded before the `cherlene` module.

(U) The Operator must have root access to the target.


## (U) Building

(U) **Compile module**

```bash
git clone https://github.com/oldrho/cherlene.git
cd cherlene
make
```

(U) The module is built as `cherlene.ko`

### (U) Debuging

(U) Compiling with debug enabled will write log messages to `/var/log/kern.log`
```bash
make debug
```


## (U) Operation


### (U) Installation

(U) Copy the module file to the target system.

(U) Ensure the `nf_nat` module is loaded
> lsmod | grep nf_nat

(U) If it is not loaded, load it
> modprobe nf_nat

(U) Load the module using `insmod`
> insmod cherlene.ko

(U) At this point the module file can be removed.
> rm cherlene.ko


### (U) Usage

(U) The `cherlene` table has a **PREROUTING** chain and supports rules using the **DNAT** target. Rules can be added or removed from the table in the same fashion 

> iptables -t cherlene -A PREROUTING -p tcp -s 1.1.1.1 -d 2.2.2.2 --dport 3333 -j DNAT --to-destination 4.4.4.4:5555

(U) This example will redirect any TCP traffic from 1.1.1.1 and destined for 2.2.2.2 on port 3333 to 4.4.4.4 on port 5555.

(U) Current rules can be listed as usual
> iptables -t cherlene -L


### (U) Removal

(U) Flush any existing rules in the `cherlene` table
> iptables -t cherlene -F

(U) Then remove the `cherlene` module using `rmmod`:
> rmmod cherlene

(U) Note: Regardless of the filename the module was installed with, the module name is configured in the *Makefile*. `rmmod` must be run on that module name, not the file name


## (U) Troubleshooting

(U) If the module fails to load:
- Was it compiled for the correct kernel?
- Is the `nf_nat` module loaded?

(U) If the `cherlene` table is not available
- Does the `cherlene` module appear in `lsmod`?
- Is the target using `nftables` or `bpfilter`? See *Limitations* below.

(U) If rules cannot be added to the `cherlene` table
- Is the rule syntax correct?
- Is the rule using a target other than DNAT?

(U) The rules do not appear to redirect traffic
- Do the rules still appear in the `cherlene` table?
- Is IP forwarding enabled? See *Limitations* below
- Are there FORWARD rules or policies that block the traffic? See *Limitations* below


## (U) Limitations

(U) Most modern kernels are using **nftables** or **bpfilter**. However, **iptables** may still be available through the `iptables-legacy` command. **This module is not compatible with nftables or bpfilter**.

(U) The target may not be configured to forward IP traffic.
```
> cat /proc/sys/net/ipv4/ip_forward
0
```

(U) A value of "0" indicates that forwarding is *not* enabled. To enable it, change the value to "1"
> echo '1' > /proc/sys/net/ipv4/ip_forward

(U) The target must have `nf_nat` available

(U) `modprobe` will not work with the module as it is not present in the target *modules.dep* file.

(U) If the target iptables service is stopped or restarted the module will not work after it is started again. To re-enable it, remove the module and reinstall it as above.

(U) `iptables` will only allow the DNAT target on the `nat` table by default. To bypass this, the module removes this restriction from the DNAT target while it is installed. **This can lead to detection** if a user on the target system notices that DNAT is available on other tables. The restriction is restored when the module is removed.

(U) IPv6 is not supported.


## (U) Development Notes

(U) This is not meant to be used in any operational capacity and was mostly a research project for myself on kernel modules and netfilter. Very limited testing has been completed and it's relevancy is quickly diminishing as most installations move to `nftables` or `bpfilter`.

(U) This is largely based on iptable_nat.c from the Linux kernel. It's been modified to avoid relying on kernel structures and to modify the DNAT target. If any of the original authors want me to add copyright notices, please let me know. [https://github.com/torvalds/linux/blob/master/net/ipv4/netfilter/iptable_nat.c](https://github.com/torvalds/linux/blob/master/net/ipv4/netfilter/iptable_nat.c)


### (U) FAQ

(U) Why did you make this?\
(U) Boredom.

(U) Does it work on *X* with *Y* kernel?\
(U) No idea. Go ahead and find out!

(U) Why is **(U)** at the beginning of every line\
(U) Classified documents in the US are classified in a number of ways. **U** is **Unclassified** which seems appropriate after it was leaked. The original document was classified **SECRET//NOFORN** (Secret, no foreign nationals) as were most of the lines in the document (**S//NF**).
