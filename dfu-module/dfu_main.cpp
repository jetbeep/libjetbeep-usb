/*
 * nrfdfu - Nordic DFU Upgrade Utility
 *
 * Copyright (C) 2019 Bruno Randolf (br1@einfach.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include <zip.h>
#include <json-c/json.h>

#include "nrfdfu/conf.h"
#include "nrfdfu/log.h"
#include "nrfdfu/serialtty.h"
#include "nrfdfu/dfu.h"
#include "nrfdfu/dfuserial.h"

struct config conf;
int ser_fd = -1;


static zip_file_t* zip_file_open(zip_t* zip, const char* name, size_t* size)
{
	struct zip_stat stat;

	zip_stat_init(&stat);
	int ret = zip_stat(zip, name, 0, &stat);
	if (ret < 0) {
		//LOG_ERR("ZIP file does not contain %s", name);
		return NULL;
	}
	*size = stat.size;

	zip_file_t* zf = zip_fopen_index(zip, stat.index, 0);
	if (zf == NULL) {
		//LOG_ERR("Error opening %s in ZIP file", name);
		return NULL;
	}
	return zf;
}

/* dat and bin have to be freed by caller */
static int read_manifest(zip_t* zip, char** dat, char** bin)
{
	char buf[200];

	zip_file_t* zf = zip_fopen(zip, "manifest.json", 0);
	if (zf == NULL) {
		//LOG_ERR("ZIP file does not contain manifest");
		return -1;
	}

	zip_int64_t len = zip_fread(zf, buf, sizeof(buf));
	if (len <= 0) {
		//LOG_ERR("Could not read Manifest");
		return -1;
	}

	/* read JSON */
	json_object* json;
	json_object* jobj;
	json_object* jobj2;
	json = json_tokener_parse(buf);
	if (json == NULL) {
		//LOG_ERR("Manifest not valid JSON");
		zip_fclose(zf);
		return -1;
	}

	if (json_object_object_get_ex(json, "manifest", &jobj) &&
	    json_object_object_get_ex(jobj, "application", &jobj2)) {
		if (json_object_object_get_ex(jobj2, "dat_file", &jobj)) {
			*dat = strdup(json_object_get_string(jobj));
		}
		if (json_object_object_get_ex(jobj2, "bin_file", &jobj)) {
			*bin = strdup(json_object_get_string(jobj));
		}
	}
	json_object_put(json);
	zip_fclose(zf);

	if (!*dat || !*bin) {
		//LOG_ERR("Manifest format unknown");
		return -1;
	}

	return 0;
}

static bool enter_dfu_cmd(void)
{
	char b[200];

	/* first read and discard anything that came before */
	read(ser_fd, b, 200);

	//LOG_INF("Sending command to enter DFU mode: '%s'", conf.dfucmd);
	/* it looks like the first two characters written are lost...
	 * and we need \r to enter CLI */
	serial_write(ser_fd, "\r\r\r", 3, 1);
	serial_write(ser_fd, conf.dfucmd, strlen(conf.dfucmd), 1);
	serial_write(ser_fd, "\r", 1, 1);
	sleep(1);

	int ret = read(ser_fd, b, 200);
	if (ret > 0) {
		/* debug output reply */
		b[ret--] = '\0';
		/* remove trailing \r \n */
		while (b[ret] == '\r' || b[ret] == '\n') {
			b[ret--] = '\0';
		}
		/* remove \r \n and zero from the beginning */
		ret = 0;
		while (b[ret] == '\r' || b[ret] == '\n' || b[ret] == '\0'
		       && ret < sizeof(b)) {
			ret++;
		}
		//LOG_INF("Device replied: '%s' (%d)", b + ret, ret);
		return true;
	} else {
		//LOG_INF("Device didn't repy (%d)", ret);
		return false;
	}
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	char* dat = NULL;
	char* bin = NULL;
	zip_file_t* zf1 = NULL;
	zip_file_t* zf2 = NULL;
	size_t zs1, zs2;
    int try_ = 0;

	//LOG_INF("Serial Port: %s", conf.serport);
	//LOG_INF("DFU Package: %s", conf.zipfile);

	zip_t* zip = zip_open(conf.zipfile, ZIP_RDONLY, NULL);
	if (zip == NULL) {
		//LOG_ERR("Could not open ZIP file '%s'", conf.zipfile);
		goto exit;
	}

	ret = read_manifest(zip, &dat, &bin);
	if (ret < 0 || !dat || !bin) {
		goto exit;
	}

	/* open data files in ZIP file */
	zf1 = zip_file_open(zip, dat, &zs1);
	zf2 = zip_file_open(zip, bin, &zs2);
	if (zf1 == NULL || zf2 == NULL) {
		goto exit;
	}

	ser_fd = serial_init(conf.serport);
	if (ser_fd <= 0) {
		goto exit;
	}

	/* first check if Bootloader responds to Ping */
	//LOG_NOTI_("Waiting for device to be ready: ");
	
	do {
		if (conf.dfucmd) {
			ret = enter_dfu_cmd();
			sleep(1);
			if (!ret) {
				/* if dfu command failed, try ping, it will
				 * usually fail with "Opcode not supported"
				 * because of the text we sent before, but then
				 * the next one below can succeed */
				ret = dfu_ping();
			}
		} else {
			sleep(1);
		}

		ret = dfu_ping();
	} while (!ret && ++try_ < conf.timeout);

	//LOG_NL(LL_NOTICE);

	if (try_ >= conf.timeout) {
		//LOG_NOTI("Device didn't respond after %d tries", conf.timeout);
		ret = EXIT_FAILURE;
		goto exit;
	}

	//LOG_NOTI("Starting DFU upgrade");

	/* Upgrade process */
	if (!dfu_set_packet_receive_notification(0))
		goto exit;

	if (!dfu_get_serial_mtu())
		goto exit;

	//LOG_NOTI_("Sending Init: ");
	if (!dfu_object_write_procedure(1, zf1, zs1))
		goto exit;
	//LOG_NL(LL_NOTICE);

	//LOG_NOTI_("Sending Firmware: ");
	if (!dfu_object_write_procedure(2, zf2, zs2))
		goto exit;
	//LOG_NL(LL_NOTICE);
	//LOG_NOTI("Done");

	ret = EXIT_SUCCESS;

exit:
	free(bin);
	free(dat);
	if (zf1) zip_fclose(zf1);
	if (zf2) zip_fclose(zf2);
	if (zip) zip_close(zip);
	serial_fini(ser_fd);
	return ret;
}
