#include "garuda.h"
#include "ui_garuda.h"

#include <QtCharts/QChartView>
#include <gmap.h>
#include <QMainWindow>
#include <QtCharts/QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QQuickWidget>
#include <QtSerialPort/QSerialPortInfo>
#include <QtQuick/QQuickView>
#include <QtSerialPort/QSerialPort>
#include <QQmlContext>
#include <ledc.h>
#include <bar.h>
#include <QDebug>
#include <backend.h>
#include <qcustomplot.h>
#include <QtMqtt/QMqttClient>
#include <QFile>
#include <QtMath>


gmap locator;
QFile file("./Serial_Data.csv");


GARUDA::GARUDA(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GARUDA)
{
    ui->setupUi(this);
    ui->map->setSource(QUrl::fromLocalFile("../TEAM_GARUDA_GCS/maps/maps/main.qml"));
    ui->map->rootContext()->setContextProperty("gmap",&locator);
    ui->map->show();

    configureGraph();
    arduino = new QSerialPort();
    m_client = new QMqttClient();

    // configure serial
    configureSerial();
    connectSerial();

    //mqtt initialization
    configureMqtt();
    connectMqtt();
}

GARUDA::~GARUDA()
{
    delete ui;
}


void GARUDA::configureGraph()
{
//    ui->sp1Pressure->addGraph();
    ui->sp1Altitude->addGraph();
    ui->sp1Rotation->addGraph();
    ui->sp1PacketCount->addGraph();

//    graphSetting(ui->sp1Pressure);
    graphSetting(ui->sp1Altitude);
    graphSetting(ui->sp1Rotation);
    graphSetting(ui->sp1PacketCount);

//    ui->sp2Pressure->addGraph();
    ui->sp2Altitude->addGraph();
    ui->sp2Rotation->addGraph();
    ui->sp2PacketCount->addGraph();

//    graphSetting(ui->sp2Pressure);
    graphSetting(ui->sp2Altitude);
    graphSetting(ui->sp2Rotation);
    graphSetting(ui->sp2PacketCount);

    ui->containerAltitude->addGraph();
    ui->containerVoltage->addGraph();
    ui->containerTemperature->addGraph();
    ui->containerPacketCount->addGraph();

    graphSetting(ui->containerAltitude);
    graphSetting(ui->containerVoltage);
    graphSetting(ui->containerTemperature);
    graphSetting(ui->containerPacketCount);

    ui->gpsaltitude->addGraph();
    graphSetting(ui->gpsaltitude);

}

void GARUDA::graphSetting(QCustomPlot *p)
{
    QColor col = QColor(233, 250, 255);
    QColor text = QColor(46, 27, 58);
    p->setBackground(col);
    p->graph(0)->setScatterStyle(QCPScatterStyle::ssDisc);
    p->graph(0)->setLineStyle(QCPGraph::lsLine);
    QPen pen;
    pen.setColor(text);
    p->xAxis->setBasePen(pen);
    p->xAxis->setTickLabelColor(text);
    p->yAxis->setBasePen(pen);
    p->yAxis->setTickLabelColor(text);
    p->xAxis->grid()->setPen(Qt::NoPen);
    p->yAxis->grid()->setPen(Qt::NoPen);
    p->xAxis->grid()->setSubGridVisible(false);
    p->yAxis->grid()->setSubGridVisible(false);

}

void GARUDA::updateGraphArray(QStringList sensorData)
{
    // 7,9,23,24,25,30,31,32,p(23),p(30);
    graphTime.append(count++);

    CAltitude.append(sensorData[7].toDouble());
    CVoltage.append(sensorData[9].toDouble());
    CTemp.append(sensorData[8].toDouble());
    CPacket.append(sensorData[2].toDouble());

    SP1Altitude.append(sensorData[23].toDouble());
    SP1TEMP.append(sensorData[24].toDouble());
    SP1Rotation.append(sensorData[25].toDouble());
    SP1PacketCount.append(sensorData[21].toDouble());

    SP2Altitude.append(sensorData[30].toDouble());
    SP2TEMP.append(sensorData[31].toDouble());
    SP2Rotation.append(sensorData[32].toDouble());
    SP2PacketCount.append(sensorData[28].toDouble());

    GPSALTITUDE.append(sensorData[13].toDouble());
}

