#ifndef DELIVERYVIEWER_H
#define DELIVERYVIEWER_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "deliveryplanner.h"

enum StepSelection{
    deliverySelection,
    pickupSelection,
    deliveryPlan
};

QT_BEGIN_NAMESPACE
namespace Ui { class DeliveryViewer; }
QT_END_NAMESPACE

class DeliveryViewer : public QMainWindow
{
    Q_OBJECT

public:
    DeliveryViewer(QWidget *parent = nullptr);
    ~DeliveryViewer();

private:
    Ui::DeliveryViewer *ui; // Main Window
    DeliveryPlanner *deliveryPlanner; // Is respnsible for calculating the planned route
    StepSelection currentStep; // Current step
    uint plottedDeliveryPlans; // number of plotted plans
    QVector<QCPItemLine*> lines; // lines of the delivery plans
    void UpdateStepLabel(); // Updates the instruction label for the user
    void NewDeliveryPlot(QVector<double> xPlanned, QVector<double> yPlanned, double length); // Plots a new delivery plan
    void AddStandardGraphs(); // Plots the labels for the delivery, pickup and depot point
private slots:
    void AddPoint(QMouseEvent *event); // User double clicks a point as a delivery/pickup point
    void StepBack(); // User steps one step back
    void StepContinue(); // User continues
    void selectionChanged();
    void mousePress();
    void mouseWheel();
    void removeAllGraphs();
    void contextMenuRequest(QPoint pos);
    void moveLegend();
};
#endif // DELIVERYVIEWER_H
