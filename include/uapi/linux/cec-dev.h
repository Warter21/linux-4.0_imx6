#ifndef _UAPI_LINUX_CEC_DEV_H
#define _UAPI_LINUX_CEC_DEV_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define MAX_MESSAGE_LEN 16

enum {
	HDMICEC_IOC_MAGIC = 'H',
	/* This is wrong: we pass the argument as a number, not a pointer */
	HDMICEC_IOC_O_SETLOGICALADDRESS	= _IOW(HDMICEC_IOC_MAGIC, 1, unsigned char),
	HDMICEC_IOC_SETLOGICALADDRESS	= _IO(HDMICEC_IOC_MAGIC, 1),
	HDMICEC_IOC_STARTDEVICE		= _IO(HDMICEC_IOC_MAGIC, 2),
	HDMICEC_IOC_STOPDEVICE		= _IO(HDMICEC_IOC_MAGIC, 3),
	HDMICEC_IOC_GETPHYADDRESS	= _IOR(HDMICEC_IOC_MAGIC, 4, unsigned char[4]),
};

enum {
	MESSAGE_TYPE_RECEIVE_SUCCESS = 1,
	MESSAGE_TYPE_NOACK,
	MESSAGE_TYPE_DISCONNECTED,
	MESSAGE_TYPE_CONNECTED,
	MESSAGE_TYPE_SEND_SUCCESS,
	MESSAGE_TYPE_SEND_ERROR,
};

struct cec_user_event {
	__u32 event_type;
	__u32 msg_len;
	__u8 msg[MAX_MESSAGE_LEN];
};

#endif