void GARUDA::plot()
{
    ui->sp1Altitude->graph(0)->setData(graphTime, SP1Altitude);
    ui->sp1Altitude->replot();
    ui->sp1Altitude->xAxis->rescale(true);
    ui->sp1Altitude->yAxis->rescale(true);

    ui->sp2Altitude->graph()->setData(graphTime, SP2Altitude);
    ui->sp2Altitude->replot();
    ui->sp2Altitude->xAxis->rescale(true);
    ui->sp2Altitude->yAxis->rescale(true);

    ui->sp1PacketCount->graph()->setData(graphTime, SP1PacketCount);
    ui->sp1PacketCount->replot();
    ui->sp1PacketCount->xAxis->rescale(true);
    ui->sp1PacketCount->yAxis->rescale(true);


    ui->sp2PacketCount->graph()->setData(graphTime, SP2PacketCount);
    ui->sp2PacketCount->replot();
    ui->sp2PacketCount->xAxis->rescale(true);
    ui->sp2PacketCount->yAxis->rescale(true);


    ui->sp1Rotation->graph()->setData(graphTime, SP1Rotation);
    ui->sp1Rotation->replot();
    ui->sp1Rotation->xAxis->rescale(true);
    ui->sp1Rotation->yAxis->rescale(true);

    ui->sp2Rotation->graph()->setData(graphTime, SP2Rotation);
    ui->sp2Rotation->replot();
    ui->sp2Rotation->xAxis->rescale(true);
    ui->sp2Rotation->yAxis->rescale(true);

    ui->containerAltitude->graph()->setData(graphTime, CAltitude);
    ui->containerAltitude->replot();
    ui->containerAltitude->xAxis->rescale(true);
    ui->containerAltitude->yAxis->rescale(true);

    ui->containerTemperature->graph()->setData(graphTime, CTemp);
    ui->containerTemperature->replot();
    ui->containerTemperature->xAxis->rescale(true);
    ui->containerTemperature->yAxis->rescale(true);

    ui->containerPacketCount->graph()->setData(graphTime, CPacket);
    ui->containerPacketCount->replot();
    ui->containerPacketCount->xAxis->rescale(true);
    ui->containerPacketCount->yAxis->rescale(true);

    ui->containerVoltage->graph()->setData(graphTime, CVoltage);
    ui->containerVoltage->replot();
    ui->containerVoltage->xAxis->rescale(true);
//    ui->containerVoltage->yAxis->setRange(8.0, 9.0);
    ui->containerVoltage->yAxis->rescale(true);


    ui->gpsaltitude->graph()->setData(graphTime, GPSALTITUDE);
    ui->gpsaltitude->replot();
    ui->gpsaltitude->xAxis->rescale(true);
    ui->gpsaltitude->yAxis->rescale(true);

}

void GARUDA::setPrecision()
{
    // 7, 8, 9, 13, 23, 24, 25, 30, 31, 32
    sensorData[7] = QString::number(sensorData[7].toDouble(), 'f', 1);
    sensorData[8] = QString::number(sensorData[8].toDouble(), 'f', 1);
    sensorData[9] = QString::number(sensorData[9].toDouble(), 'f', 2);
    sensorData[13] = QString::number(sensorData[13].toDouble(), 'f', 1);
    sensorData[23] = QString::number(sensorData[23].toDouble(), 'f', 1);
    sensorData[24] = QString::number(sensorData[24].toDouble(), 'f', 1);
    sensorData[25] = QString::number(sensorData[25].toDouble(), 'f', 2);
    sensorData[30] = QString::number(sensorData[30].toDouble(), 'f', 1);
    sensorData[31] = QString::number(sensorData[31].toDouble(), 'f', 1);
    sensorData[32] = QString::number(sensorData[32].toDouble(), 'f', 2);


}

