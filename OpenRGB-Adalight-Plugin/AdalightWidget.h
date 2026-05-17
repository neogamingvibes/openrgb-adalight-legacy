/*---------------------------------------------------------*\
| AdalightWidget.h                                          |
|                                                           |
|   UI Widget für das Adalight Plugin                       |
|   Einstellungen: COM-Port, Baudrate, LED Count,           |
|   Color Order, Reconnect Button                           |
\*---------------------------------------------------------*/

#pragma once

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
| AdalightWidget Klasse                                     |
\*---------------------------------------------------------*/
class AdalightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdalightWidget(QWidget* parent = nullptr);
    ~AdalightWidget();

    /*-----------------------------------------------------*\
    | Einstellungen laden / speichern                       |
    \*-----------------------------------------------------*/
    void        LoadSettings(const std::string& configPath);
    void        SaveSettings(const std::string& configPath);

    /*-----------------------------------------------------*\
    | Getter für aktuelle Einstellungen                     |
    \*-----------------------------------------------------*/
    std::string GetPortName()   const;
    int         GetBaudRate()   const;
    int         GetLedCount()   const;
    ColorOrder  GetColorOrder() const;

    /*-----------------------------------------------------*\
    | Status anzeigen                                       |
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
    | UI Elemente                                           |
    \*-----------------------------------------------------*/
    QLineEdit*   m_portEdit;
    QComboBox*   m_baudrateCombo;
    QSpinBox*    m_ledCountSpin;
    QComboBox*   m_colorOrderCombo;
    QPushButton* m_reconnectButton;
    QLabel*      m_statusLabel;
};
