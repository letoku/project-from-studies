package pl.edu.mimuw.simulationobjects;

import pl.edu.mimuw.datastructures.specific.Direction;
import pl.edu.mimuw.datastructures.specific.LineStop;
import pl.edu.mimuw.datastructures.specific.NextStopArrival;
import pl.edu.mimuw.datastructures.general.EventsQueue;

public class Vehicle {
    protected final Line line;
    protected final int sideNumber;

    protected int currentStopN;
    protected Direction direction;
    protected final EventsQueue eventsQueue;

    public Vehicle(Line line, int sideNumber, EventsQueue eventsQueue) {
        this.line = line;
        this.sideNumber = sideNumber;
        this.eventsQueue = eventsQueue;
    }

    protected NextStopArrival getNextStop() {
        // we are assuming that lines have at least 2 stops(otherwise it is pointless to have line)
        if (direction == Direction.FORWARD) {
            return getNextForwardStop();
        }
        return getNextBackwardStop();
    }

    private NextStopArrival getNextForwardStop() {
        LineStop nextStop;
        int stopN;
        int minutesToArrive;
        boolean startingNewRoute = false;
        boolean endOfRoute = false;
        if (currentStopN == (line.getLineLength() - 1)) {
            nextStop = line.getLineStopAt(line.getLineLength() - 1);
            stopN = line.getLineLength() - 1;
            minutesToArrive = line.minutesOfStopAtTheEnd();
            startingNewRoute = true;
        } else {
            nextStop = line.getLineStopAt(currentStopN + 1);
            stopN = currentStopN + 1;
            minutesToArrive = line.getLineStopAt(currentStopN).travelTimeToNext();
            if (stopN == (line.getLineLength() - 1)) {
                endOfRoute = true;
            }
        }

        return new NextStopArrival(nextStop.stop(), stopN, minutesToArrive, startingNewRoute, endOfRoute);
    }

    private NextStopArrival getNextBackwardStop() {
        LineStop nextStop;
        int stopN;
        int minutesToArrive;
        boolean startingNewRoute = false;
        boolean endOfRoute = false;
        if (currentStopN == 0) {
            nextStop = line.getLineStopAt(0);
            stopN = 0;
            minutesToArrive = line.minutesOfStopAtTheEnd();
            startingNewRoute = true;
        } else {
            nextStop = line.getLineStopAt(currentStopN - 1);
            stopN = currentStopN - 1;
            if (stopN == 0) {
                endOfRoute = true;
            }
            minutesToArrive = nextStop.travelTimeToNext();
        }

        return new NextStopArrival(nextStop.stop(), stopN, minutesToArrive, startingNewRoute, endOfRoute);
    }

    protected void changeDirection() {
        if (direction == Direction.FORWARD) {
            direction = Direction.BACKWARD;
        } else {
            direction = Direction.FORWARD;
        }
    }

    public LineStop[] stopsOnVehicleLine() {
        return line.getLineStops();
    }

    public int getCurrentStopN() {
        return currentStopN;
    }

    public Direction getDirection() {
        return direction;
    }
}
