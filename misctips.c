/*
 * Copyright 2007, Intel Corporation
 *
 * This file is part of PowerTOP
 *
 * This program file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program in a file named COPYING; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * Authors:
 * 	Arjan van de Ven <arjan@linux.intel.com>
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>

#include "powertop.h"

void set_laptop_mode(void)
{
	FILE *file;
	file = fopen("/proc/sys/vm/laptop_mode", "w");
	if (!file)
		return;
	fprintf(file,"5\n");
	fclose(file);
}

void suggest_laptop_mode(void)
{
	FILE *file;
	int i;
	char buffer[1024];
	/*
	 * Check to see if we are on AC - lots of distros have
	 * annoying scripts to turn laptop mode off when on AC, which
	 * results in annoying distracting return of set laptop mode
	 * hint.
	 */
	file = fopen("/proc/acpi/ac_adapter/AC/state", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	fclose(file);
	if (strstr(buffer, "on-line") != NULL)
		return;

	/* Now check for laptop mode */
	file = fopen("/proc/sys/vm/laptop_mode", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	i = strtoul(buffer, NULL, 10);
	if (i<1) {
		add_suggestion( _("Suggestion: Enable laptop-mode by executing the following command:\n"
		 	"   echo 5 > /proc/sys/vm/laptop_mode \n"), 15, 'L', _(" L - enable Laptop mode "), set_laptop_mode);
	}
	fclose(file);
}

void nmi_watchdog_off(void)
{
	FILE *file;
	file = fopen("/proc/sys/kernel/nmi_watchdog", "w");
	if (!file)
		return;
	fprintf(file,"0\n");
	fclose(file);
}
void suggest_nmi_watchdog(void)
{
	FILE *file;
	int i;
	char buffer[1024];
	file = fopen("/proc/sys/kernel/nmi_watchdog", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	i = strtoul(buffer, NULL, 10);
	if (i!=0) {
		add_suggestion( _("Suggestion: disable the NMI watchdog by executing the following command:\n"
		 	"   echo 0 > /proc/sys/kernel/nmi_watchdog \n"
			"The NMI watchdog is a kernel debug mechanism to detect deadlocks"), 25, 'N', _(" N - Turn NMI watchdog off "), nmi_watchdog_off);
	}
	fclose(file);
}

void suggest_hpet(void)
{
	FILE *file;
	char buffer[1024];
	file = fopen("/sys/devices/system/clocksource/clocksource0/available_clocksource", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	
	if (strstr(buffer, "hpet")) {
		fclose(file);
		return;
	}

	fclose(file);

	add_suggestion( _("Suggestion: enable the HPET (Multimedia Timer) in your BIOS or add \n"
		          "the kernel patch to force-enable HPET. HPET support allows Linux to \n"
			  "have much longer sleep intervals."), 7, 0, NULL, NULL);
}

void ac97_power_on(void)
{
	FILE *file;
	file = fopen("/sys/module/snd_ac97_codec/parameters/power_save", "w");
	if (!file)
		return;
	fprintf(file,"1");
	fclose(file);
	file = fopen("/dev/dsp", "w");
	if (file) {
		fprintf(file,"1");
		fclose(file);
	}
}

void suggest_ac97_powersave(void)
{
	FILE *file;
	char buffer[1024];
	file = fopen("/sys/module/snd_ac97_codec/parameters/power_save", "r");
	if (!file)
		return;
	memset(buffer, 0, 1024);
	if (!fgets(buffer, 1023, file)) {
		fclose(file);
		return;
	}
	if (buffer[0]=='N') {
		add_suggestion( _("Suggestion: enable AC97 powersave mode by executing the following command:\n"
		 	"   echo 1 > /sys/module/snd_ac97_codec/parameters/power_save \n"
			"or by passing power_save=1 as module parameter."), 25, 'A', _(" A - Turn AC97 powersave on "), ac97_power_on);
	}
	fclose(file);
}

void noatime_on(void)
{
	system("/bin/mount -o remount,noatime /");
}

void suggest_noatime(void)
{
	FILE *file;
	char buffer[1024];
	int suggest = 0;
	file = fopen("/proc/mounts","r");
	if (!file)
		return;
	while (!feof(file)) {
		memset(buffer, 0, 1024);
		if (!fgets(buffer, 1023, file))
			break;
		if (strstr(buffer, " / ext3") && !strstr(buffer, "noatime"))
			suggest = 1;

	}
	if (suggest) {
		add_suggestion( _("Suggestion: enable the noatime filesystem option by executing the following command:\n"
		 	"   mount -o remount,noatime /          or by pressing the T key \n"
			"noatime disables persistent access time of file accesses, which causes lots of disk IO."), 5, 'T', _(" T - enable noatime "), noatime_on);
	}
	fclose(file);
}
