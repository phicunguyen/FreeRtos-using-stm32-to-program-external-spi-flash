/**
 * @file inv_hal_serial_api.h
 *
 * @brief register access APIs
 *
 *****************************************************************************/

#ifndef INV_HAL_SERIAL_API_H
#define INV_HAL_SERIAL_API_H
#include <stdio.h>
#include <conio.h>

 /* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/***** #include statements ***************************************************/
struct AdapterTypedef_t;

/***** typedef  ***************************************************/
enum {
	SERIAL_PACKET__NONE,
	SERIAL_PACKET__FLASH_WRITE,
	SERIAL_PACKET__FLASH_READ,
	SERIAL_PACKET__LED,
};
typedef void(*serial_mon)(struct AdapterTypedef_t *p_adapter);
typedef bool(*serial_connect)(struct AdapterTypedef_t *p_adapter);
typedef void(*user_callback)(uint8_t *pbuf, uint16_t *plen);
typedef void(*serial_trx)(struct AdapterTypedef_t *p_adapter, uint8_t *p_buf, uint32_t size);
typedef void(*serial_flash)(struct AdapterTypedef_t *p_adapter, uint8_t *p_buf, uint32_t size, uint32_t addr);
/***** local type definitions ************************************************/

struct AdapterTypedef_t {
	char							*p_name;
	serial_mon						ser_mon;
	serial_connect					is_port_open;
	user_callback					callback;
	serial_trx						write;
	serial_trx						read;
	serial_flash					flash;
	int32_t 						Handle;
	unsigned int					port_id_sel;
	bool							port_open;
	uint8_t							buffer[512];
	uint16_t						bufindx;
};

/***** public functions ******************************************************/

bool serial_devices_mon(void);
void serial_devices_get(char **p_connection[]);
void serial_devices_create(struct AdapterTypedef_t *p_adapter, user_callback callback, char *portname);
bool serial_devices_open(struct AdapterTypedef_t *p_adapter);
#endif //SI_HAL_SERIAL_API_H