void GARUDA::updateContainerLabel(QStringList sensorData)
{
    ui->teamIDval->setText(sensorData[0]);
    ui->missionTimeval->setText(sensorData[1]);
    ui->packetCountval->setText(sensorData[2]);
    ui->packetTypeval->setText(sensorData[3]);
    ui->modeval->setText(sensorData[4]);
    ui->sp1Releasedval->setText(sensorData[5]);
    ui->sp2Releasedval->setText(sensorData[6]);
    ui->AltitudeVal->setText(sensorData[7]);
    ui->TempVal->setText(sensorData[8]);
    ui->BatteryVoltageVal->setText(sensorData[9]);
    ui->gpsTimeval->setText(sensorData[10]);
    ui->gpsLatitudeval->setText(sensorData[11]);
    ui->gpsLongitudeval->setText(sensorData[12]);
    ui->gpsAltitudeval->setText(sensorData[13]);
    ui->gpsStateval->setText(sensorData[14]);
    ui->softwareStateval->setText(sensorData[15]);
    ui->SP1PacketCVal->setText(sensorData[16]);
    ui->SP2PacketCVal->setText(sensorData[17]);
    ui->cmdEchoval->setText(sensorData[18]);
}

void GARUDA::updatePayload1Label(QStringList sensorData)
{
    // 21,23,24,25,p(23)
    ui->SP1PACKETCOUNT->setText(sensorData[21]);
    ui->SP1ALTITUDE->setText(sensorData[23]);
    ui->SP1TEMPERATURE->setText(sensorData[24]);
    ui->SP1ROTATION->setText(sensorData[25]);
    ui->SP1PRESSURE->setText(QString::number(altitudeToPressure(sensorData[23])));
}

void GARUDA::updatePayload2Label(QStringList sensorData)
{
    //28,30,31,32, p(30)
    ui->SP2PACKETCOUNT->setText(sensorData[28]);
    ui->SP2ALTITUDE->setText(sensorData[30]);
    ui->SP2TEMPERATURE->setText(sensorData[31]);
    ui->SP2ROTATION->setText(sensorData[32]);
    ui->SP2PRESSURE->setText(QString::number(altitudeToPressure(sensorData[30])));

}

void GARUDA::updateLed(QString state)
{
    ui->state1->toggle(0);
    ui->state2->toggle(0);
    ui->state3->toggle(0);
    ui->state4->toggle(0);
    ui->state5a->toggle(0);
    ui->state5b->toggle(0);
    ui->state6->toggle(0);
    int st = state.toInt();
    switch(st) {
    case 1: ui->state1->toggle(1);
        break;
    case 2: ui->state2->toggle(1);
        break;
    case 3: ui->state3->toggle(1);
        break;
    case 4: ui->state4->toggle(1);
        break;
    case 5: ui->state5a->toggle(1);
        break;
    case 6: ui->state5b->toggle(1);
        break;
    case 7: ui->state6->toggle(1);
        break;
    }
}

void GARUDA::updateBar(bar *b, QString val)
{
    int value = val.toInt();
    b->getpos(value);
}

void GARUDA::convertTelemetry()
{
    if(sensorData.length() >= 11 && sensorData[9] != "-") {
        QDateTime date_time = QDateTime::currentDateTime();
        QString temp = date_time.time().toString("hh:mm:ss");
        sensorData[10] = temp;
    }
}

void GARUDA::writeCSV(QString telemetry)
{
    if(file.open(QFile::WriteOnly | QFile::Append)) {
        QTextStream stream(&file);
        stream << telemetry << "\n";
    }
    file.close();
}

bool GARUDA::checkArduinoConnection()
{
    bool arduino_is_available = false;
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        //  check if the serialport has both a product identifier and a vendor identifier
        if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier()){
            //  check if the product ID and the vendor ID match those of the arduino uno
            if((serialPortInfo.productIdentifier() ==  arduino_uno_product_id)
                    && (serialPortInfo.vendorIdentifier() == arduino_uno_vendor_id)){
                arduino_is_available = true; //    arduino uno is available on this port
                arduino_uno_port_name = serialPortInfo.portName();
            }

        }
    }
    return arduino_is_available;
}

