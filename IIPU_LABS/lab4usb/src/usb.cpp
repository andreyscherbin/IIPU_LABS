#include <stdio.h>
#include <string.h>
#include <cstdio>
#include <libusb-1.0/libusb.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <error.h>
#include <sys/mount.h>
#include <scsi/sg_lib.h>
#include <scsi/sg_cmds.h>
#include <libudev.h>
#include <glib.h>
#include <unistd.h>
#include <boost/xpressive/xpressive.hpp>
#include <chrono>
#include <ctime>
#include <pthread.h>
#include <dirent.h>
#include <bits/stdc++.h>
#include <mutex>

using namespace boost::xpressive;

#define USB_DEVICE_CLASS_RESERVED           0x00
#define USB_DEVICE_CLASS_COMMUNICATIONS     0x02
#define USB_DEVICE_CLASS_MONITOR            0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE 0x05
#define USB_DEVICE_CLASS_POWER              0x06
#define USB_DEVICE_CLASS_PRINTER            0x07
#define USB_DEVICE_CLASS_STORAGE            0x08
#define USB_DEVICE_CLASS_HUB                0x09
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC    0xFF*/

#define USB_DEVICE_CLASS_HUMAN_INTERFACE    0x03
#define USB_DEVICE_PROTOCOL_KEYBOARD        0x01
#define USB_DEVICE_PROTOCOL_MOUSE           0x02

#define USB_DEVICE_CLASS_AUDIO              0x01
#define USB_DEVICE_SUBCLASS_HEADPHONES      0x02

#define USB_DEVICE_CLASS_STORAGE            0x08

#define MOUSE                               0
#define HEADPHONES                          1
#define STORAGE                             2
#define NUMBER_DEVICES                      3

bool flag = false;
char NAME_DEVICE[100];
std::mutex g_lock;

class Device {
	// Access specifier
public:

	// Member Functions()
	Device(uint16_t idProduct, uint16_t idVendor, std::string type,
			uint16_t class_device, uint16_t subclass_device,
			uint16_t protocol_device, uint16_t class_interface,
			uint16_t subclass_interface, uint16_t protocol_interface);
	std::string get_type() const;
	uint16_t get_id_product() const;
	uint16_t get_id_vendor() const;
	uint16_t get_class_device() const;
	uint16_t get_subclass_device() const;
	uint16_t get_protocol_device() const;
	uint16_t get_class_interface() const;
	uint16_t get_subclass_interface() const;
	uint16_t get_protocol_interface() const;

private:
	// Data Members
	uint16_t idProduct;
	uint16_t idVendor;
	uint16_t class_device;
	uint16_t subclass_device;
	uint16_t protocol_device;
	uint16_t class_interface;
	uint16_t subclass_interface;
	uint16_t protocol_interface;
	std::string type;

};

Device::Device(uint16_t idProduct, uint16_t idVendor, std::string type,
		uint16_t class_device, uint16_t subclass_device,
		uint16_t protocol_device, uint16_t class_interface,
		uint16_t subclass_interface, uint16_t protocol_interface) {

	this->idProduct = idProduct;
	this->idVendor = idVendor;
	this->type = type;
	this->class_device = class_device;
	this->subclass_device = subclass_device;
	this->protocol_device = protocol_device;
	this->class_interface = class_interface;
	this->subclass_interface = subclass_interface;
	this->protocol_interface = protocol_interface;
}

std::string Device::get_type() const {
	return type;
}

uint16_t Device::get_id_product() const {
	return idProduct;
}

uint16_t Device::get_id_vendor() const {
	return idVendor;
}

uint16_t Device::get_class_device() const {
	return class_device;
}
uint16_t Device::get_subclass_device() const {

	return subclass_device;
}
uint16_t Device::get_protocol_device() const {

	return protocol_device;
}
uint16_t Device::get_class_interface() const {

	return class_interface;
}
uint16_t Device::get_subclass_interface() const {

	return subclass_interface;
}
uint16_t Device::get_protocol_interface() const {

	return protocol_interface;
}

std::ostream& operator<<(std::ostream &os, Device const &m) {
	return os << std::hex << m.get_id_product() << " " << std::hex
			<< m.get_id_vendor() << " " << std::hex << m.get_class_device()
			<< " " << std::hex << m.get_subclass_device() << " " << std::hex
			<< m.get_protocol_device() << " " << std::hex
			<< m.get_class_interface() << " " << std::hex
			<< m.get_subclass_interface() << " " << std::hex
			<< m.get_protocol_interface() << " " << m.get_type();
}

