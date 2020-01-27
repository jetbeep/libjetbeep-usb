#include <stdio.h>
#include <string.h>
#include "uart_drv.h"
#include "uart_slip.h"
#include "dfu.h"
#include "logging.h"


int main(int argc, char *argv[])
{
	int err_code = 0;
	//TODO implement zip and port search
	char *portName = NULL;
	uart_drv_t uart_drv;
	char *zipName = NULL;
	int argn;
	int info_lvl = LOGGER_INFO_LVL_0;


	logger_set_info_level(info_lvl);

	uart_drv.p_PortName = portName;

	if (!err_code)
	{
		err_code = uart_slip_open(&uart_drv);
	}

	if (!err_code)
	{
		dfu_param_t dfu_param;

		dfu_param.p_uart = &uart_drv;
		dfu_param.p_pkg_file = zipName;
		err_code = dfu_send_package(&dfu_param);
	}

	if (!err_code)
	{
		int err_code = uart_slip_close(&uart_drv);
	}

	return err_code;
}
