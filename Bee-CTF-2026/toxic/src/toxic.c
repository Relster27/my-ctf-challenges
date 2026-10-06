#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/init.h>

#define TOXIC_MAX_OBJS  16
#define TOXIC_ALLOC     0x1337
#define TOXIC_FREE      0x1338
#define TOXIC_READ      0x1339
#define TOXIC_WRITE     0x133a

struct toxic_args {
        __u32 cmd;
        __u32 idx;
        __u64 buf;
        __u64 size;
};

struct toxic_obj {
        char data[32];
};

static struct toxic_obj *toxic_tbl[TOXIC_MAX_OBJS];
static struct kmem_cache *toxic_cache;

static int __init toxic_init(void)
{
        toxic_cache = kmem_cache_create_usercopy("toxic_obj",
                                        sizeof(struct toxic_obj),
                                        0,
                                        SLAB_HWCACHE_ALIGN,
                                        0,
                                        sizeof(struct toxic_obj),
                                        NULL);
        if (!toxic_cache) {
                pr_err("toxic: failed to create cache\n");
                return -ENOMEM;
        }
        pr_info("toxic: cache initialized\n");
        return 0;
}
early_initcall(toxic_init);

SYSCALL_DEFINE1(toxic, struct toxic_args __user *, uargs)
{
        struct toxic_args args;

        if (copy_from_user(&args, uargs, sizeof(args)))
                return -EFAULT;

        switch (args.cmd) {
                case TOXIC_ALLOC: {
                        int i;
                        for (i = 0; i < TOXIC_MAX_OBJS; i++) {
                                if (!toxic_tbl[i])
                                        break;
                        }
                        if (i >= TOXIC_MAX_OBJS)
                                return -ENOMEM;

                        toxic_tbl[i] = kmem_cache_alloc(toxic_cache, GFP_KERNEL);
                        if (!toxic_tbl[i])
                                return -ENOMEM;
                        pr_info("toxic: slot %d allocated\n", i);
                        return i;
                }

                case TOXIC_FREE: {
                        if (args.idx >= TOXIC_MAX_OBJS || !toxic_tbl[args.idx])
                                return -EINVAL;

                        kmem_cache_free(toxic_cache, toxic_tbl[args.idx]);
                        pr_info("toxic: slot %u freed\n", args.idx);
                        return 0;
                }

                case TOXIC_READ: {
                        if (args.idx >= TOXIC_MAX_OBJS || !toxic_tbl[args.idx])
                                return -EINVAL;
                        if (args.size > sizeof(struct toxic_obj))
                                return -EINVAL;

                        if (copy_to_user((void __user *)args.buf,
                                        toxic_tbl[args.idx]->data, args.size))
                                return -EFAULT;
                        pr_info("toxic: slot %u read\n", args.idx);
                        return 0;
                }

                case TOXIC_WRITE: {
                        if (args.idx >= TOXIC_MAX_OBJS || !toxic_tbl[args.idx])
                                return -EINVAL;
                        if (args.size > sizeof(struct toxic_obj))
                                return -EINVAL;

                        if (copy_from_user(toxic_tbl[args.idx]->data,
                                        (void __user *)args.buf, args.size))
                                return -EFAULT;
                        pr_info("toxic: slot %u written\n", args.idx);
                        return 0;
                }

                default:
                        return -EINVAL;
        }
}