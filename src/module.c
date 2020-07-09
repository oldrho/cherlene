#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/ip.h>
#include <net/ip.h>

#include <net/netfilter/nf_nat.h>

static struct xt_table *cherlene_table = NULL;

static int __net_init cherlene_table_init(struct net *net);

static const struct xt_table cherlene_ipv4_table = {
	.name		= "cherlene",
	.valid_hooks	= (1 << NF_INET_PRE_ROUTING),
	.me		= THIS_MODULE,
	.af		= NFPROTO_IPV4,
	.table_init	= cherlene_table_init,
};

static unsigned int cherlene_do_chain(void *priv,
					 struct sk_buff *skb,
					 const struct nf_hook_state *state)
{
#ifdef DEBUG
	printk(KERN_INFO "cherlene packet\n");
#endif
	return ipt_do_table(skb, state, cherlene_table);
}

static const struct nf_hook_ops cherlene_ipv4_ops[] = {
	{
		.hook		= cherlene_do_chain,
		.pf			= NFPROTO_IPV4,
		.hooknum	= NF_INET_PRE_ROUTING,
		.priority	= INT_MIN,
	},
};

static int cherlene_register_lookups(struct net *net)
{
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(cherlene_ipv4_ops); i++) {
		ret = nf_nat_ipv4_register_fn(net, &cherlene_ipv4_ops[i]);
		if (ret) {
			while (i)
				nf_nat_ipv4_unregister_fn(net, &cherlene_ipv4_ops[--i]);

			return ret;
		}
	}

	return 0;
}

static void cherlene_unregister_lookups(struct net *net)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cherlene_ipv4_ops); i++)
		nf_nat_ipv4_unregister_fn(net, &cherlene_ipv4_ops[i]);
}

static int __net_init cherlene_table_init(struct net *net)
{
	struct ipt_replace *repl;
	int ret;

	if (cherlene_table)
		return 0;

	repl = ipt_alloc_initial_table(&cherlene_ipv4_table);
	if (repl == NULL)
		return -ENOMEM;
	ret = ipt_register_table(net, &cherlene_ipv4_table, repl,
				 NULL, &cherlene_table);
	if (ret < 0) {
		kfree(repl);
		return ret;
	}

	ret = cherlene_register_lookups(net);
	if (ret < 0) {
		ipt_unregister_table(net, cherlene_table, NULL);
		cherlene_table = NULL;
	}

	kfree(repl);
	return ret;
}

static void __net_exit cherlene_net_pre_exit(struct net *net)
{
	if (cherlene_table)
		cherlene_unregister_lookups(net);
}

static void __net_exit cherlene_net_exit(struct net *net)
{
	if (!cherlene_table)
		return;
	ipt_unregister_table(net, cherlene_table, NULL);
	cherlene_table = NULL;
}

static struct pernet_operations cherlene_net_ops = {
	.pre_exit = cherlene_net_pre_exit,
	.exit	= cherlene_net_exit,
};

static const char *dnat_table = NULL;

static int __init cherlene_init(void)
{
	int ret;
	struct xt_target *tgt;

	// Register 'cherlene' subsys
	ret = register_pernet_subsys(&cherlene_net_ops);
	if (ret)
		return ret;

	// Register 'cherlene' table and chains
	ret = cherlene_table_init(&init_net);
	if (ret) {
		unregister_pernet_subsys(&cherlene_net_ops);
		return ret;
	}

	// Tweak DNAT target to allow any table
	tgt = xt_request_find_target(AF_INET, "DNAT", 2);
	if (tgt) {
		dnat_table = tgt->table;
		tgt->table = NULL;
	}


#ifdef DEBUG
	printk(KERN_INFO "cherlene loaded\n");
#endif

	return ret;
}

static void __exit cherlene_exit(void)
{
	// Restore DNAT target if it was changed
	struct xt_target *tgt;

	tgt = xt_request_find_target(AF_INET, "DNAT", 2);
	if (dnat_table && tgt) {
		tgt->table = dnat_table;
	}

	unregister_pernet_subsys(&cherlene_net_ops);

#ifdef DEBUG
	printk(KERN_INFO "cherlene unloaded\n");
#endif
}

module_init(cherlene_init);
module_exit(cherlene_exit);

MODULE_LICENSE("GPL");
