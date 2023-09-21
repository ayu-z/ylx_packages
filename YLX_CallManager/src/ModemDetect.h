#ifndef __MODEMDETECT_H__
#define __MODEMDETECT_H__

enum ACTION_TYPE{
    ACTION_ADD,
    ACTION_REMOVE,
};

typedef struct {
    int action;
    int bus;
    int port;
    char *path;
    char *idProduct;
    char *idVendor;
    char *interface;
    char *driver;
    char *tty;
} modem_info_t;

typedef struct {
    int action;
    char *path;
    char *product;
    char *interface;
    char *driver;
    char *devname;
} modem_raw_info_t;


#define NETLINK_MAX_PAYLOAD 2048
#define MODEM_INFO_PATH "/tmp/modem.json"

void *hotplugEventDetect(void *arg);
int usbColdplugDetect();

int delCharFromString(char *str, const char *c);
int runShellCmd(const char *cmd, char *result, int resultSize);

int update_modem_json(modem_info_t modem);
int modem_raw_info_parse(modem_raw_info_t raw, modem_info_t *result);

#endif // !__MODEMDETECT_H__
