/*---------------------------------------------------------*\
| AdalightDevice.h                                          |
|                                                           |
|   OpenRGB Plugin for Adalight/Ardulight devices           |
|   Serial communication via Windows API                    |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <windows.h>

/*---------------------------------------------------------*\
| Color Order Enum                                          |
\*---------------------------------------------------------*/
enum class ColorOrder
{
    RGB,
    RBG,
    BRG,
    BGR,
    GRB,
    GBR
};

/*---------------------------------------------------------*\
| AdalightDevice Class                                      |
| Manages the serial connection and the Adalight protocol   |
\*---------------------------------------------------------*/
class AdalightDevice
{
public:
    AdalightDevice();
    ~AdalightDevice();

    /*-----------------------------------------------------*\
    | Open / close connection                               |
    \*-----------------------------------------------------*/
    bool        Open(const std::string& portName, int baudRate);
    void        Close();
    bool        IsOpen() const;

    /*-----------------------------------------------------*\
    | Send LED data                                         |
    \*-----------------------------------------------------*/
    bool        SendColors(const std::vector<unsigned int>& colors,
                           int ledCount,
                           ColorOrder colorOrder);

    /*-----------------------------------------------------*\
    | Turn off all LEDs                                     |
    \*-----------------------------------------------------*/
    bool        TurnOff(int ledCount);

private:
    /*-----------------------------------------------------*\
    | Build Adalight protocol header                        |
    | 'A' 'd' 'a' | Hi | Lo | (Hi ^ Lo ^ 0x55)             |
    \*-----------------------------------------------------*/
    std::vector<unsigned char> BuildHeader(int ledCount);

    /*-----------------------------------------------------*\
    | Write data to serial port                             |
    \*-----------------------------------------------------*/
    bool        WriteBuffer(const std::vector<unsigned char>& buffer);

    /*-----------------------------------------------------*\
    | Private member variables                              |
    \*-----------------------------------------------------*/
    HANDLE      m_serialHandle;
    bool        m_isOpen;
};
