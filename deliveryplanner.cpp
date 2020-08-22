#include "deliveryplanner.h"

DeliveryPlanner::DeliveryPlanner():
    deliveryCount(0),
    pickupCount(0)
{
    xDepot.append(0); yDepot.append(0); // Add the depot point x/y=0/0
}

// Nearest neighbor algorithm: Calculates the planned route
double DeliveryPlanner::CalculateDeliveryPlan(){
    // Initialize the eventList with all points
    FillEventList();

    int eventsPlanned = 0; // Planned events so far
    int closestEvent = 0; // Next closest event index
    int remainingEvents = 1 + deliveryCount + pickupCount; // number of remaining events to be added

    double distClosestEvent = INT_MAX; // Initialize distance to nearest neighbor with large number
    double distPlannedSoFar = 0; // Distance of route so far
    double distCurrent = 0; // holds current distance during runtime

    // Clear the eventList from previous runtimes
    deliveryEventList.clear();
    // First point of the route is the depot at index 0
    deliveryEventList.push_back(eventList[0]);
    // Erase that point so that it doesn't get added twice
    eventList.erase(eventList.begin());
    remainingEvents--;
    eventsPlanned++;
    // While we still have events to add
    while(remainingEvents > 0)
    {
        distClosestEvent = INT_MAX;
        // Find the nearest neighbor of the last added event
        for (int i = 0; i < remainingEvents; ++i)
        {
            distCurrent = eventList[i]->GetDistance(*deliveryEventList[eventsPlanned-1]);
            if (distCurrent < distClosestEvent)
            {
                closestEvent = i; // nearest neighbor index
                distClosestEvent = distCurrent; // nearest neighbor distance
            }
        }
        // Add the nearest neighbor distance
        distPlannedSoFar += distClosestEvent;
        // Add nearest neighbor to our route
        deliveryEventList.push_back(eventList[closestEvent]);

        // If we found a pickup point we can clear all of the other pickup points
        // (only visit one (1) pickup pont)
        if(eventList[closestEvent]->GetIndex() > deliveryCount){
            eventList.erase(eventList.end() - (pickupCount -1), eventList.end());
            remainingEvents = remainingEvents - pickupCount;
        }else{
            // Otherwise just erase the added event (nearest neighbor)
            eventList.erase(eventList.begin() + closestEvent);
            remainingEvents--;
        }
        // One more event added
        eventsPlanned++;
    }
    // The last point of our route is the depot
    deliveryEventList.push_back(deliveryEventList.at(0));

    // prepare the planned route that we found so that we can plot it
    FillPlannedRoute();

    // Return the distance of our route (The distance between the last added point and the depot has to be considered)
    return distPlannedSoFar + deliveryEventList[0]->GetDistance(*deliveryEventList[eventsPlanned-1]);

}

// Adds a delivery point
void DeliveryPlanner::AddDeliveryPoint(double x, double y){
    xDelivery.append(x);
    yDelivery.append(y);
    deliveryCount++;
}

// Adds a pickup point
void DeliveryPlanner::AddPickupPoint(double x, double y){
    xPickup.append(x);
    yPickup.append(y);
    pickupCount++;
}

// Resets the planner to the initial state
void DeliveryPlanner::Reset(){
    xDelivery.clear(); yDelivery.clear();
    xPickup.clear(); yPickup.clear();
    xPlanned.clear(); yPlanned.clear();

    deliveryCount = 0; pickupCount = 0;

    for (auto const& i : eventList) {
        delete i;
    }
    eventList.clear();
    deliveryEventList.clear();

}

// Fills the eventList with the delivery points, pickup points and the depot point
void DeliveryPlanner::FillEventList(){
    eventList.push_back(new Event(xDepot.at(0), yDepot.at(0), 0));

    int deliveryCount = xDelivery.count();
    for(int i = 0; i < deliveryCount; i++){
        eventList.push_back(new Event(xDelivery.at(i), yDelivery.at(i), i + 1));
    }

    int pickupCount = xPickup.count();
    for(int i = 0; i < pickupCount; i++){
        eventList.push_back(new Event(xPickup.at(i), yPickup.at(i), i + deliveryCount + 1));
    }
}

// Prepare planned route vectors for plotting
void DeliveryPlanner::FillPlannedRoute(){
    for (auto const& i : deliveryEventList) {
        xPlanned.push_back(i->GetX());
        yPlanned.push_back(i->GetY());
    }
}

// Executed on finish
DeliveryPlanner::~DeliveryPlanner(){
    Reset();
}
