/*---------------------------------------------------------*\
| OpenRGBAdalightPlugin.h                                   |
|                                                           |
|   OpenRGB Plugin für Adalight/Ardulight Geräte            |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <QObject>
#include <QMenu>
#include "OpenRGBPluginInterface.h"
#include "ResourceManagerInterface.h"
#include "RGBController.h"
#include "AdalightDevice.h"
#include "AdalightWidget.h"

class OpenRGBAdalightPlugin : public QObject, public OpenRGBPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OpenRGBPluginInterface_IID)
    Q_INTERFACES(OpenRGBPluginInterface)

public:
    OpenRGBAdalightPlugin();
    ~OpenRGBAdalightPlugin();

    /*-----------------------------------------------------*\
    | OpenRGBPluginInterface                                |
    \*-----------------------------------------------------*/
    OpenRGBPluginInfo   GetPluginInfo()         override;
    unsigned int        GetPluginAPIVersion()   override;

    void                Load(ResourceManagerInterface* resource_manager_ptr) override;
    QWidget*            GetWidget()             override;
    QMenu*              GetTrayMenu()           override;
    void                Unload()                override;

private slots:
    void                OnReconnectRequested();
    void                OnSettingsChanged();

private:
    void                Connect();
    void                Disconnect();
    void                SendColorsToDevice();

    /*-----------------------------------------------------*\
    | Callback wenn OpenRGB Controller aktualisiert wird    |
    \*-----------------------------------------------------*/
    static void         DeviceListChangedCallback(void* arg);
    void                OnDeviceListChanged();

    static void         ControllerUpdateCallback(void* arg);
    void                OnControllerUpdate();

    /*-----------------------------------------------------*\
    | Member Variablen                                      |
    \*-----------------------------------------------------*/
    ResourceManagerInterface*   m_resourceManager;
    AdalightDevice*             m_device;
    AdalightWidget*             m_widget;
    std::string                 m_configPath;

    std::vector<RGBController*> m_registeredControllers;
};
