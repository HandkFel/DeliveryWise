// Used: QCustomPlot and examples from https://www.qcustomplot.com/

#include "deliveryviewer.h"
#include "ui_deliveryviewer.h"

DeliveryViewer::DeliveryViewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DeliveryViewer)
    , deliveryPlanner(new DeliveryPlanner())
    , currentStep(StepSelection::deliverySelection)
    , plottedDeliveryPlans(0)
{
    // Setup the gui and the plot
    ui->setupUi(this);

    srand(QDateTime::currentDateTime().toTime_t());

    ui->deliveryPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);
    ui->deliveryPlot->xAxis->setRange(-8, 8);
    ui->deliveryPlot->yAxis->setRange(-5, 5);
    ui->deliveryPlot->axisRect()->setupFullAxesBox();

    ui->deliveryPlot->plotLayout()->insertRow(0);
    QCPTextElement *title = new QCPTextElement(ui->deliveryPlot, "Delivery Plan", QFont("sans", 17, QFont::Bold));
    ui->deliveryPlot->plotLayout()->addElement(0, 0, title);

    ui->deliveryPlot->xAxis->setLabel("x Axis");
    ui->deliveryPlot->yAxis->setLabel("y Axis");
    ui->deliveryPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->deliveryPlot->legend->setFont(legendFont);
    ui->deliveryPlot->legend->setSelectedFont(legendFont);
    ui->deliveryPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

    QObject::connect(ui->deliveryPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(AddPoint(QMouseEvent*)));

    // setup policy and connect slot for context menu popup:
    ui->deliveryPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->deliveryPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    // connect slot that ties some axis selections together (especially opposite axes):
    connect(ui->deliveryPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->deliveryPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->deliveryPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    // add data point graph for the delivery/pickup points:
    AddStandardGraphs();

    // Initialize the step label
    UpdateStepLabel();

    // Connect the signals so that the user can step back and continue
    QObject::connect(ui->btnBack, SIGNAL(pressed()), this, SLOT(StepBack()));
    QObject::connect(ui->btnContinue, SIGNAL(pressed()), this, SLOT(StepContinue()));
}

void DeliveryViewer::AddStandardGraphs(){
    ui->deliveryPlot->addGraph();
    ui->deliveryPlot->graph(0)->setPen(QPen(Qt::black));
    ui->deliveryPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    ui->deliveryPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 12));
    ui->deliveryPlot->graph(0)->setName("Delivery Points");

    ui->deliveryPlot->addGraph();
    ui->deliveryPlot->graph(1)->setPen(QPen(Qt::red));
    ui->deliveryPlot->graph(1)->setLineStyle(QCPGraph::lsNone);
    ui->deliveryPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 12));
    ui->deliveryPlot->graph(1)->setName("Pickup Points");

    ui->deliveryPlot->addGraph();
    ui->deliveryPlot->graph(2)->setPen(QPen(Qt::green));
    ui->deliveryPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->deliveryPlot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 12));
    ui->deliveryPlot->graph(2)->setName("Depot");
    ui->deliveryPlot->graph(2)->setData(deliveryPlanner->xDepot, deliveryPlanner->yDepot);
}

DeliveryViewer::~DeliveryViewer()
{
    delete ui;
    delete deliveryPlanner;

    for (auto const& i : lines) {
        delete i;
    }
}

// Gets triggered when user double clicks the plot
void DeliveryViewer::AddPoint(QMouseEvent *event){
    // Get coordinates of input
    double x; double y;
    if(event->button() == Qt::LeftButton)
       {
            if(ui->deliveryPlot->axisRect()->rect().contains(event->pos()))
            {
                x = ui->deliveryPlot->xAxis->pixelToCoord(event->x());
                y = ui->deliveryPlot->yAxis->pixelToCoord(event->y());
            }
       }
    // depending on the current step the point gets added as a delivery or pickup point
    switch(currentStep){
        case deliverySelection:{deliveryPlanner->AddDeliveryPoint(x, y);
                                ui->deliveryPlot->graph(0)->setData(deliveryPlanner->xDelivery, deliveryPlanner->yDelivery);
                                break;
        }
        case pickupSelection:{  deliveryPlanner->AddPickupPoint(x, y);
                                ui->deliveryPlot->graph(1)->setData(deliveryPlanner->xPickup, deliveryPlanner->yPickup);
                                break;
        }
    }
}

// User presses the "Back" button
void DeliveryViewer::StepBack(){
    switch(currentStep){
        // If we are at the first step we delete all points and the user can select from scratch
        case deliverySelection:{ currentStep = deliverySelection;
                                deliveryPlanner->Reset();
                                ui->deliveryPlot->graph(0)->setData(deliveryPlanner->xDelivery, deliveryPlanner->yDelivery);
                                ui->deliveryPlot->graph(1)->setData(deliveryPlanner->xPickup, deliveryPlanner->yPickup);
                                ui->deliveryPlot->replot();
                                break;
        }
        // If we computed a delivery plan the user can set up a new plan
        case deliveryPlan:{ deliveryPlanner->Reset();
                           currentStep = (StepSelection)(((int)currentStep) - 1);
                           ui->deliveryPlot->graph(0)->setData(deliveryPlanner->xDelivery, deliveryPlanner->yDelivery);
                           ui->deliveryPlot->graph(1)->setData(deliveryPlanner->xPickup, deliveryPlanner->yPickup);
                           ui->deliveryPlot->replot();
                           break;
        }
        // Take a step back
        default:{ currentStep = (StepSelection)(((int)currentStep) - 1);
                 break;
        }
    }
    // Update the step label
    UpdateStepLabel();
}

