#include "ModemDetect.h"
#include "cJSON.h"
// #include <cjson/cJSON.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <libusb-1.0/libusb.h>

modem_raw_info_t modem_raw;


void timeout_handler(int signum) {
    modem_info_t modem_info;
    memset(&modem_info, 0, sizeof(modem_info_t));
    int parseResult = modem_raw_info_parse(modem_raw, &modem_info);

    if (parseResult == 0) {
        update_modem_json(modem_info);
    } else {
        printf("Parsing failed.\n");
    }
    memset(&modem_raw, 0, sizeof(modem_raw));
}


void *hotplugEventDetect(void *arg) 
{
    signal(SIGALRM, timeout_handler);
    
    memset(&modem_raw, 0, sizeof(modem_raw));

    struct sockaddr_nl sa;
    int sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (sock_fd == -1) {  
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = NETLINK_KOBJECT_UEVENT;

    if (bind(sock_fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        perror("bind");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        char buffer[NETLINK_MAX_PAYLOAD];
        memset(buffer, 0, sizeof(buffer));

        if (recv(sock_fd, &buffer, sizeof(buffer), 0) == -1) {
            perror("recv");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }

        int found_action_add = 0;
        int found_action_remove = 0;
        int found_action_unbind = 0;
        int found_subsystem_usb = 0;
        int found_subsystem_net = 0;
        int found_subsystem_tty = 0;
        int found_devtype_dev = 0;
        int found_devtype_if = 0;
        char *devpath = NULL;
        char *product = NULL;
        char *interface = NULL;
        char *driver = NULL;
        char *devname = NULL;
        char tmp[256] = {};

        for (int i = 0; i < sizeof(buffer) / sizeof(buffer[0]); i++) {
            if (strstr(&buffer[i], "ACTION=add")) {
                found_action_add = 1;
            } else if (strstr(&buffer[i], "ACTION=remove")) {
                found_action_remove = 1;
            } else if (strstr(&buffer[i], "ACTION=unbind")) {
                found_action_unbind = 1;
            } else if (strstr(&buffer[i], "SUBSYSTEM=usb")) {
                found_subsystem_usb = 1;
            } else if (strstr(&buffer[i], "DEVTYPE=usb_device")) {
                found_devtype_dev = 1;
            } else if (strstr(&buffer[i], "PRODUCT=")) {
                product = strstr(&buffer[i], "PRODUCT=");
            } else if (strstr(&buffer[i], "DEVPATH=")) {
                devpath = strstr(&buffer[i], "DEVPATH=");
            }if (strstr(&buffer[i], "SUBSYSTEM=net")) {
                found_subsystem_net = 1;
            } else if (strstr(&buffer[i], "INTERFACE=")) {
                interface = strstr(&buffer[i], "INTERFACE=");
            }else if (strstr(&buffer[i], "DEVTYPE=usb_interface")) {
                found_devtype_if = 1;
            }else if (strstr(&buffer[i], "DRIVER=")) {
                driver = strstr(&buffer[i], "DRIVER=");
            }else if (strstr(&buffer[i], "SUBSYSTEM=tty")) {
                found_subsystem_tty = 1;
            }else if (strstr(&buffer[i], "DEVNAME=")) {
                devname = strstr(&buffer[i], "DEVNAME=");
            }
        }

        alarm(1);

        if (found_action_add && found_subsystem_usb && found_devtype_dev && product && devpath) {
            modem_raw.action = ACTION_ADD;
            modem_raw.path = strdup(devpath);
            modem_raw.product = strdup(product);
        } else if(found_action_add && found_subsystem_net && interface){
            modem_raw.action = ACTION_ADD;
            modem_raw.interface = strdup(interface);
        }else if(found_action_add && found_subsystem_usb && found_devtype_if && driver){
            modem_raw.action = ACTION_ADD;
            modem_raw.driver = strdup(driver);
        }else if(found_action_add && found_subsystem_tty && devname){
            modem_raw.action = ACTION_ADD;
            char *tmp = modem_raw.devname;

            if (tmp == NULL || strlen(tmp) == 0) {
                tmp = strdup(devname); 
            } else {
                char *result = (char *)malloc(strlen(tmp) + strlen(devname) + 2);
                if (result == NULL) {
                    return NULL;
                }
                strcpy(result, tmp);
                strcat(result, "-");
                strcat(result, devname);
                tmp = result;
            }

            if (modem_raw.devname != NULL) {
                free(modem_raw.devname); 
            }

        modem_raw.devname = tmp;
        }else if (found_action_remove && found_subsystem_net && interface){
            modem_raw.action = ACTION_REMOVE;
            modem_raw.interface = strdup(interface);
        }else if (found_action_unbind && found_subsystem_usb && found_devtype_dev && product && devpath) {
            modem_raw.action = ACTION_REMOVE;
            modem_raw.path = strdup(devpath);
            modem_raw.product = strdup(product);
        }
    }

    close(sock_fd);
    return NULL;
}


int usbColdplugDetect()
{
    libusb_context *ctx = NULL;
    modem_info_t modem;
    char have_device = 0;
    char path[512];
    char cmd[256];

    if (libusb_init(&ctx) < 0) {
        fprintf(stderr, "Unable to initialize libusb\n");
        return 1;
    }

    libusb_device **devs;
    ssize_t cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        fprintf(stderr, "The list of USB devices cannot be obtained\n");
        libusb_exit(ctx);
        return 1;
    }

    for (ssize_t i = 0; i < cnt; i++) {
        libusb_device *dev = devs[i];
        struct libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc) < 0) {
            fprintf(stderr, "Could not get the device descriptor\n");
            continue;
        }

        // if(desc.idVendor == 0x2c7c&& desc.idProduct == 0x0900){
        if(desc.idVendor == 0x2c7c && desc.idVendor == 0x2dee){
            char base[256] = {};

            modem.action = ACTION_ADD;

            sprintf(base, "%x", desc.idProduct);
            modem.idProduct = strdup(base);
            sprintf(base, "%x", desc.idVendor);
            modem.idVendor = strdup(base);

            modem.bus = libusb_get_bus_number(dev);
            modem.port = libusb_get_port_number(dev);
            sprintf(cmd,"find /sys/devices/platform |grep usb%d/%d-%d/idProduct", libusb_get_bus_number(dev), libusb_get_bus_number(dev), libusb_get_port_number(dev));       
            runShellCmd(cmd, path, sizeof(path));

            
            char * offset=strrchr(path, '/');
            if (offset != NULL) {
                strncpy(base, path, (offset - path));
            }
            modem.path = strdup(base);
            sprintf(cmd, "find %s -maxdepth 2 | grep tty | sort", modem.path);
            runShellCmd(cmd, path, sizeof(path));
   
            char *line, *result;
            char *delimiter = "\n";

            base[0] = '\0';
            line = strtok(path, delimiter);
            while (line != NULL) {
                result = strstr(line, "ttyUSB");
                if (result != NULL) {
                    strcat(base, result);
                    strcat(base, "-");
                }
                line = strtok(NULL, delimiter);
            }

            if (strlen(base) > 0) {
                base[strlen(base) - 1] = '\0';
            }

            modem.tty = strdup(base);
            sprintf(cmd, "find %s -maxdepth 3 | grep net/ | sort", modem.path);
            runShellCmd(cmd, base, sizeof(base));
            if (strlen(base) > 0) {
                base[strlen(base) - 1] = '\0';
            }
            modem.interface = strdup(strrchr(base, '/')+1);

            update_modem_json(modem);
        }
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);

    return 0;
}


