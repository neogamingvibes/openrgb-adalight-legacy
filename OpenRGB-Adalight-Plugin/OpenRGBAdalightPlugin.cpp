/*---------------------------------------------------------*\
| OpenRGBAdalightPlugin.cpp                                 |
|                                                           |
|   OpenRGB Plugin für Adalight/Ardulight Geräte            |
\*---------------------------------------------------------*/

#include "OpenRGBAdalightPlugin.h"
#include <QImage>

OpenRGBAdalightPlugin::OpenRGBAdalightPlugin()
    : m_resourceManager(nullptr)
    , m_device(new AdalightDevice())
    , m_widget(nullptr)
{
}

OpenRGBAdalightPlugin::~OpenRGBAdalightPlugin()
{
    Unload();
    delete m_device;
}

/*---------------------------------------------------------*\
| GetPluginInfo                                             |
\*---------------------------------------------------------*/
OpenRGBPluginInfo OpenRGBAdalightPlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info;

    info.Name           = "Adalight Plugin";
    info.Description    = "Unterstützung für Adalight/Ardulight Geräte über serielle Schnittstelle";
    info.Version        = "1.0.0";
    info.Commit         = "N/A";
    info.URL            = "https://github.com/yourusername/OpenRGB-Adalight-Plugin";
    info.Icon           = QImage();

    info.Location       = OPENRGB_PLUGIN_LOCATION_SETTINGS;
    info.Label          = "Adalight";
    info.TabIconString  = "";
    info.TabIcon        = QImage();

    return info;
}

/*---------------------------------------------------------*\
| GetPluginAPIVersion                                       |
\*---------------------------------------------------------*/
unsigned int OpenRGBAdalightPlugin::GetPluginAPIVersion()
{
    return OPENRGB_PLUGIN_API_VERSION;
}

/*---------------------------------------------------------*\
| Load                                                      |
| Wird aufgerufen wenn das Plugin geladen wird              |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::Load(ResourceManagerInterface* resource_manager_ptr)
{
    m_resourceManager = resource_manager_ptr;

    /*-----------------------------------------------------*\
    | Konfig Pfad holen                                     |
    \*-----------------------------------------------------*/
    m_configPath = m_resourceManager->GetConfigurationDirectory().string();

    /*-----------------------------------------------------*\
    | Widget erstellen                                      |
    \*-----------------------------------------------------*/
    m_widget = new AdalightWidget();

    /*-----------------------------------------------------*\
    | Einstellungen laden                                   |
    \*-----------------------------------------------------*/
    m_widget->LoadSettings(m_configPath);

    /*-----------------------------------------------------*\
    | Widget Signals verbinden                              |
    \*-----------------------------------------------------*/
    connect(m_widget, &AdalightWidget::ReconnectRequested,
            this, &OpenRGBAdalightPlugin::OnReconnectRequested);

    connect(m_widget, &AdalightWidget::SettingsChanged,
            this, &OpenRGBAdalightPlugin::OnSettingsChanged);

    /*-----------------------------------------------------*\
    | Callback registrieren wenn neue Controller erkannt    |
    \*-----------------------------------------------------*/
    m_resourceManager->RegisterDeviceListChangeCallback(
        DeviceListChangedCallback, this);

    /*-----------------------------------------------------*\
    | Automatisch verbinden beim Start                      |
    \*-----------------------------------------------------*/
    Connect();
}

/*---------------------------------------------------------*\
| GetWidget                                                 |
\*---------------------------------------------------------*/
QWidget* OpenRGBAdalightPlugin::GetWidget()
{
    return m_widget;
}

/*---------------------------------------------------------*\
| GetTrayMenu                                               |
\*---------------------------------------------------------*/
QMenu* OpenRGBAdalightPlugin::GetTrayMenu()
{
    return nullptr;
}

/*---------------------------------------------------------*\
| Unload                                                    |
| Wird aufgerufen wenn das Plugin entladen wird             |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::Unload()
{
    /*-----------------------------------------------------*\
    | Callbacks abmelden                                    |
    \*-----------------------------------------------------*/
    if (m_resourceManager != nullptr)
    {
        m_resourceManager->UnregisterDeviceListChangeCallback(
            DeviceListChangedCallback, this);

        for (RGBController* controller : m_registeredControllers)
        {
            controller->UnregisterUpdateCallback(this);
        }
        m_registeredControllers.clear();
    }

    /*-----------------------------------------------------*\
    | Einstellungen speichern                               |
    \*-----------------------------------------------------*/
    if (m_widget != nullptr)
    {
        m_widget->SaveSettings(m_configPath);
    }

    /*-----------------------------------------------------*\
    | Gerät trennen                                         |
    \*-----------------------------------------------------*/
    Disconnect();
}

