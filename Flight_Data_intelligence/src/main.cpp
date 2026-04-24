#include <iostream>
#include "telemetry.h"

int main() {
    std::cout << "AeroSentinel: systems online" << std::endl;
    std::cout << "TelemetryFrame size: " << sizeof(TelemetryFrame) << " bytes" << std::endl;
    return 0;
}