int update_modem_json(modem_info_t modem)
{
    const char *filename = MODEM_INFO_PATH;
    FILE *file = fopen(filename, "a+");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file\n");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize == 0) {
        cJSON *root = cJSON_CreateObject();
        if (root == NULL) {
            fclose(file);
            fprintf(stderr, "Error creating JSON object\n");
            return 1;
        }

        cJSON_AddItemToObject(root, "modem", cJSON_CreateArray());

        char *json_str = cJSON_Print(root);
        cJSON_Delete(root);
        fileSize = strlen(json_str);
        fclose(file);
        file = fopen(filename, "w");
        fprintf(file, "%s\n", json_str);
        free(json_str);
        fclose(file);

        file = fopen(filename, "a+");
        if (file == NULL) {
            fprintf(stderr, "Unable to open file\n");
            return 1;
        }

        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
    }

    char *fileContent = (char *)malloc(fileSize + 1);
    if (fileContent == NULL) {
        fprintf(stderr, "Memory error\n");
        fclose(file);
        return 1;
    }

    size_t bytesRead = fread(fileContent, 1, fileSize, file);
    fclose(file);

    if (bytesRead != fileSize) {
        fprintf(stderr, "File reading failure\n");
        free(fileContent);
        return 1;
    }

    fileContent[fileSize] = '\0'; 
    cJSON *root = cJSON_Parse(fileContent);
    if (root == NULL) {
        const char *error = cJSON_GetErrorPtr();
        if (error != NULL) {
            fprintf(stderr, "JSON parse error: %s\n", error);
        }
        free(fileContent);
        return 1;
    }

    cJSON *modemArray = cJSON_GetObjectItem(root, "modem");
    if (modemArray == NULL || !cJSON_IsArray(modemArray)) {
        fprintf(stderr, "Invalid JSON data\n");
        cJSON_Delete(root);
        free(fileContent);
        return 1;
    }

    int i = cJSON_GetArraySize(modemArray) - 1;
    while (i >= 0) {
        cJSON *element = cJSON_GetArrayItem(modemArray, i);
        cJSON *usb_node = cJSON_GetObjectItem(element, "usb");

        cJSON *busItem = cJSON_GetObjectItem(usb_node, "bus");
        cJSON *portItem = cJSON_GetObjectItem(usb_node, "port");

        if (busItem != NULL && portItem != NULL && cJSON_IsNumber(busItem) && cJSON_IsNumber(portItem)) {
            if (busItem->valueint == modem.bus && portItem->valueint == modem.port) {
                cJSON_DeleteItemFromArray(modemArray, i);
            }
        }
        i--;
    }

    if (modem.action==ACTION_ADD&&modem.path&&modem.idProduct&&modem.interface&&modem.idVendor&&modem.tty){
        cJSON *newModem = cJSON_CreateObject();
        cJSON *usbObject = cJSON_CreateObject();
        cJSON *ttyObject = cJSON_CreateObject();

        cJSON_AddStringToObject(usbObject, "path", modem.path);
        cJSON_AddNumberToObject(usbObject, "bus", modem.bus);
        cJSON_AddNumberToObject(usbObject, "port", modem.port);
        cJSON_AddStringToObject(newModem, "idProduct", modem.idProduct);
        cJSON_AddStringToObject(newModem, "idVendor", modem.idVendor);
        cJSON_AddStringToObject(newModem, "interface", modem.interface);
        cJSON_AddStringToObject(newModem, "driver", modem.driver);
        
        char index = 0;
        while (1)
        {   
            char tty[9];
            modem.tty = strstr(modem.tty, "ttyUSB");
            if (modem.tty == NULL){
                break;
            }
            
            if (sscanf(modem.tty, "%8[^-]", tty) == 1) {
                char c[3];
                sprintf(c, "%d", index);
                cJSON_AddStringToObject(ttyObject, c, tty);
                index++;
            }

            modem.tty = strchr(modem.tty, '-');
            if (modem.tty == NULL) {
                break; 
            }
            modem.tty++; 
        }
        
        cJSON_AddItemToObject(newModem, "usb", usbObject);
        cJSON_AddItemToObject(newModem, "tty", ttyObject);
        cJSON_AddItemToArray(modemArray, newModem);
    }
        
    FILE *outputFile = fopen(filename, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Unable to open file for writing\n");
        cJSON_Delete(root);
        free(fileContent);
        return 1;
    }

    char *updatedJsonStr = cJSON_Print(root);
    fputs(updatedJsonStr, outputFile);
    fclose(outputFile);
    cJSON_Delete(root);
    free(fileContent);
    free(updatedJsonStr);

    return 0;
}

