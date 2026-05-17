/*---------------------------------------------------------*\
| AdalightDevice.h                                          |
|                                                           |
|   OpenRGB Plugin für Adalight/Ardulight Geräte            |
|   Serielle Kommunikation über Windows API                 |
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
| AdalightDevice Klasse                                     |
| Verwaltet die serielle Verbindung und das Adalight        |
| Protokoll                                                 |
\*---------------------------------------------------------*/
class AdalightDevice
{
public:
    AdalightDevice();
    ~AdalightDevice();

    /*-----------------------------------------------------*\
    | Verbindung öffnen / schließen                         |
    \*-----------------------------------------------------*/
    bool        Open(const std::string& portName, int baudRate);
    void        Close();
    bool        IsOpen() const;

    /*-----------------------------------------------------*\
    | LED Daten senden                                      |
    \*-----------------------------------------------------*/
    bool        SendColors(const std::vector<unsigned int>& colors,
                           int ledCount,
                           ColorOrder colorOrder);

    /*-----------------------------------------------------*\
    | Alle LEDs ausschalten                                 |
    \*-----------------------------------------------------*/
    bool        TurnOff(int ledCount);

private:
    /*-----------------------------------------------------*\
    | Adalight Protokoll Header bauen                       |
    | 'A' 'd' 'a' | Hi | Lo | (Hi ^ Lo ^ 0x55)             |
    \*-----------------------------------------------------*/
    std::vector<unsigned char> BuildHeader(int ledCount);

    /*-----------------------------------------------------*\
    | Daten seriell senden                                  |
    \*-----------------------------------------------------*/
    bool        WriteBuffer(const std::vector<unsigned char>& buffer);

    /*-----------------------------------------------------*\
    | Private Member Variablen                              |
    \*-----------------------------------------------------*/
    HANDLE      m_serialHandle;
    bool        m_isOpen;
};