void GARUDA::configureSerial()
{
    if(checkArduinoConnection()) {
        qDebug() << "Found the arduino port...\n";
        arduino->setPortName(arduino_uno_port_name);
        arduino->open(QSerialPort::ReadWrite);
        arduino->setBaudRate(QSerialPort::Baud9600);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setReadBufferSize(5000);

    }
    else {
        QMessageBox::information(this, "Serial Port Error", "Couldn't open serial port to arduino.");
    }
}

void GARUDA::connectSerial()
{
    isConectedSerial = true;
    QObject::connect(arduino, SIGNAL(readyRead()), this, SLOT(readSerial()));
}

void GARUDA::disconnectSerial()
{
    QObject::disconnect(arduino, SIGNAL(readyRead()), this, SLOT(readSerial()));
}

void GARUDA::configureMqtt()
{
    m_client->setProtocolVersion(QMqttClient::MQTT_3_1_1);
    m_client->setPort(1883);
    m_client->setHostname("cansat.info");
    m_client->setClientId("Team G.A.R.U.D.A");
    m_client->setUsername("3394");
    m_client->setPassword("Jeorgifa191!");
}

void GARUDA::connectMqtt()
{
    m_client->connectToHost();
}

void GARUDA::publishMqtt(QStringList sensorData)
{
    QString container = "";
    for(int i=0; i<18; i++)
        container += sensorData[i] + ",";
    container += sensorData[18];

    QString payload1 = "";
    for(int i = 19; i<26; i++)
        payload1 += sensorData[i] + ",";
    payload1 += ",,,,,,,,,,,";

    QString payload2 = "";

    for(int i=26; i<33; i++)
        payload2 += sensorData[i] + ",";
    payload2 += ",,,,,,,,,,,";

    m_client->publish(topic, container.toUtf8());

    if(sensorData[20] != "-")
        m_client->publish(topic, payload1.toUtf8());
    if(sensorData[27] != "-")
        m_client->publish(topic, payload2.toUtf8());

}


void GARUDA::writeSerial(QString cmd)
{
    if(CX) {
        arduino->write(cmd.toLocal8Bit());
    }
}

double GARUDA::altitudeToPressure(QString altitude)
{
    pressure = 101325*qPow((1 - (altitude.toDouble()/44330)), 5.33);
    return pressure;
}

QString c = "1";
void GARUDA::readSerial()
{
    if(CX) {
        QStringList buffer = serialBuffer.split("\r\n");

        // check if complete telemetry is recieved
        if(buffer.length() > 1) {
            // clearing buffer;
            serialBuffer = "";
            telemetry = buffer[0];
            telemetry = telemetry.replace("><", ",").replace("<","").replace(">","");
            sensorData = telemetry.split(",");

            if(sensorData.size() == 14) {
                telemetry = "-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-," + telemetry;
                sensorData = telemetry.split(",");
            }

            if(sensorData.length() > 22 && sensorData[22] == "S1")
                sensorData[22] = "SP1";
            if(sensorData.length() > 29 && sensorData[29] == "S2")
                sensorData[22] = "SP2";

            //convert telemetry to desired format
            convertTelemetry();
            telemetry = sensorData.join(',');
            ui->telemetryDisplay->setPlainText(telemetry);

            if(sensorData.length() != 33) {
                sensorData = lastCorrectTelemetry;
            }


            telemetry = sensorData.join(',');

            // write data to csv file
            writeCSV(telemetry);

//            qDebug() << telemetry;
            // telemetry recieved from 1 container + 2 payloads
            if(sensorData.length() == 33) {
                // telemetry is complete and without loss
                //setting Precision
                setPrecision();

                // publish telemetry to mqtt
                if(startMqtt)
                    publishMqtt(sensorData);


                updateGraphArray(sensorData);

                //plot telemetry on graphs
                plot();

                //show data on labels;
                updateContainerLabel(sensorData);

                // update map
                locator.setData(sensorData[11], sensorData[12]);

                updateLed(sensorData[15]);

                updatePayload1Label(sensorData);
                updatePayload2Label(sensorData);

                ui->sp1Temp->getpos(sensorData[24].toDouble());
                ui->sp2Temp->getpos(sensorData[31].toDouble());

                lastCorrectTelemetry = sensorData;
            }
        }

        serial = arduino->readAll();
        serialBuffer += serial;
    }
}




