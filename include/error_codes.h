#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include <vector>
#include <string>

const std::vector<std::vector<std::string>> ERROR_CODES = {
    {
        "Cell volt high level 1",
        "Cell volt high level 2",
        "Cell volt low level 1",
        "Cell volt low level 2",
        "Sum volt high level 1",
        "Sum volt high level 2",
        "Sum volt low level 1",
        "Sum volt low level 2"
    },
    {
        "Chg temp high level 1",
        "Chg temp high level 2",
        "Chg temp low level 1",
        "Chg temp low level 2",
        "Dischg temp high level 1",
        "Dischg temp high level 2",
        "Dischg temp low level 1",
        "Dischg temp low level 2"
    },
    {
        "Chg overcurrent level 1",
        "Chg overcurrent level 2",
        "Dischg overcurrent level 1",
        "Dischg overcurrent level 2",
        "SOC high level 1",
        "SOC high level 2",
        "SOC low level 1",
        "SOC low level 2"
    },
    {
        "Diff volt level 1",
        "Diff volt level 2",
        "Diff temp level 1",
        "Diff temp level 2",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved"
    },
    {
        "Chg MOS temp high alarm",
        "Dischg MOS temp high alarm",
        "Chg MOS temp sensor err",
        "Dischg MOS temp sensor err",
        "Chg MOS adhesion err",
        "Dischg MOS adhesion err",
        "Chg MOS open circuit err",
        "Dischg MOS open circuit err"
    },
    {
        "AFE collect chip err",
        "Voltage collect dropped",
        "Cell temp sensor err",
        "EEPROM err",
        "RTC err",
        "Precharge failure",
        "Communication failure",
        "Internal communication failure"
    },
    {
        "Current module fault",
        "Sum voltage detect fault",
        "Short circuit protect fault",
        "Low volt forbidden chg fault",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved"
    },
    {
        "Fault code"
    }
};

#endif // ERROR_CODES_H
