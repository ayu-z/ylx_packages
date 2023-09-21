#ifndef __COMMON_H__
#define __COMMON_H__

enum dialmode_type{
    GOBINET,
    QMI_WWAN,
    ACM,
    ECM,
    RNDIS,
    NCM,
    MBIM,
    PPP,
    ALL,
};

typedef struct {
    int mode;
    int at_idx;
}dialmode_t; 

typedef struct{
    int idProduct;
    int idVendor;

}modem_info_t;


#endif // !__COMMON_H__