void GARUDA::on_SETTIME_clicked()
{
    if(isConectedSerial) {
        QDateTime date_time = QDateTime::currentDateTimeUtc();
        QString time = "C";
        qDebug()<< time;
        writeSerial(time);
        QThread::sleep(1);
        time = "<ST,";
        time += QString::number(date_time.toTime_t());
        time += ">";
        qDebug()<< time;
        writeSerial(time);
    }

}


void GARUDA::on_SIMENABLE_clicked()
{
    if(isConectedSerial) {
        QString string = "F";
        qDebug() << string;
        writeSerial(string);
        simEnable = true;

    }

}


void GARUDA::on_SIMACTIVATE_clicked()
{
    if(isConectedSerial) {
        QString string = "G";
        qDebug() << string;
        writeSerial(string);
        simActivate = true;
    }

}

void GARUDA::on_SIMP_clicked()
{
    if(simEnable && simActivate) {
        QFile simpFile("./CanSat_2021_sim_file_A.txt");

        if(!simpFile.open(QIODevice::ReadOnly))
            qDebug() << "cound not open file";
        else {
            while(!simpFile.atEnd()) {
                QString data = simpFile.readLine();
                if(data[0] == 'C' && data[1] == 'M' && data[2] == 'D' && data[3] == ',') {
                    QStringList temp = data.split(',');
                    temp[3].chop(1);
                    QString finalSimp = "<SIMP,";
                    finalSimp += temp[3];
                    finalSimp += ">";
                    simpList.append(finalSimp);
                }
            }
            simpFile.close();

            timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(sendSimp()));
            timer->start(1000);
        }
    }
}

void GARUDA::sendSimp()
{
    if(simpIndex < simpList.size()) {
        QString simpstr = simpList[simpIndex++];

        qDebug() << simpstr;
        if(isConectedSerial)
            writeSerial(simpstr);
    }
}



void GARUDA::on_CX_clicked()
{
    if(!CX) {
        ui->CX->setText("CX OFF");
        CX = true;

        QString string = "A";
        qDebug() << string;
        writeSerial(string);


        if (file.open(QFile::WriteOnly|QFile::Truncate)) {
            QTextStream stream(&file);
            stream << "<TEAM_ID>,<MISSION_TIME>,<PACKET_COUNT>,<PACKET_TYPE>,<MODE>,<SP1_RELEASED>,<SP2_RELEASED>,<ALTITUDE>,<TEMP>,<VOLTAGE>,<GPS_TIME>,<GPS_LATITUDE>,<GPS_LONGITUDE>,<GPS_ALTITUDE>,<GPS_SATS>,<SOFTWARE_STATE>,<SP1_PACKET_COUNT>,<SP2_PACKET_COUNT>,<CMD_ECHO>,<TEAM_ID>,<MISSION_TIME>,<PACKET_COUNT>,<PACKET_TYPE>,<SP_ALTITUDE>,<SP_TEMP>,<SP_ROTATION_RATE>,<TEAM_ID>,<MISSION_TIME>,<PACKET_COUNT>,<PACKET_TYPE>,<SP_ALTITUDE>,<SP_TEMP>,<SP_ROTATION_RATE>" << "\n";
        }
        file.close();
    }
    else {
        ui->CX->setText("CX ON");
        CX = false;
        QString string = "H";
        qDebug() << string;
        writeSerial(string);
//        disconnectSerial();
    }
}

void GARUDA::on_MQTT_clicked()
{
    if(!startMqtt) {
        ui->MQTT->setText("DISABLE MQTT");
        startMqtt = true;
    }
    else {
        ui->MQTT->setText("ENABLE MQTT");
        startMqtt = false;
    }
}