std::string convertToString(char *a, int size) {
	int i;
	std::string s = "";
	for (i = 0; i < size; i++) {
		s = s + a[i];
	}
	return s;
}

int get_device(const char *name) {
	struct stat fs;
	char *token;

	if (stat(name, &fs) < 0) {
		fprintf(stderr, "%s: No such file or directory\n", name);
		return 0;
	}

	FILE *f;
	char sline[256];
	char minmaj[128];

	sprintf(minmaj, "%d:%d ", (int) fs.st_dev >> 8, (int) fs.st_dev & 0xff);

	f = fopen("/proc/self/mountinfo", "r");

	if (f == NULL) {
		fprintf(stderr, "Failed to open /proc/self/mountinfo\n");
		exit(-1);
	}

	while (fgets(sline, 256, f)) {

		char *token;
		char *where;

		token = strtok(sline, "-");
		where = strstr(token, minmaj);
		if (where) {
			token = strtok(NULL, " -:");
			token = strtok(NULL, " -:");
			strcpy(NAME_DEVICE, token);
			break;
		}

	}
	return 1;
	fclose(f);

}

static gboolean sysfs_exists(const gchar *path, const gchar *file) {
	struct stat statbuf;
	gchar *s;
	gboolean ret;

	ret = FALSE;

	s = g_strdup_printf("%s/%s", path, file);
	if (stat(s, &statbuf) == 0)
		ret = TRUE;
	g_free(s);
	return ret;
}

static gboolean sysfs_write(const gchar *path, const gchar *file,
		const gchar *value) {
	FILE *f;
	gchar *s;
	gboolean ret;

	ret = FALSE;
	s = NULL;
	f = NULL;

	s = g_strdup_printf("%s/%s", path, file);
	f = fopen(s, "w");
	if (f == NULL) {
		g_printerr("FAILED: Cannot open %s for writing: %m\n", s);
		return false;
	}

	if (fwrite(value, sizeof(char), strlen(value), f) < strlen(value)) {
		g_printerr("FAILED: Error writing %s to %s: %m\n", value, s);
		return false;
	}

	ret = TRUE;

	if (f != NULL)
		fclose(f);
	return ret;
}

void* events_scanner(void *arg) {
	// detach the current thread
	// from the calling thread
	pthread_detach(pthread_self());

	std::string filename = "/var/log/syslog";
	std::ifstream input;

	int hour_old = 0;
	int minute_old = 0;
	int second_old = 0;

	while (1) {
		input.open(filename);
		if (!input.is_open()) {
			perror("is_open:");
			break;
		}

		time_t now = time(0);
		tm *ltm = localtime(&now);

		std::string hour = std::to_string(ltm->tm_hour);
		std::string minute = std::to_string(ltm->tm_min);
		if (ltm->tm_min >= 0 && ltm->tm_min <= 9) {
			std::string new_minute = "0" + minute;
			minute = new_minute;
		}
		std::string seconds = std::to_string(ltm->tm_sec);
		if (ltm->tm_sec >= 0 && ltm->tm_sec <= 9) {
			std::string new_seconds = "0" + seconds;
			seconds = new_seconds;
		}

		std::string safely_remove1 = hour + ":" + minute + ":" + seconds
				+ ".+(Powered off /dev/sd)";
		std::string failure_safely_remove = hour + ":" + minute + ":" + seconds
				+ ".+(Unmount operation failed)";
		std::string safely_remove2 = hour + ":" + minute + ":" + seconds
				+ ".+(Unmounted /dev/sd)";
		sregex rex_safely_remove1 = sregex::compile(safely_remove1);
		sregex rex_safely_remove2 = sregex::compile(safely_remove2);
		sregex rex_failure_safely_remove = sregex::compile(
				failure_safely_remove);

		while (input) {
			std::string line;
			smatch what;
			getline(input, line);

			if (regex_search(line, what, rex_safely_remove1)) {

				if (hour_old == ltm->tm_hour && minute_old == ltm->tm_min
						&& second_old == ltm->tm_sec) {
				} else {

					g_lock.lock();
					flag = true;
					g_lock.unlock();
					std::cout << "Last system event: = Safely remove "
							<< std::endl;
					hour_old = ltm->tm_hour;
					minute_old = ltm->tm_min;
					second_old = ltm->tm_sec;
				}

			}
			if (regex_search(line, what, rex_failure_safely_remove)) {

				if (hour_old == ltm->tm_hour && minute_old == ltm->tm_min
						&& second_old == ltm->tm_sec) {
				} else {
					std::cout << "Last system event: = Failure safely remove "
							<< std::endl;
					hour_old = ltm->tm_hour;
					minute_old = ltm->tm_min;
					second_old = ltm->tm_sec;
				}

			}
			if (regex_search(line, what, rex_safely_remove2)) {

				if (hour_old == ltm->tm_hour && minute_old == ltm->tm_min
						&& second_old == ltm->tm_sec) {
				} else {

					g_lock.lock();
					flag = true;
					g_lock.unlock();
					std::cout << "Last system event: = Safely remove "
							<< std::endl;
					hour_old = ltm->tm_hour;
					minute_old = ltm->tm_min;
					second_old = ltm->tm_sec;
				}

			}

		}

		input.close();
	}

	// exit the current thread
	pthread_exit(NULL);
}

