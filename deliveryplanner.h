#ifndef DELIVERYPLANNER_H
#define DELIVERYPLANNER_H

#include <QVector>
#include "event.h"

class DeliveryPlanner
{
public:
    DeliveryPlanner();
    ~DeliveryPlanner();
    QVector<double> xDelivery, yDelivery; // hold the current delivery points of the planner
    QVector<double> xPickup, yPickup; // hold the current pickup points of the planner
    QVector<double> xDepot, yDepot; // holds the coordinate of the depot
    QVector<double> xPlanned, yPlanned; // holds the calculated planned delivery route
    void AddDeliveryPoint(double x, double y); // adds another delivery point
    void AddPickupPoint(double x, double y); // adds another pickup point
    void Reset(); // resets the planner to the initial state
    double CalculateDeliveryPlan(); // calculates the delivery plan (nearest neighbor algorithm)
private:
    QVector<Event*> eventList; // holds the remaining event points that are not part of the route yet
    QVector<Event*> deliveryEventList; // holds the events that are part of the planned route
    uint deliveryCount; // number of delivery points
    uint pickupCount; // number of pickup points
    void FillEventList(); // prepares the eventList for the algorithm
    void FillPlannedRoute(); // Gets filled with the planned route after the algorithm finishes
};

#endif // DELIVERYPLANNER_H
