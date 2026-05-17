/*---------------------------------------------------------*\
| AdalightWidget.cpp                                        |
|                                                           |
|   UI Widget für das Adalight Plugin                       |
\*---------------------------------------------------------*/

#include "AdalightWidget.h"
#include <QSettings>
#include <QString>
#include <QGroupBox>

AdalightWidget::AdalightWidget(QWidget* parent)
    : QWidget(parent)
{
    SetupUI();
}

AdalightWidget::~AdalightWidget()
{
}

/*---------------------------------------------------------*\
| SetupUI                                                   |
| Erstellt die UI Elemente                                  |
\*---------------------------------------------------------*/
void AdalightWidget::SetupUI()
{
    /*-----------------------------------------------------*\
    | Haupt Layout                                          |
    \*-----------------------------------------------------*/
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);

    /*-----------------------------------------------------*\
    | Gruppe: Verbindungseinstellungen                      |
    \*-----------------------------------------------------*/
    QGroupBox* connectionGroup = new QGroupBox("Adalight Verbindung", this);
    QFormLayout* formLayout = new QFormLayout(connectionGroup);

    /*-----------------------------------------------------*\
    | COM Port Eingabe                                      |
    \*-----------------------------------------------------*/
    m_portEdit = new QLineEdit(this);
    m_portEdit->setPlaceholderText("z.B. COM3");
    m_portEdit->setMaximumWidth(150);
    formLayout->addRow("COM Port:", m_portEdit);

    /*-----------------------------------------------------*\
    | Baudrate Auswahl                                      |
    \*-----------------------------------------------------*/
    m_baudrateCombo = new QComboBox(this);
    m_baudrateCombo->addItem("9600",    9600);
    m_baudrateCombo->addItem("19200",   19200);
    m_baudrateCombo->addItem("38400",   38400);
    m_baudrateCombo->addItem("57600",   57600);
    m_baudrateCombo->addItem("115200",  115200);
    m_baudrateCombo->addItem("230400",  230400);
    m_baudrateCombo->addItem("500000",  500000);
    m_baudrateCombo->addItem("1000000", 1000000);

    /*-----------------------------------------------------*\
    | Standard: 115200                                      |
    \*-----------------------------------------------------*/
    m_baudrateCombo->setCurrentText("115200");
    m_baudrateCombo->setMaximumWidth(150);
    formLayout->addRow("Baudrate:", m_baudrateCombo);

    /*-----------------------------------------------------*\
    | LED Count Eingabe                                     |
    \*-----------------------------------------------------*/
    m_ledCountSpin = new QSpinBox(this);
    m_ledCountSpin->setMinimum(1);
    m_ledCountSpin->setMaximum(1000);
    m_ledCountSpin->setValue(25);
    m_ledCountSpin->setMaximumWidth(150);
    formLayout->addRow("LED Anzahl:", m_ledCountSpin);

    /*-----------------------------------------------------*\
    | Color Order Auswahl                                   |
    \*-----------------------------------------------------*/
    m_colorOrderCombo = new QComboBox(this);
    m_colorOrderCombo->addItem("RGB", (int)ColorOrder::RGB);
    m_colorOrderCombo->addItem("RBG", (int)ColorOrder::RBG);
    m_colorOrderCombo->addItem("BRG", (int)ColorOrder::BRG);
    m_colorOrderCombo->addItem("BGR", (int)ColorOrder::BGR);
    m_colorOrderCombo->addItem("GRB", (int)ColorOrder::GRB);
    m_colorOrderCombo->addItem("GBR", (int)ColorOrder::GBR);
    m_colorOrderCombo->setMaximumWidth(150);
    formLayout->addRow("Farbreihenfolge:", m_colorOrderCombo);

    mainLayout->addWidget(connectionGroup);

    /*-----------------------------------------------------*\
    | Reconnect Button und Status                           |
    \*-----------------------------------------------------*/
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_reconnectButton = new QPushButton("Verbinden / Neu verbinden", this);
    m_reconnectButton->setMaximumWidth(250);
    buttonLayout->addWidget(m_reconnectButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    /*-----------------------------------------------------*\
    | Status Label                                          |
    \*-----------------------------------------------------*/
    m_statusLabel = new QLabel("Status: Nicht verbunden", this);
    mainLayout->addWidget(m_statusLabel);

    /*-----------------------------------------------------*\
    | Signals verbinden                                     |
    \*-----------------------------------------------------*/
    connect(m_reconnectButton, &QPushButton::clicked,
            this, &AdalightWidget::OnReconnectClicked);

    connect(m_portEdit,        &QLineEdit::textChanged,
            this, &AdalightWidget::OnSettingsChanged);

    connect(m_baudrateCombo,   QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdalightWidget::OnSettingsChanged);

    connect(m_ledCountSpin,    QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AdalightWidget::OnSettingsChanged);

    connect(m_colorOrderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdalightWidget::OnSettingsChanged);
}

/*---------------------------------------------------------*\
| LoadSettings                                              |
| Lädt die gespeicherten Einstellungen                      |
\*---------------------------------------------------------*/
void AdalightWidget::LoadSettings(const std::string& configPath)
{
    QString path = QString::fromStdString(configPath) + "/Adalight.ini";
    QSettings settings(path, QSettings::IniFormat);

    m_portEdit->setText(settings.value("port", "COM3").toString());

    QString baudrate = settings.value("baudrate", "115200").toString();
    int idx = m_baudrateCombo->findText(baudrate);
    if (idx >= 0)
    {
        m_baudrateCombo->setCurrentIndex(idx);
    }

    m_ledCountSpin->setValue(settings.value("ledcount", 25).toInt());

    int colorOrderIdx = settings.value("colororder", 0).toInt();
    if (colorOrderIdx >= 0 && colorOrderIdx < m_colorOrderCombo->count())
    {
        m_colorOrderCombo->setCurrentIndex(colorOrderIdx);
    }
}

/*---------------------------------------------------------*\
| SaveSettings                                              |
| Speichert die aktuellen Einstellungen                     |
\*---------------------------------------------------------*/
void AdalightWidget::SaveSettings(const std::string& configPath)
{
    QString path = QString::fromStdString(configPath) + "/Adalight.ini";
    QSettings settings(path, QSettings::IniFormat);

    settings.setValue("port",       QString::fromStdString(GetPortName()));
    settings.setValue("baudrate",   m_baudrateCombo->currentText());
    settings.setValue("ledcount",   GetLedCount());
    settings.setValue("colororder", m_colorOrderCombo->currentIndex());
}

/*---------------------------------------------------------*\
| Getter                                                    |
\*---------------------------------------------------------*/
std::string AdalightWidget::GetPortName() const
{
    return m_portEdit->text().toStdString();
}

int AdalightWidget::GetBaudRate() const
{
    return m_baudrateCombo->currentData().toInt();
}

int AdalightWidget::GetLedCount() const
{
    return m_ledCountSpin->value();
}

ColorOrder AdalightWidget::GetColorOrder() const
{
    return (ColorOrder)m_colorOrderCombo->currentData().toInt();
}

/*---------------------------------------------------------*\
| setStatus                                                 |
| Zeigt den Verbindungsstatus an                            |
\*---------------------------------------------------------*/
void AdalightWidget::setStatus(const QString& status)
{
    m_statusLabel->setText("Status: " + status);
}

/*---------------------------------------------------------*\
| Slots                                                     |
\*---------------------------------------------------------*/
void AdalightWidget::OnReconnectClicked()
{
    emit ReconnectRequested();
}

void AdalightWidget::OnSettingsChanged()
{
    emit SettingsChanged();
}