void* safely_remove(void *arg) {

	std::cout << "Press e to safely remove" << std::endl;
	pthread_detach(pthread_self());
	while (1) {
		sleep(1);
		char c;
		scanf("%c%*c", &c);

		if (c != 'e') {
			continue;
		}

		int sg_fd = -1;
		int ret;

		struct udev *udev;
		struct udev_device *udevice;
		struct udev_device *udevice_usb_interface;
		struct udev_device *udevice_usb_device;
		gchar *unbind_path;
		gchar *power_level_path;
		gchar *usb_interface_name;
		struct stat statbuf;
		const char *bNumInterfaces;
		gchar *endp;
		int num_interfaces;

		udev = NULL;
		udevice = NULL;
		udevice_usb_interface = NULL;
		udevice_usb_device = NULL;
		usb_interface_name = NULL;
		unbind_path = NULL;
		power_level_path = NULL;

		ret = 1;
		sg_fd = -1;
		int i = 34;
		i++;
		DIR *dir;
		struct dirent *ent;
		int index = 0;
		std::vector<std::string> mount_points_names;

		if ((dir = opendir("/media/andrey/")) != NULL) {
			/* print all the files and directories within directory */

			while ((ent = readdir(dir)) != NULL) {

				if (ent->d_name[0] == '.')
					continue;
				std::cout << index + 1 << " " << ent->d_name << std::endl;
				char str[100];
				strcpy(str, "/media/andrey/");
				strcat(str, ent->d_name);
				std::string mount_point_name = std::string(str);
				mount_points_names.push_back(mount_point_name);
				i++;
				index++;
			}
			closedir(dir);
		} else {
			/* could not open directory */
			perror("");
		}

		if (index == 0) {
			std::cout << "no devices to safely remove " << std::endl;
			continue;
		}
		std::cout << "choose number device" << std::endl;

		int ch;
		scanf("%d%*c", &ch);
		ch = ch - 1;
		std::cout << ch << std::endl;
		get_device(mount_points_names[ch].c_str());

		if (!umount(mount_points_names[ch].c_str())) {

			std::cout << "unmount success" << std::endl;
		} else
			perror("unmount ERROR:");

		sg_fd = sg_cmds_open_device(NAME_DEVICE, 1 /* read_only */, 1);
		if (sg_fd < 0) {

			std::cout << "Cannot open " << NAME_DEVICE << std::endl;
			if (sg_fd > 0)
				sg_cmds_close_device(sg_fd);
		}

		std::cout << "SYNCHRONIZE CACHE: " << std::endl;
		if (sg_ll_sync_cache_10(sg_fd, 0, /* sync_nv */
		0, /* immed */
		0, /* group */
		0, /* lba */
		0, /* count */
		1, /* noisy */
		0 /* verbose */
		) != 0) {

			std::cout << "FAILED: " << std::endl;
			/* this is not a catastrophe, carry on */

			std::cout << "Continuing despite SYNCHRONIZE CACHE failure"
					<< std::endl;
		} else {

			std::cout << "OK" << std::endl;
		}

		std::cout << "STOP UNIT: " << std::endl;
		refresh();
		if (sg_ll_start_stop_unit(sg_fd, 0, /* immed */
		0, /* pc_mod__fl_num */
		0, /* power_cond */
		0, /* noflush__fl */
		0, /* loej */
		0, /* start */
		1, /* noisy */
		0 /* verbose */
		) != 0) {

			std::cout << "FAILED: " << std::endl;
			if (sg_fd > 0)
				sg_cmds_close_device(sg_fd);
		} else {

			std::cout << "OK" << std::endl;
		}

		/* OK, close the device */
		sg_cmds_close_device(sg_fd);
		sg_fd = -1;

		if (stat(NAME_DEVICE, &statbuf) != 0) {

			return 0;
		}
		if (statbuf.st_rdev == 0) {

			return 0;
		}

		udev = udev_new();
		if (udev == NULL) {

			return 0;
		}

		udevice = udev_device_new_from_devnum(udev, 'b', statbuf.st_rdev);
		if (udevice == NULL) {

			return 0;
		}
		udevice_usb_interface = udev_device_get_parent_with_subsystem_devtype(
				udevice, "usb", "usb_interface");
		if (udevice_usb_interface == NULL) {

			return 0;
		}
		udevice_usb_device = udev_device_get_parent_with_subsystem_devtype(
				udevice, "usb", "usb_device");
		if (udevice_usb_device == NULL) {

			return 0;
		}

		std::cout << "Detaching device " << NAME_DEVICE << "\nUSB device: "
				<< udev_device_get_syspath(udevice_usb_device) << std::endl;

		usb_interface_name = g_path_get_basename(
				udev_device_get_devpath(udevice_usb_interface));

		std::cout << "Unbinding USB interface driver: " << std::endl;
		if (!sysfs_write(udev_device_get_syspath(udevice_usb_interface),
				"driver/unbind", usb_interface_name))
			return 0;

		std::cout << "OK" << std::endl;

		bNumInterfaces = udev_device_get_sysattr_value(udevice_usb_device,
				"bNumInterfaces");
		num_interfaces = strtol(bNumInterfaces, &endp, 0);
		if (endp != NULL && num_interfaces == 1) {

			std::cout << "Suspending USB device: " << std::endl;
			if (sysfs_exists(udev_device_get_syspath(udevice_usb_device),
					"power/control"))
				power_level_path = "power/control";
			else
				power_level_path = "power/level";

			if (!sysfs_write(udev_device_get_syspath(udevice_usb_device),
					power_level_path, "auto")
					|| !sysfs_write(udev_device_get_syspath(udevice_usb_device),
							"power/autosuspend", "0"))
				return 0;

			std::cout << "OK" << std::endl;

			if (sysfs_exists(udev_device_get_syspath(udevice_usb_device),
					"remove")) {

				std::cout << "Disabling USB port for device: " << std::endl;
				if (!sysfs_write(udev_device_get_syspath(udevice_usb_device),
						"remove", "1"))
					return 0;

				std::cout << "OK" << std::endl;
				flag = true;
				std::cout << "Last system event: = Safely remove " << std::endl;
			}
		} else {

			std::cout
					<< "Not powering down device since multiple USB interfaces exist."
					<< std::endl;
		}
		ret = 0;

		if (sg_fd > 0)
			sg_cmds_close_device(sg_fd);
		if (udevice != NULL)
			udev_device_unref(udevice);
		if (udev != NULL)
			udev_unref(udev);
	}

	pthread_exit(NULL);

}

