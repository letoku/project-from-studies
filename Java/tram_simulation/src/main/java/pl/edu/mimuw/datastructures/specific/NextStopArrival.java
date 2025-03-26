package pl.edu.mimuw.datastructures.specific;

import pl.edu.mimuw.simulationobjects.Stop;

public record NextStopArrival(Stop stop, int stopN, int minutesToArrive, boolean startingNewRoute, boolean endOfRoute) {
    public NextStopArrival(Stop stop, int stopN, int minutesToArrive, boolean startingNewRoute, boolean endOfRoute) {
        this.stop = stop;
        this.stopN = stopN;
        this.minutesToArrive = minutesToArrive;
        this.startingNewRoute = startingNewRoute;  // is now beginning new route(starting riding from the boundary stop)
        this.endOfRoute = endOfRoute;  // is now arriving to the boundary stop on the route
    }
}
