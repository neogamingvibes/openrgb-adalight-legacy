/*---------------------------------------------------------*\
| AdalightWidget.h                                          |
|                                                           |
|   UI widget for the Adalight plugin                       |
|   Settings: COM port, baud rate, LED count,               |
|   color order, reconnect button                           |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QString>
#include "AdalightDevice.h"

/*---------------------------------------------------------*\
| AdalightWidget Class                                      |
\*---------------------------------------------------------*/
class AdalightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdalightWidget(QWidget* parent = nullptr);
    ~AdalightWidget();

    /*-----------------------------------------------------*\
    | Load / save settings                                  |
    \*-----------------------------------------------------*/
    void        LoadSettings(const std::string& configPath);
    void        SaveSettings(const std::string& configPath);

    /*-----------------------------------------------------*\
    | Getters for current settings                          |
    \*-----------------------------------------------------*/
    std::string GetPortName()   const;
    int         GetBaudRate()   const;
    int         GetLedCount()   const;
    ColorOrder  GetColorOrder() const;

    /*-----------------------------------------------------*\
    | Display status message                                |
    \*-----------------------------------------------------*/
    void        setStatus(const QString& status);

signals:
    void        SettingsChanged();
    void        ReconnectRequested();

private slots:
    void        OnReconnectClicked();
    void        OnSettingsChanged();

private:
    void        SetupUI();

    /*-----------------------------------------------------*\
    | UI elements                                           |
    \*-----------------------------------------------------*/
    QLineEdit*   m_portEdit;
    QComboBox*   m_baudrateCombo;
    QSpinBox*    m_ledCountSpin;
    QComboBox*   m_colorOrderCombo;
    QPushButton* m_reconnectButton;
    QLabel*      m_statusLabel;
};
