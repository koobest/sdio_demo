#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sip.h"
#include "sip2_common.h"

#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))

#define SIF_TO_DEVICE     0x1
#define SIF_FROM_DEVICE   0x2
#define SIF_SYNC          0x00000010
#define SIF_ASYNC         0x00000020
#define SIF_BYTE_BASIS    0x00000040
#define SIF_BLOCK_BASIS   0x00000080
#define SIF_FIXED_ADDR    0x00000100
#define SIF_INC_ADDR      0x00000200

enum io_sync_type { 
    ESP_SIF_NOSYNC = 0,
    ESP_SIF_SYNC, 
};


static uint32_t txseq = 0;
static uint32_t rxseq = 0;
static uint32_t txdata_seq = 0;

essl_handle_t essl_handle = NULL;

void essl_set_handle(essl_handle_t _handle)
{
    essl_handle = _handle;
}

int sif_io_sync(uint32_t addr, uint8_t *buf, int len, uint32_t flag)
{
    int ret = 0;
    if (flag & SIF_TO_DEVICE) {
        if (flag & SIF_FIXED_ADDR) {
            // sdio_writesb(addr, buf, len);
            ret = essl_send_packet(essl_handle, (const void *)buf, len, 50);
        } else if (flag & SIF_INC_ADDR) {
            // sdio_memcpy_toio(addr, buf, len);
            ret = essl_send_packet(essl_handle, (const void *)buf, len, 50);
        }
    } else if(flag & SIF_FROM_DEVICE) {
        if (flag & SIF_FIXED_ADDR) {
            ret = essl_send_packet(essl_handle, (const void *)buf, len, 50);
            // sdio_readsb(buf, addr, len);
        } else if (flag & SIF_INC_ADDR) {
            // sdio_memcpy_fromio(buf, addr, len);
            ret = essl_send_packet(essl_handle, (const void *)buf, len, 50);
        }
    }
    return ret;
}

int sif_lldesc_write_sync(uint8_t *buf, uint32_t len)
{
    uint32_t write_len = 0;
    int target_id = 0;
    uint32_t end_addr = 0x20000;
    if (target_id == 0x100) {
        write_len = len;
    } else if (target_id == 0x600) {
        write_len = roundup(len, SIF_SLC_BLOCK_SIZE);
    } else {
        write_len = len;
    }
    return sif_io_sync((end_addr - len), (buf), (write_len), (SIF_TO_DEVICE | SIF_BYTE_BASIS | SIF_INC_ADDR));
}

int esp_comm_write(uint8_t *buf, uint32_t len, int sync)
{
    return sif_lldesc_write_sync(buf, len);
}


int sip_write_mem(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t *src = NULL;
    struct sip_hdr *chdr;
    uint32_t load_addr = 0;
    uint16_t hdrs, buf_size, rem;
    struct sip_cmd_write_memory *cmd;
    uint8_t *buf_temp = (uint8_t *)malloc(SIP_BOOT_BUF_SIZE);
    int err = -1;
    memset(buf_temp, 0, SIP_BOOT_BUF_SIZE);

    chdr = (struct sip_hdr *)buf_temp;
    SIP_HDR_SET_TYPE(chdr->fc[0], SIP_CTRL);
    chdr->c_cmdid = SIP_CMD_WRITE_MEMORY;
    rem = len;
    hdrs = sizeof(struct sip_hdr) + sizeof(struct sip_cmd_write_memory);
    while (rem) {
        src = &buf[len - rem];
        load_addr = addr + (len -  rem);
        if (rem < (SIP_BOOT_BUF_SIZE - hdrs)) {
            buf_size = roundup(rem, 4);
            memset(buf_temp + hdrs, 0, buf_size);
            rem = 0;
        } else {
            buf_size = SIP_BOOT_BUF_SIZE - hdrs;
            rem -= buf_size;
        }
        chdr->len = buf_size + hdrs;
        chdr->seq = txseq++;
        cmd = (struct sip_cmd_write_memory *)(buf_temp + SIP_CTRL_HDR_LEN);
        cmd->len = buf_size;
        cmd->addr = load_addr;
        memcpy(buf_temp +hdrs, src, buf_size);
        // uint32_t *t = (uint32_t *)buf_temp;
        // printf("-- buf size %d\n", chdr->len);
        err = esp_comm_write(buf_temp, chdr->len, ESP_SIF_SYNC);
        if (err) {
            printf("err com write ret%x\n", err);
            break;
        }
        ets_delay_us(1000);
    }
    free(buf_temp);
    return err;
}

int sip_send_cmd(int cid, uint32_t cmd_len, void *cmd)
{
    int ret = -1;
    struct sip_hdr *chdr;
    int cmd_pkt_size = cmd_len + SIP_CTRL_HDR_LEN;
    uint8_t *pkt = malloc(cmd_pkt_size);
    memset(pkt, 0, cmd_pkt_size);
    memcpy(pkt + SIP_CTRL_HDR_LEN, cmd, cmd_len);

    chdr = (struct sip_hdr *)pkt;
    SIP_HDR_SET_TYPE(chdr->fc[0], SIP_CTRL);
    chdr->len = cmd_pkt_size;
    chdr->seq = txseq++;
    chdr->c_cmdid = cid;
    ret = esp_comm_write(pkt, chdr->len, ESP_SIF_SYNC);
    free(pkt);
    if (cid == SIP_CMD_BOOTUP) {
        txseq = 0;
        rxseq = 0;
        txdata_seq = 0;
    }
    return ret;
}

int sip_send_bootup(void)
{
    int ret = -1;
    struct sip_cmd_bootup bootcmd;
    bootcmd.boot_addr = 0x40100000;
    ret = sip_send_cmd(SIP_CMD_BOOTUP, sizeof(struct sip_cmd_bootup), &bootcmd);
    if(ret) {
        printf("cmd tx err %x\n", ret);
    }
    return ret;
}