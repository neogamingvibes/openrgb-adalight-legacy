/*---------------------------------------------------------*\
| OpenRGBAdalightPlugin.cpp                                 |
|                                                           |
|   OpenRGB Plugin for Adalight/Ardulight devices           |
\*---------------------------------------------------------*/

#include "OpenRGBAdalightPlugin.h"
#include <QImage>

OpenRGBAdalightPlugin::OpenRGBAdalightPlugin()
    : m_resourceManager(nullptr)
    , m_device(new AdalightDevice())
    , m_widget(nullptr)
    , m_keepaliveTimer(new QTimer(this))
{
    /*-----------------------------------------------------*\
    | Send colors every 2 seconds as keepalive              |
    \*-----------------------------------------------------*/
    connect(m_keepaliveTimer, &QTimer::timeout,
            this, &OpenRGBAdalightPlugin::OnKeepaliveTimer);
    m_keepaliveTimer->setInterval(2000);
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
    info.Description    = "Support for Adalight/Ardulight devices via serial interface";
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
| Called when the plugin is loaded                          |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::Load(ResourceManagerInterface* resource_manager_ptr)
{
    m_resourceManager = resource_manager_ptr;

    /*-----------------------------------------------------*\
    | Retrieve configuration path                           |
    \*-----------------------------------------------------*/
    m_configPath = m_resourceManager->GetConfigurationDirectory().string();

    /*-----------------------------------------------------*\
    | Create widget                                         |
    \*-----------------------------------------------------*/
    m_widget = new AdalightWidget();

    /*-----------------------------------------------------*\
    | Load settings                                         |
    \*-----------------------------------------------------*/
    m_widget->LoadSettings(m_configPath);

    /*-----------------------------------------------------*\
    | Connect widget signals                                |
    \*-----------------------------------------------------*/
    connect(m_widget, &AdalightWidget::ReconnectRequested,
            this, &OpenRGBAdalightPlugin::OnReconnectRequested);

    connect(m_widget, &AdalightWidget::SettingsChanged,
            this, &OpenRGBAdalightPlugin::OnSettingsChanged);

    /*-----------------------------------------------------*\
    | Register callback for when new controllers are found  |
    \*-----------------------------------------------------*/
    m_resourceManager->RegisterDeviceListChangeCallback(
        DeviceListChangedCallback, this);

    /*-----------------------------------------------------*\
    | Automatically connect on startup                      |
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
| Called when the plugin is unloaded                        |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::Unload()
{
    /*-----------------------------------------------------*\
    | Stop keepalive timer                                  |
    \*-----------------------------------------------------*/
    m_keepaliveTimer->stop();

    /*-----------------------------------------------------*\
    | Unregister callbacks                                  |
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
    | Save settings                                         |
    \*-----------------------------------------------------*/
    if (m_widget != nullptr)
    {
        m_widget->SaveSettings(m_configPath);
    }

    /*-----------------------------------------------------*\
    | Disconnect device                                     |
    \*-----------------------------------------------------*/
    Disconnect();
}

/*---------------------------------------------------------*\
| Connect                                                   |
| Connects to the Adalight device                           |
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
        m_widget->setStatus("No COM port specified");
        return;
    }

    Disconnect();

    bool ok = m_device->Open(portName, baudRate);

    if (ok)
    {
        m_widget->setStatus("Connected to " + QString::fromStdString(portName));
        OnDeviceListChanged();

        /*-------------------------------------------------*\
        | Start keepalive timer                             |
        \*-------------------------------------------------*/
        m_keepaliveTimer->start();
    }
    else
    {
        m_widget->setStatus("Connection failed: " + QString::fromStdString(portName));
    }
}

/*---------------------------------------------------------*\
| Disconnect                                                |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::Disconnect()
{
    m_keepaliveTimer->stop();

    if (m_device->IsOpen())
    {
        int ledCount = (m_widget != nullptr) ? m_widget->GetLedCount() : 1;
        m_device->TurnOff(ledCount);
        m_device->Close();

        if (m_widget != nullptr)
        {
            m_widget->setStatus("Not connected");
        }
    }
}

/*---------------------------------------------------------*\
| SendColorsToDevice                                        |
| Reads all colors from the OpenRGB controllers and sends   |
| them to the Adalight device                               |
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
    | Collect colors from all registered controllers        |
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
    | If fewer colors than LEDs: fill remainder with black  |
    \*-----------------------------------------------------*/
    while ((int)allColors.size() < ledCount)
    {
        allColors.push_back(0);
    }

    m_device->SendColors(allColors, ledCount, colorOrder);
}

/*---------------------------------------------------------*\
| OnKeepaliveTimer                                          |
| Periodically resends the current colors to prevent the    |
| Adalight firmware from turning off the LEDs               |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::OnKeepaliveTimer()
{
    SendColorsToDevice();
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
| Registers update callbacks for all new controllers        |
\*---------------------------------------------------------*/
void OpenRGBAdalightPlugin::OnDeviceListChanged()
{
    if (m_resourceManager == nullptr)
    {
        return;
    }

    /*-----------------------------------------------------*\
    | Unregister old callbacks                              |
    \*-----------------------------------------------------*/
    for (RGBController* controller : m_registeredControllers)
    {
        controller->UnregisterUpdateCallback(this);
    }
    m_registeredControllers.clear();

    /*-----------------------------------------------------*\
    | Register new callbacks for all controllers            |
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
| Called when a controller updates its colors               |
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