std::string compare_two_vector_usb_devices(
		std::vector<Device> &devices_list_old,
		std::vector<Device> &devices_list_new) {

	for (int i = 0; i < devices_list_new.size(); i++)
		for (int j = 0; j < devices_list_old.size(); j++) {
			if (devices_list_new[i].get_id_product()
					!= devices_list_old[j].get_id_product()
					&& devices_list_new[i].get_id_vendor()
							!= devices_list_old[j].get_id_vendor()
					&& (devices_list_new[i].get_type() == "MOUSE"
							|| devices_list_new[i].get_type() == "STORAGE"
							|| devices_list_new[i].get_type() == "AUDIO_DEVICE")) {
				return devices_list_new[i].get_type();
			}
		}
	return "null";
}

void create_vector_usb_devices(std::vector<Device> &devices_list) {

	int rc = 0;
	libusb_context *context = NULL;
	libusb_device **list = NULL;
	rc = libusb_init(&context);

	ssize_t count_new = libusb_get_device_list(context, &list);

	for (size_t idx = 0; idx < count_new; ++idx) {
		libusb_device *device = list[idx];
		libusb_device_descriptor desc = { 0 };

		libusb_get_device_descriptor(device, &desc);
		libusb_get_device_address(device);
		libusb_get_port_number(device);

		libusb_device_handle *handle = libusb_open_device_with_vid_pid(
		NULL, desc.idVendor, desc.idProduct);
		if (handle != NULL) {
			libusb_config_descriptor *config_descriptor = { 0 };
			rc = libusb_get_active_config_descriptor(device,
					&config_descriptor);

			uint8_t bInterfaceClass =
					config_descriptor->interface[0].altsetting[0].bInterfaceClass;
			uint8_t bInterfaceSubClass =
					config_descriptor->interface[0].altsetting[0].bInterfaceSubClass;
			uint8_t bInterfaceProtocol =
					config_descriptor->interface[0].altsetting[0].bInterfaceProtocol;

			std::string type_device;

			switch (desc.bDeviceClass) {
			case USB_DEVICE_CLASS_AUDIO:
				type_device = "AUDIO_DEVICE";
				break;
			case USB_DEVICE_CLASS_COMMUNICATIONS:
				type_device = "COMMUNICATIONS_DEVICE";
				break;
			case USB_DEVICE_CLASS_HUMAN_INTERFACE:
				type_device = "HUMAN_INTERFACE_DEVICE";
				break;
			case USB_DEVICE_CLASS_MONITOR:
				type_device = "MONITOR_DEVICE";
				break;
			case USB_DEVICE_CLASS_PHYSICAL_INTERFACE:
				type_device = "PHYSICAL_INTERFACE";
				break;
			case USB_DEVICE_CLASS_POWER:
				type_device = "POWER";
				break;
			case USB_DEVICE_CLASS_PRINTER:
				type_device = "PRINTER";
				break;
			case USB_DEVICE_CLASS_STORAGE:
				type_device = "STORAGE";
				break;
			case USB_DEVICE_CLASS_HUB:
				type_device = "HUB";
				break;
			case 0:
				if (bInterfaceClass == 0x08) {
					type_device = "STORAGE";
				}
				if (bInterfaceClass == 0x03) {
					if (bInterfaceProtocol == 0x01) {
						type_device = "KEYBOARD";
					}
					if (bInterfaceProtocol == 0x02) {
						type_device = "MOUSE";
					}
				}
				break;
			}

			Device dev(desc.idProduct, desc.idVendor, type_device,
					desc.bDeviceClass, desc.bDeviceSubClass,
					desc.bDeviceProtocol, bInterfaceClass, bInterfaceSubClass,
					bInterfaceProtocol);
			devices_list.push_back(dev);
		}

	}

}

