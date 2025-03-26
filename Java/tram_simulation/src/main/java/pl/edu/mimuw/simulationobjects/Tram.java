package pl.edu.mimuw.simulationobjects;

import pl.edu.mimuw.utils.Logger;
import pl.edu.mimuw.datastructures.specific.Direction;
import pl.edu.mimuw.datastructures.specific.NextStopArrival;
import pl.edu.mimuw.events.ArrivalAtStopEvent;
import pl.edu.mimuw.datastructures.general.EventsQueue;
import pl.edu.mimuw.datastructures.specific.Time;

public class Tram extends Vehicle {
    private static int numberOfAllTrams = 0;
    private final static int latestHourOfDeparture = 23;
    private final int passengersCapacity;
    private final Passenger[] passengers;
    private int currentPassengersNumber;
    private final Logger simulationLogger;

    public Tram(Line line, int passengersCapacity, EventsQueue eventsQueue, Logger simulationLogger) {
        super(line, numberOfAllTrams, eventsQueue);
        numberOfAllTrams++;
        this.passengersCapacity = passengersCapacity;
        this.passengers = new Passenger[passengersCapacity];
        this.currentPassengersNumber = 0;
        this.simulationLogger = simulationLogger;
    }

    /**
     * Starting tram at the beginning of the day.
     * However, it is implemented as departure from the boundary stop with opposite direction.
     * Because of this, initial direction is reversed and there is need to move back time by time of waiting at the end stop
     *
     * @param direction
     * @param time
     */
    public void start(Direction direction, Time time) {
        // as described above, there is need to reverse direction, to imitate starting from the first stop in right direction
        if (direction == Direction.FORWARD) {
            currentStopN = 0;
            this.direction = Direction.BACKWARD;
        } else {
            currentStopN = line.getLineLength() - 1;
            this.direction = Direction.FORWARD;
        }

        // this ensures that tram arrives at given 'time' on the ending stop
        Time correctedTime = new Time(time.getDay(), time.getMinute() - line.minutesOfStopAtTheEnd());
        travelToNextStop(correctedTime);
    }

    /**
     * Travels tram to the next stop. Doing so it updates localization of tram and simulation's events queue.
     *
     * @param currentTime - time of departure from current stop of tram
     */
    private void travelToNextStop(Time currentTime) {
        NextStopArrival nextStopArrival = getNextStop();
        updateTravelInfo(nextStopArrival);

        if (nextStopArrival.startingNewRoute() && !shouldDepartAgain(currentTime, nextStopArrival)) return; // after
        // latestHourOfDeparture no trams are starting again
        addArrivalAtStopEvent(nextStopArrival, currentTime);
    }

    /**
     * Correctly sets up next stop of tram, if needed also changes direction of travel.
     *
     * @param nextStopArrival
     */
    private void updateTravelInfo(NextStopArrival nextStopArrival) {
        if (nextStopArrival.startingNewRoute()) {
            changeDirection();
        }
        currentStopN = nextStopArrival.stopN();
    }

    private void addArrivalAtStopEvent(NextStopArrival nextStopArrival, Time currentTime) {
        Time timeOfEvent = new Time(
                currentTime.getDay(),
                currentTime.getMinute() + nextStopArrival.minutesToArrive()
        );
        ArrivalAtStopEvent event = new ArrivalAtStopEvent(timeOfEvent, nextStopArrival.stop(),
                nextStopArrival.endOfRoute(), this);
        eventsQueue.add(event);
    }

    private boolean shouldDepartAgain(Time currentTime, NextStopArrival nextStopArrival) {
        int timeOfDeparture = currentTime.getMinute() + nextStopArrival.minutesToArrive();
        return timeOfDeparture <= latestHourOfDeparture * Time.MINUTES_IN_HOUR;
    }

    public void arriveAtStop(Stop stop, boolean endOfRoute, Time time) {
        simulationLogger.addLog(logOfArrivingToStop(stop, time));
        releasePassengersToStop(stop, time);
        if (!endOfRoute) takePassengersFromStop(stop, time);
        travelToNextStop(time);
    }

    private void releasePassengersToStop(Stop stop, Time time) {
        int passengersToReleaseLimit = stop.freeSeatsNumber();
        int releasedPassengers = 0;
        for (int i = 0; i < currentPassengersNumber && releasedPassengers < passengersToReleaseLimit; i++) {
            if (passengers[i].isDestinationStop(stop)) {
                releasePassenger(stop, i, time);
                releasedPassengers++;
                i--; // this is because array was moved one place left(because of hole after passenger who just left)
            }
        }
    }

    /**
     * @param stop stop to which passengers goes to
     * @param i    index of passenger to release
     */
    private void releasePassenger(Stop stop, int i, Time time) {
        stop.takePassenger(passengers[i], time, true);
        removeEmptyPlaceInPassengersArray(i);
        currentPassengersNumber--;
    }

    /**
     * When passengers left the tram, empty place occurs in the passengers array. This function moves elements
     * by one place to come back to contiguous arrangement
     *
     * @param indexOfEmptyPlace
     */
    private void removeEmptyPlaceInPassengersArray(int indexOfEmptyPlace) {
        for (int i = indexOfEmptyPlace + 1; i < currentPassengersNumber; i++) {
            passengers[i - 1] = passengers[i];
        }
    }

    private void takePassengersFromStop(Stop stop, Time time) {
        stop.releasePassengersToTram(this, numberOfFreeSeats(), time);
    }

    public void takePassengerFromStop(Passenger passenger) {
        if (currentPassengersNumber >= passengersCapacity) {
            return;
        }
        passengers[currentPassengersNumber] = passenger;
        currentPassengersNumber++;
    }

    private String logOfArrivingToStop(Stop stop, Time time) {
        return time.logTimeInfoString() + "Tramwaj linii " + line.getLineNumber() +
                " (nr boczny " + sideNumber + ") przyjechał na przystanek " + stop.getName();
    }

    private int numberOfFreeSeats() {
        return passengersCapacity - currentPassengersNumber;
    }
}