/*---------------------------------------------------------*\
| Connect                                                   |
| Verbindet mit dem Adalight Gerät                          |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::Connect()
{
    if (m_widget == nullptr)
    {
        return;
    }

    std::string portName = m_widget->GetPortName();
    int         baudRate = m_widget->GetBaudRate();

    if (portName.empty())
    {
        m_widget->setStatus("Kein COM Port angegeben");
        return;
    }

    Disconnect();

    bool ok = m_device->Open(portName, baudRate);

    if (ok)
    {
        m_widget->setStatus("Verbunden mit " + QString::fromStdString(portName));
        OnDeviceListChanged();
    }
    else
    {
        m_widget->setStatus("Verbindung fehlgeschlagen: " + QString::fromStdString(portName));
    }
}

/*---------------------------------------------------------*\
| Disconnect                                                |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::Disconnect()
{
    if (m_device->IsOpen())
    {
        int ledCount = (m_widget != nullptr) ? m_widget->GetLedCount() : 1;
        m_device->TurnOff(ledCount);
        m_device->Close();

        if (m_widget != nullptr)
        {
            m_widget->setStatus("Nicht verbunden");
        }
    }
}

/*---------------------------------------------------------*\
| SendColorsToDevice                                        |
| Liest alle Farben aus den OpenRGB Controllern und sendet  |
| sie an das Adalight Gerät                                 |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::SendColorsToDevice()
{
    if (!m_device->IsOpen() || m_widget == nullptr)
    {
        return;
    }

    int        ledCount   = m_widget->GetLedCount();
    ColorOrder colorOrder = m_widget->GetColorOrder();

    /*-----------------------------------------------------*\
    | Farben aus allen registrierten Controllern sammeln    |
    \*-----------------------------------------------------*/
    std::vector<unsigned int> allColors;

    std::vector<RGBController*>& controllers =
        m_resourceManager->GetRGBControllers();

    for (RGBController* controller : controllers)
    {
        for (unsigned int i = 0; i < controller->colors.size(); i++)
        {
            allColors.push_back(controller->colors[i]);

            if ((int)allColors.size() >= ledCount)
            {
                break;
            }
        }

        if ((int)allColors.size() >= ledCount)
        {
            break;
        }
    }

    /*-----------------------------------------------------*\
    | Falls weniger Farben als LEDs: Rest mit Schwarz füllen|
    \*-----------------------------------------------------*/
    while ((int)allColors.size() < ledCount)
    {
        allColors.push_back(0);
    }

    m_device->SendColors(allColors, ledCount, colorOrder);
}

/*---------------------------------------------------------*\
| DeviceListChangedCallback (static)                        |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::DeviceListChangedCallback(void* arg)
{
    OpenRGBAdalightPlugin* plugin = static_cast<OpenRGBAdalightPlugin*>(arg);
    plugin->OnDeviceListChanged();
}

/*---------------------------------------------------------*\
| OnDeviceListChanged                                       |
| Registriert Update Callbacks für alle neuen Controller    |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::OnDeviceListChanged()
{
    if (m_resourceManager == nullptr)
    {
        return;
    }

    /*-----------------------------------------------------*\
    | Alte Callbacks abmelden                               |
    \*-----------------------------------------------------*/
    for (RGBController* controller : m_registeredControllers)
    {
        controller->UnregisterUpdateCallback(this);
    }
    m_registeredControllers.clear();

    /*-----------------------------------------------------*\
    | Neue Callbacks für alle Controller registrieren       |
    \*-----------------------------------------------------*/
    std::vector<RGBController*>& controllers =
        m_resourceManager->GetRGBControllers();

    for (RGBController* controller : controllers)
    {
        controller->RegisterUpdateCallback(ControllerUpdateCallback, this);
        m_registeredControllers.push_back(controller);
    }
}

/*---------------------------------------------------------*\
| ControllerUpdateCallback (static)                         |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::ControllerUpdateCallback(void* arg)
{
    OpenRGBAdalightPlugin* plugin = static_cast<OpenRGBAdalightPlugin*>(arg);
    plugin->OnControllerUpdate();
}

/*---------------------------------------------------------*\
| OnControllerUpdate                                        |
| Wird aufgerufen wenn ein Controller seine Farben ändert   |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::OnControllerUpdate()
{
    SendColorsToDevice();
}

/*---------------------------------------------------------*\
| Slots                                                     |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::OnReconnectRequested()
{
    m_widget->SaveSettings(m_configPath);
    Connect();
}

void OpenRGBAdalightPlugin::OnSettingsChanged()
{
    m_widget->SaveSettings(m_configPath);
}