void GARUDA::on_SP1X_clicked()
{
    if(isConectedSerial) {
        if(!SP1X) {
            ui->SP1X->setText("SP1X OFF");
            QString string = "D";
            qDebug() << string;
            writeSerial(string);
            SP1X = true;
        }
        else {
            ui->SP1X->setText("SP1X ON");
            QString string = "I";
            qDebug() << string;
            writeSerial(string);
            SP1X = false;
        }
    }

}

void GARUDA::on_SP2X_clicked()
{
    if(isConectedSerial) {
        if(!SP2X) {
            ui->SP2X->setText("SP2X OFF");
            QString string = "E";
            qDebug() << string;
            writeSerial(string);
            SP2X = true;
        }
        else {
            ui->SP2X->setText("SP2X ON");
            QString string = "J";
            qDebug() << string;
            writeSerial(string);
            SP2X = false;
        }
    }
}

void GARUDA::on_CALIBRATION_clicked()
{
    if(isConectedSerial) {
        QString string = "B";
        qDebug() << string;
        writeSerial(string);
    }
}



void GARUDA::readSampleCSV()
{
    QFile d("./samplecsv.csv");
    qDebug() << "CSV printing";


    if(d.open(QFile::ReadOnly)) {
        while(!d.atEnd()) {
            QString s = d.readLine();
            plotTest.append(s);
        }
    }

    for(int i=0; i<7140; i++) {
        QStringList a = plotTest[i].split("\r\n");
        sensorData = a[0].split(",");
        if(sensorData.length() > 22 && sensorData[22] == "S1")
            sensorData[22] = "SP1";
        if(sensorData.length() > 29 && sensorData[29] == "S2")
            sensorData[22] = "SP2";

        writeCSV(a[0]);

        ui->telemetryDisplay->setPlainText(sensorData.join(','));

        //setting Precision
        setPrecision();


        // publish telemetry to mqtt
        if(startMqtt)
            publishMqtt(sensorData);


        updateGraphArray(sensorData);

        //plot telemetry on graphs
        plot();

        //show data on labels;
        updateContainerLabel(sensorData);

        // update map
        locator.setData(sensorData[11], sensorData[12]);

        updateLed(sensorData[15]);

        updatePayload1Label(sensorData);
        updatePayload2Label(sensorData);

        ui->sp1Temp->getpos(sensorData[24].toDouble());
        ui->sp2Temp->getpos(sensorData[31].toDouble());
    }

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(plotSec()));
    timer->start(1000);
}

int loop1 = 7140;
void GARUDA::plotSec()
{
    if(loop1 < plotTest.size()) {
        QStringList a = plotTest[loop1++].split("\r\n");
        sensorData = a[0].split(",");
        writeCSV(a[0]);

        ui->telemetryDisplay->setPlainText(sensorData.join(','));

        //setting Precision
        setPrecision();


        // publish telemetry to mqtt
        if(startMqtt)
            publishMqtt(sensorData);


        updateGraphArray(sensorData);

        //plot telemetry on graphs
        plot();

        //show data on labels;
        updateContainerLabel(sensorData);

        // update map
        locator.setData(sensorData[11], sensorData[12]);

        updateLed(sensorData[15]);

        updatePayload1Label(sensorData);
        updatePayload2Label(sensorData);

        ui->sp1Temp->getpos(sensorData[24].toDouble());
        ui->sp2Temp->getpos(sensorData[31].toDouble());
    }
}

void GARUDA::on_Check_clicked()
{
    if(isConectedSerial) {
        QString temp = "O";
        qDebug() << temp;
        writeSerial(temp);
    }

}

void GARUDA::on_Reset_clicked()
{
    if(isConectedSerial) {
        QString temp = "K";
        qDebug() << temp;
        writeSerial(temp);
    }
}

void GARUDA::on_SIM_DISABLE_clicked()
{
    if(isConectedSerial) {
        QString temp = "<SIMDISABLE>";
        qDebug() << temp;
        writeSerial(temp);
        timer->stop();
    }
}
