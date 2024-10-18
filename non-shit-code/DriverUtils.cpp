#include "DriverUtils.h"

bool DriverUtils::isIntelCPU() {

    int cpuInfo[4];
    __cpuid(cpuInfo, 0); // Call CPUID with EAX=0 to get vendor ID
    return cpuInfo[1] == 'I';

}