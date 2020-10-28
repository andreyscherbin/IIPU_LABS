#include <stdio.h>
#include <sys/io.h>
#include "pci.h"


int GetVendorID(unsigned short int data[2]) {
    // pci.h
    for (int i = 0; i < PCI_VENTABLE_LEN; i++)
    {
        if (PciVenTable[i].VendorId == data[0])
        {
            printf("Vendor ID: 0x%X", data[0]);
            printf("  %s\n", PciVenTable[i].VendorName);
            return 0;
        }
    }
    return 0;
}


int GetDeviceID(unsigned short int data[2]) {
    // pci.h
    for (int i = 0; i < PCI_DEVTABLE_LEN; i++)
    {
        if (PciDevTable[i].VendorId == data[0])
        {
            if (PciDevTable[i].DeviceId == data[1])
            {
                printf("Device ID: 0x%X", data[1]);
                printf("  %s\n", PciDevTable[i].DeviceName);
                return 0;
            }
        }
    }
    return 0;
}

/* Calculate the address of the configuration register using the bus number,
   the device number, function number and register number */
unsigned int CalculateAddress(int bus, int device, int function, int reg) {
    unsigned int address = 1;
    address = address << 15;
    address += bus;          // Bus number, 8 bits
    address = address << 5;
    address += device;       // Device number, 5 bits
    address = address << 3;
    address += function;     // Function number, 3 bits
    address = address << 8;
    address += reg;          // Port number
    return address;
}

void ShowDeviceInfo(int bus, int dev, int func) {

    // Configuration register address
    unsigned int configAddress = CalculateAddress(bus, dev, func, 0x00);

    // Read fields of the configuration space
    outl(configAddress, 0xCF8);
    unsigned int regData = inl(0xCFC);

    if (regData != 0xFFFFFFFF) // If there is no device, then the register value is = 0xFFFFFFFF
    {        
        GetVendorID((unsigned short int *)&regData);
        GetDeviceID((unsigned short int *)&regData);        
        printf("_________________________________________________________________________________________________________\n\n");
    }

}

int main(void) {
    //Granted privileges 3 for port access
    if (iopl(3))
    {
        printf("I/O Privilege level change error.\nTry running under ROOT user\n");
        return 1;
    }

    //Main cycle
    for (int bus = 0; bus < 256; bus++)              // Bus number, 8 bits, 2^8 = 256
        for (int dev = 0; dev < 32; dev++)           // Device number, 5 bits, 2^5 = 32
            for (int func = 0; func < 8; func++)     // Function number, 3 bits, 2^3 = 8
                ShowDeviceInfo(bus, dev, func);

    return 0;
}
