#ifndef __SIP_H__
#define __SIP_H__

#include "esp32/rom/ets_sys.h"
#include "esp_serial_slave_link/essl_sdio.h"
#define SIF_SLC_BLOCK_SIZE (512)


void essl_set_handle(essl_handle_t _handle);

int sif_io_sync(uint32_t addr, uint8_t *buf, int len, uint32_t flag);

int sif_lldesc_write_sync(uint8_t *buf, uint32_t len);

int esp_comm_write(uint8_t *buf, uint32_t len, int sync);

int sip_write_mem(uint32_t addr, uint8_t *buf, uint16_t len);

int sip_send_bootup(void);

#endif