int modem_raw_info_parse(modem_raw_info_t raw, modem_info_t *result)
{

    char idProduct[5];
    char idVendor[5];
    memset(result, 0, sizeof(modem_info_t));

    if(raw.path == NULL && raw.product == NULL && raw.interface == NULL) {
        return 1;
    }

    result->path = raw.path + strlen("DEVPATH=");
    if (sscanf(strrchr(raw.path, '/'), "/%d-%d", &result->bus, &result->port) != 2 || result->path == NULL) {
        return 1;
    }

    if (sscanf(raw.product, "PRODUCT=%4[^/]/%4[^/]", idProduct, idVendor) == 2) {
        result->idProduct = strdup(idProduct);
        result->idVendor = strdup(idVendor);
    }

    result->interface = raw.interface + strlen("INTERFACE=");
    if(raw.action == ACTION_ADD) {
        if(raw.devname != NULL && raw.driver != NULL){
            result->driver = raw.driver + strlen("DRIVER=");
            char *buffer = strdup(raw.devname);
            delCharFromString(buffer, "DEVNAME=");
            result->tty = buffer;
        }else {
            return 1;
        }
    }
    return 0;
}

int delCharFromString(char *str, const char *c) {

    if (str == NULL || c == NULL) return 0;

    size_t cLen = strlen(c);
    char *src = str;
    char *dest = str;

    while (*src) {
        if (strncmp(src, c, cLen) == 0) {
            src += cLen; 
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0'; 

    return 0;
}

int runShellCmd(const char *cmd, char *result, int resultSize)
{
    char buf[256];
    int len = 0;

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("%s run error!\n", cmd);
        return 0;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        int lineLen = strlen(buf);
        if (len + lineLen < resultSize) {
            strcpy(result + len, buf);
            len += lineLen;
        } 
    }

    pclose(fp);

    if (len < resultSize) {
        result[len] = '\0';
    }
    return len;
}