// User presses the "Continue" button
void DeliveryViewer::StepContinue(){
    switch(currentStep){
        // Don't change step (last step)
        case deliveryPlan: {currentStep = deliveryPlan;
                           break;
        }
        // Perform the calculation of the delivery route (nearest neighbor algorithm)
        case pickupSelection: {currentStep = (StepSelection)(((int)currentStep) + 1);
                              // Perform algorithm to find the delivery plan
                              double length = deliveryPlanner->CalculateDeliveryPlan(); // length of the route
                              // Plot the new route
                              NewDeliveryPlot(deliveryPlanner->xPlanned, deliveryPlanner->yPlanned, length);
                              break;
        }
        // Next step
        case deliverySelection: {currentStep = (StepSelection)(((int)currentStep) + 1);
                                break;
        }
    }
    // Update the step label
    UpdateStepLabel();
}

// Plots a ne delivery route (Length: length)
void  DeliveryViewer::NewDeliveryPlot(QVector<double> xPlanned, QVector<double> yPlanned, double length){
    plottedDeliveryPlans++;
    int currentPlot = 2 + plottedDeliveryPlans;
    // Setup the plot
    ui->deliveryPlot->addGraph();
    // Colour selection
    if(plottedDeliveryPlans <= 9)
        ui->deliveryPlot->graph(currentPlot)->setPen(QPen((Qt::GlobalColor)(9 + plottedDeliveryPlans)));
    else
        ui->deliveryPlot->graph(currentPlot)->setPen(QPen(Qt::black));
    // Style of line
    ui->deliveryPlot->graph(currentPlot)->setLineStyle(QCPGraph::lsNone);
    ui->deliveryPlot->graph(currentPlot)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 12));

    // Legend text
    QString legendText = QString(QLatin1String("Plan %1; Length: %2"))
                        .arg(plottedDeliveryPlans)
                        .arg(length);
    ui->deliveryPlot->graph(currentPlot)->setName(legendText);

    ui->deliveryPlot->graph(currentPlot)->setData(xPlanned, yPlanned, true);

    // Connect all points by lines
    for(int i = 0; i < xPlanned.count() - 1; i++){
        QCPItemLine *line = new QCPItemLine(ui->deliveryPlot);
        lines.push_back(line);
        line->start->setCoords(xPlanned.at(i), yPlanned.at(i));
        line->end->setCoords(xPlanned.at(i+1), yPlanned.at(i+1));
        ui->deliveryPlot->replot();
    }


}

// Updates the step label depending on the current step
void DeliveryViewer::UpdateStepLabel(){
    switch(currentStep){
        case deliverySelection: ui->lblStep->setText("Step 1: Double click to select your "
                                                "delivery points. Press \"Continue\" afterwards. You can start a new delivery plan by clicking \"Back\"."); break;
        case pickupSelection: ui->lblStep->setText("Step 2: Double click to select your "
                                                "pickup points. Press \"Continue\" afterwards to get the route."); break;
        case deliveryPlan: ui->lblStep->setText("Step 3: Delivery Plan computed! You can start a new delivery plan by returning to Step 1."); break;
    }
}

void DeliveryViewer::selectionChanged()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */

  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->deliveryPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->deliveryPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->deliveryPlot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->deliveryPlot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->deliveryPlot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->deliveryPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->deliveryPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->deliveryPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->deliveryPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->deliveryPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->deliveryPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->deliveryPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  // synchronize selection of graphs with selection of corresponding legend items:
  for (int i=0; i<ui->deliveryPlot->graphCount(); ++i)
  {
    QCPGraph *graph = ui->deliveryPlot->graph(i);
    QCPPlottableLegendItem *item = ui->deliveryPlot->legend->itemWithPlottable(graph);
    if (item->selected() || graph->selected())
    {
      item->setSelected(true);
      graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
    }
  }
}

void DeliveryViewer::mousePress()
{
  // if an axis is selected, only allow the direction of that axis to be dragged
  // if no axis is selected, both directions may be dragged

  if (ui->deliveryPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->deliveryPlot->axisRect()->setRangeDrag(ui->deliveryPlot->xAxis->orientation());
  else if (ui->deliveryPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->deliveryPlot->axisRect()->setRangeDrag(ui->deliveryPlot->yAxis->orientation());
  else
    ui->deliveryPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void DeliveryViewer::mouseWheel()
{
  // if an axis is selected, only allow the direction of that axis to be zoomed
  // if no axis is selected, both directions may be zoomed

  if (ui->deliveryPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->deliveryPlot->axisRect()->setRangeZoom(ui->deliveryPlot->xAxis->orientation());
  else if (ui->deliveryPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->deliveryPlot->axisRect()->setRangeZoom(ui->deliveryPlot->yAxis->orientation());
  else
    ui->deliveryPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void DeliveryViewer::removeAllGraphs()
{
  ui->deliveryPlot->clearGraphs();

  for (auto const& i : lines) {
      delete i;
  }
  lines.clear();
  AddStandardGraphs();
  plottedDeliveryPlans = 0;
  ui->deliveryPlot->replot();
  deliveryPlanner->Reset();

}

void DeliveryViewer::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);

  if (ui->deliveryPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
  {
    menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
    menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
    menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
    menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
    menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
  } else  // general context menu on graphs requested
  {
      if (ui->deliveryPlot->graphCount() > 0)
        menu->addAction("Remove all graphs and start from scratch", this, SLOT(removeAllGraphs()));
  }

  menu->popup(ui->deliveryPlot->mapToGlobal(pos));
}

void DeliveryViewer::moveLegend()
{
  if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
  {
    bool ok;
    int dataInt = contextAction->data().toInt(&ok);
    if (ok)
    {
      ui->deliveryPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
      ui->deliveryPlot->replot();
    }
  }
}

