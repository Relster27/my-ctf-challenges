#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/cred.h>
#include <linux/ktime.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("relster");
MODULE_DESCRIPTION("bad bad ring buffer");

#define DEVICE_NAME "badring"

#define RING_CAPACITY 3
#define OBJECT_SIZE   1024
#define HEADER_SIZE   24
#define USER_SIZE     (OBJECT_SIZE - HEADER_SIZE)

#define CMD_INSERT  0xa5138
#define CMD_WRITE   0xa5139
#define CMD_CONSUME 0xa513a
#define CMD_RESET   0xa513b
#define CMD_UNDO    0xa513c

struct request {
        unsigned int idx;
        unsigned int data_len;
        char __user *buf;
};

struct ring_object {
        /* HEADER */
        unsigned long long timestamp_ns;
        unsigned int pid;
        unsigned int uid;
        unsigned long long user_len;

        /* USER */
        char user[USER_SIZE];
};

static_assert(offsetof(struct ring_object, user) == HEADER_SIZE);
static_assert(sizeof(struct ring_object) == OBJECT_SIZE);

struct ring_buffer {
        unsigned int head;
        unsigned int tail;
        unsigned int count;
        struct ring_object *slots[RING_CAPACITY];
};

static struct ring_buffer ring;
static struct ring_buffer undo_ring;
static DEFINE_MUTEX(badring_lock);

/* === Aux func(s) === */
static unsigned int ring_next(unsigned int pos)
{
        return (pos + 1) % RING_CAPACITY;
}

static unsigned int ring_slot(unsigned int idx)
{
        return (ring.head + idx) % RING_CAPACITY;
}

static void ring_flush(const struct ring_object *obj)
{
        pr_info("badring: timestamp_ns=%llu pid=%u uid=%u "
                "len=%llu msg=\"%*pE\"\n",
                obj->timestamp_ns, obj->pid, obj->uid,
                obj->user_len, (int)obj->user_len, obj->user);
}

/* ================= */

/* === Main func(s) === */
static long ring_insert(void)
{
        struct ring_object *obj = kzalloc(sizeof(*obj), GFP_KERNEL);
        if (!obj) {
                return -ENOMEM;
        }

        if (ring.count == RING_CAPACITY) {
                ring_flush(ring.slots[ring.head]);
                kfree(ring.slots[ring.head]);
                ring.slots[ring.head] = NULL;
                ring.head = ring_next(ring.head);
                ring.count--;
        }

        ring.slots[ring.tail] = obj;
        ring.tail = ring_next(ring.tail);
        ring.count++;

        return 0;
}

static long ring_write(struct request *req)
{
        if (req->data_len >= USER_SIZE) {
                return -EINVAL;
        }

        if (req->data_len != 0 && !req->buf) {
                return -EFAULT;
        }

        if (req->idx >= ring.count) {
                return -EINVAL;
        }

        unsigned int slot = ring_slot(req->idx);
        struct ring_object *obj = ring.slots[slot];
        if (!obj) {
                return -EINVAL;
        }

        if (copy_from_user(obj->user, req->buf, req->data_len)) {
                return -EFAULT;
        }
        obj->user[req->data_len] = '\0';

        obj->timestamp_ns = ktime_get_real_ns();
        obj->pid = (unsigned int)task_pid_nr(current);
        obj->uid = (unsigned int)from_kuid_munged(current_user_ns(), current_uid());
        obj->user_len = req->data_len;

        return 0;
}

static long ring_consume(void)
{
        if (ring.count == 0) {
                return -ENOENT;
        }
        
        struct ring_object *obj = ring.slots[ring.head];
        if (!obj) {
                return -ENOENT;
        }

        ring_flush(obj);
        kfree(obj);

        ring.slots[ring.head] = NULL;
        ring.head = ring_next(ring.head);
        ring.count--;

        return 0;
}

static long ring_reset(void)
{
        if (ring.count == 0) {
                return -ENOENT;
        }

        unsigned int old_count = ring.count;
        undo_ring = ring;

        if (ring.head == 0) {
                for (unsigned int i = 0; i < old_count; i++) {
                        unsigned int slot = ring_slot(i);
                        if (ring.slots[slot]) {
                                ring_flush(ring.slots[slot]);
                                ring.slots[slot] = NULL;
                                ring.count--;
                        }
                }
                ring.tail = ring.head;
                return 0;
        }

        for (unsigned int i = 0; i < old_count; i++) {
                unsigned int slot = i;
                if (ring.slots[slot]) {
                        ring_flush(ring.slots[slot]);
                        kfree(ring.slots[slot]);
                        ring.slots[slot] = NULL;
                        ring.count--;
                }
        }
        ring.tail = ring.head;

        return 0;
}

static long ring_undo(void)
{
        if (undo_ring.count == 0) {
                return -ENOENT;
        }

        ring = undo_ring;
        memset(&undo_ring, 0, sizeof(undo_ring));

        return 0;
}
/* ==================== */

static long badring_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct request req;
        long ret;

        mutex_lock(&badring_lock);

        switch (cmd) {
                case CMD_INSERT:
                        ret = ring_insert();
                        break;

                case CMD_WRITE:
                        if (copy_from_user(&req, (void __user *)arg, sizeof(req))) {
                                ret = -EFAULT;
                                break;
                        }

                        ret = ring_write(&req);
                        break;

                case CMD_CONSUME:
                        ret = ring_consume();
                        break;

                case CMD_RESET:
                        ret = ring_reset();
                        break;

                case CMD_UNDO:
                        ret = ring_undo();
                        break;

                default:
                        ret = -EINVAL;
                        break;
        }

        mutex_unlock(&badring_lock);

        return ret;
}

static const struct file_operations badring_fops = {
        .owner          = THIS_MODULE,
        .unlocked_ioctl = badring_ioctl,
};

static struct miscdevice badring_device = {
        .minor = MISC_DYNAMIC_MINOR,
        .name  = DEVICE_NAME,
        .fops  = &badring_fops,
};

static int __init badring_init(void)
{
        int ret;

        memset(&ring, 0x0, sizeof(ring));
        memset(&undo_ring, 0, sizeof(undo_ring));

        ret = misc_register(&badring_device);
        if (ret) {
                pr_err("badring: failed to register device: %d\n", ret);
                return ret;
        }

        pr_info("badring: module initialized\n");
        return 0;
}

static void __exit badring_exit(void)
{
        misc_deregister(&badring_device);
        pr_info("badring: module exited\n");
}

module_init(badring_init);
module_exit(badring_exit);
