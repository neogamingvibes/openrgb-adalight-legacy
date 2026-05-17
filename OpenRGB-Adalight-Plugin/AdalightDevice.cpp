/*---------------------------------------------------------*\
| AdalightDevice.cpp                                        |
|                                                           |
|   OpenRGB Plugin für Adalight/Ardulight Geräte            |
|   Serielle Kommunikation über Windows API                 |
\*---------------------------------------------------------*/

#include "AdalightDevice.h"
#include <string>

AdalightDevice::AdalightDevice()
    : m_serialHandle(INVALID_HANDLE_VALUE)
    , m_isOpen(false)
{
}

AdalightDevice::~AdalightDevice()
{
    Close();
}

/*---------------------------------------------------------*\
| Open                                                      |
| Öffnet den seriellen Port mit den angegebenen Einstellungen|
\*---------------------------------------------------------*/
bool AdalightDevice::Open(const std::string& portName, int baudRate)
{
    Close();

    /*-----------------------------------------------------*\
    | Windows benötigt den Prefix "\\.\" für COM Ports      |
    | über COM9, aber wir nutzen ihn immer für Sicherheit   |
    \*-----------------------------------------------------*/
    std::string fullPortName = "\\\\.\\" + portName;

    m_serialHandle = CreateFileA(
        fullPortName.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (m_serialHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    /*-----------------------------------------------------*\
    | Serielle Port Einstellungen konfigurieren             |
    \*-----------------------------------------------------*/
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(m_serialHandle, &dcbSerialParams))
    {
        Close();
        return false;
    }

    dcbSerialParams.BaudRate    = baudRate;
    dcbSerialParams.ByteSize    = 8;
    dcbSerialParams.StopBits    = ONESTOPBIT;
    dcbSerialParams.Parity      = NOPARITY;
    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

    if (!SetCommState(m_serialHandle, &dcbSerialParams))
    {
        Close();
        return false;
    }

    /*-----------------------------------------------------*\
    | Timeouts setzen                                       |
    \*-----------------------------------------------------*/
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.WriteTotalTimeoutConstant   = 100;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(m_serialHandle, &timeouts))
    {
        Close();
        return false;
    }

    m_isOpen = true;
    return true;
}

/*---------------------------------------------------------*\
| Close                                                     |
| Schließt den seriellen Port                               |
\*---------------------------------------------------------*/
void AdalightDevice::Close()
{
    if (m_serialHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_serialHandle);
        m_serialHandle = INVALID_HANDLE_VALUE;
    }
    m_isOpen = false;
}

/*---------------------------------------------------------*\
| IsOpen                                                    |
\*---------------------------------------------------------*/
bool AdalightDevice::IsOpen() const
{
    return m_isOpen;
}

/*---------------------------------------------------------*\
| BuildHeader                                               |
| Baut den Adalight Protokoll Header:                       |
| 'A' 'd' 'a' | Hi | Lo | (Hi ^ Lo ^ 0x55)                 |
\*---------------------------------------------------------*/
std::vector<unsigned char> AdalightDevice::BuildHeader(int ledCount)
{
    std::vector<unsigned char> header;

    int ledsCountHi = ((ledCount - 1) >> 8) & 0xFF;
    int ledsCountLo =  (ledCount - 1)       & 0xFF;

    header.push_back('A');
    header.push_back('d');
    header.push_back('a');
    header.push_back((unsigned char)ledsCountHi);
    header.push_back((unsigned char)ledsCountLo);
    header.push_back((unsigned char)(ledsCountHi ^ ledsCountLo ^ 0x55));

    return header;
}

/*---------------------------------------------------------*\
| SendColors                                                |
| Sendet die LED Farben an das Adalight Gerät               |
\*---------------------------------------------------------*/
bool AdalightDevice::SendColors(const std::vector<unsigned int>& colors,
                                 int ledCount,
                                 ColorOrder colorOrder)
{
    if (!m_isOpen)
    {
        return false;
    }

    /*-----------------------------------------------------*\
    | Buffer aufbauen: Header + LED Daten                   |
    \*-----------------------------------------------------*/
    std::vector<unsigned char> buffer = BuildHeader(ledCount);

    int count = (int)colors.size();
    if (count > ledCount)
    {
        count = ledCount;
    }

    for (int i = 0; i < count; i++)
    {
        /*-------------------------------------------------*\
        | Farbe aus OpenRGB RGBColor extrahieren            |
        | RGBColor = 0x00BBGGRR                             |
        \*-------------------------------------------------*/
        unsigned char r = (colors[i])        & 0xFF;
        unsigned char g = (colors[i] >> 8)   & 0xFF;
        unsigned char b = (colors[i] >> 16)  & 0xFF;

        /*-------------------------------------------------*\
        | Color Order anwenden                              |
        \*-------------------------------------------------*/
        switch (colorOrder)
        {
            case ColorOrder::RGB:
                buffer.push_back(r);
                buffer.push_back(g);
                buffer.push_back(b);
                break;

            case ColorOrder::RBG:
                buffer.push_back(r);
                buffer.push_back(b);
                buffer.push_back(g);
                break;

            case ColorOrder::BRG:
                buffer.push_back(b);
                buffer.push_back(r);
                buffer.push_back(g);
                break;

            case ColorOrder::BGR:
                buffer.push_back(b);
                buffer.push_back(g);
                buffer.push_back(r);
                break;

            case ColorOrder::GRB:
                buffer.push_back(g);
                buffer.push_back(r);
                buffer.push_back(b);
                break;

            case ColorOrder::GBR:
                buffer.push_back(g);
                buffer.push_back(b);
                buffer.push_back(r);
                break;

            default:
                buffer.push_back(r);
                buffer.push_back(g);
                buffer.push_back(b);
                break;
        }
    }

    return WriteBuffer(buffer);
}

/*---------------------------------------------------------*\
| TurnOff                                                   |
| Schaltet alle LEDs aus (sendet schwarze Farbe)            |
\*---------------------------------------------------------*/
bool AdalightDevice::TurnOff(int ledCount)
{
    if (!m_isOpen)
    {
        return false;
    }

    std::vector<unsigned char> buffer = BuildHeader(ledCount);

    for (int i = 0; i < ledCount; i++)
    {
        buffer.push_back(0);
        buffer.push_back(0);
        buffer.push_back(0);
    }

    return WriteBuffer(buffer);
}

/*---------------------------------------------------------*\
| WriteBuffer                                               |
| Schreibt den Buffer auf den seriellen Port                |
\*---------------------------------------------------------*/
bool AdalightDevice::WriteBuffer(const std::vector<unsigned char>& buffer)
{
    if (m_serialHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD bytesWritten = 0;
    BOOL result = WriteFile(
        m_serialHandle,
        buffer.data(),
        (DWORD)buffer.size(),
        &bytesWritten,
        NULL
    );

    return (result && bytesWritten == (DWORD)buffer.size());
}