int main(int argc, char *argv[]) {

	pthread_t thread_events_scanner;
	pthread_t thread_safely_remove;

	// Creating a new thread

	pthread_create(&thread_events_scanner, NULL, &events_scanner, NULL);
	pthread_create(&thread_safely_remove, NULL, &safely_remove, NULL);
	printf("This line may be printed"
			" before thread terminates\n");

	libusb_context *context = NULL;
	libusb_device **list = NULL;
	int rc = 0;

	rc = libusb_init(&context);
	assert(rc == 0);

	ssize_t count_new = libusb_get_device_list(context, &list);
	ssize_t count_old = libusb_get_device_list(context, &list);

	std::vector<Device> device_list_old;
	create_vector_usb_devices(device_list_old);

	//count_new = libusb_get_device_list(context, &list);
	//count_new = count_old;

	while (1) {
		sleep(1);
		bool mouse_headphones_storage_isConnected[3] = { 0, 0, 0 };
		count_new = libusb_get_device_list(context, &list);

		if (count_old > count_new && !flag) {

			std::vector<Device> device_list_new;
			create_vector_usb_devices(device_list_new);
			std::cout << "Last system event: = Not safely remove " << std::endl;
			device_list_old = device_list_new;
			g_lock.lock();
			flag = false;
			g_lock.unlock();
		}
		if (count_old < count_new) {

			std::vector<Device> device_list_new;
			create_vector_usb_devices(device_list_new);
			std::string name_device = compare_two_vector_usb_devices(
					device_list_old, device_list_new);
			if (name_device != "null") {
				std::cout << "Last system event: = The device is attached "
						<< name_device << std::endl;
				device_list_old = device_list_new;
				g_lock.lock();
				flag = false;
				g_lock.unlock();
			}
		}
		count_old = count_new;

	}
	// Waiting for the created thread to terminate
	pthread_join(thread_events_scanner, NULL);
	pthread_join(thread_safely_remove, NULL);

	printf("This line will be printed"
			" after thread ends\n");

	pthread_exit(NULL);
	libusb_free_device_list(list, 1);
	libusb_exit(NULL);
	endwin();
	return 0;

}
