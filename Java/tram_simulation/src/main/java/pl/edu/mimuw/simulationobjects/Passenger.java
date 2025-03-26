package pl.edu.mimuw.simulationobjects;

import pl.edu.mimuw.utils.Logger;
import pl.edu.mimuw.utils.Losowanie;
import pl.edu.mimuw.datastructures.specific.Direction;
import pl.edu.mimuw.datastructures.specific.LineStop;
import pl.edu.mimuw.datastructures.general.EventsQueue;
import pl.edu.mimuw.events.GoToStopEvent;
import pl.edu.mimuw.datastructures.specific.Time;

public class Passenger {
    private final static int earliestHour = 6;
    private final static int latestHour = 12;
    private final int id;
    private int numberOfTravels;
    private final Stop homeStop;
    private Stop destinationStop;

    private Time timeOfGettingToCurrentStop;
    private final EventsQueue eventsQueue;
    private final Logger simulationLogger;

    public Passenger(int id, Stop homeStop, EventsQueue eventsQueue, Logger simulationLogger) {
        this.id = id;
        this.homeStop = homeStop;
        this.eventsQueue = eventsQueue;
        this.simulationLogger = simulationLogger;
        numberOfTravels = 0;
    }

    public static Passenger[] initPassengers(int passengersNumber, Stop[] stops, EventsQueue eventsQueue, Logger simulationLogger) {
        Passenger[] passengers = new Passenger[passengersNumber];
        for (int i = 0; i < passengersNumber; i++) {
            // stop was not given, so we are assuming it should be randomly chosen
            int chosenStopIndex = Losowanie.losuj(0, stops.length - 1);
            passengers[i] = new Passenger(i, stops[chosenStopIndex], eventsQueue, simulationLogger);
        }
        return passengers;
    }

    public void addGoToStopEvent(int simulationDay) {
        Time timeOfEvent = new Time(simulationDay, Time.randomMinute(earliestHour, latestHour));
        GoToStopEvent event = new GoToStopEvent(timeOfEvent, this);
        eventsQueue.add(event);
    }

    public boolean isDestinationStop(Stop stop) {
        return stop.getName().equals(destinationStop.getName());
    }

    public void goToStop(Time time) {
        homeStop.takePassenger(this, time, false);
    }

    public void getOnStop(Stop stop, Time time, boolean fromTram) {
        timeOfGettingToCurrentStop = time;
        logGettingOnStop(stop, time, fromTram);
    }

    /**
     * This method is called only when it is assured that passenger can get on the tram
     *
     * @param tram
     * @return time that this passenger was waiting for the tram
     */
    public int getOnTram(Tram tram, Time time) {
        chooseDestinationStop(tram.stopsOnVehicleLine(), tram.getCurrentStopN(), tram.getDirection());
        logGettingOnTram(tram, time);
        tram.takePassengerFromStop(this);
        numberOfTravels++;

        return time.minutesFrom(timeOfGettingToCurrentStop);
    }

    private void logGettingOnTram(Tram tram, Time time) {
        simulationLogger.addLog(
                time.logTimeInfoString() + "Pasażer " + id + " wsiadł do tramwaju linii "
                        + tram.line.getLineNumber() + " (nr boczny " + tram.sideNumber + ") z zamiarem dojechania do przystanku "
                        + destinationStop.getName());
    }

    private void logGettingOnStop(Stop stop, Time time, boolean fromTram) {
        if (fromTram) {
            simulationLogger.addLog(
                    time.logTimeInfoString() + "Pasażer " + id + " wysiadł z tramwaju na przystanek " + stop.getName()
            );
        } else {
            simulationLogger.addLog(
                    time.logTimeInfoString() + "Pasażer " + id + " przyszedł na przystanek " + stop.getName()
            );
        }
    }

    private void chooseDestinationStop(LineStop[] stopsToChooseFrom, int currentTramLocation, Direction tramDirection) {
        int[] bounds = rangeOfPossibleStops(stopsToChooseFrom, currentTramLocation, tramDirection);
        int chosenStopIndex = Losowanie.losuj(bounds[0], bounds[1]);
        destinationStop = stopsToChooseFrom[chosenStopIndex].stop();
    }

    private int[] rangeOfPossibleStops(LineStop[] stopsToChooseFrom, int currentTramLocation, Direction tramDirection) {
        if (tramDirection == Direction.FORWARD) {
            return rangeOfPossibleStopsForForwardDirection(stopsToChooseFrom, currentTramLocation);
        }
        return rangeOfPossibleStopsForBackwardDirection(stopsToChooseFrom, currentTramLocation);
    }

    private int[] rangeOfPossibleStopsForForwardDirection(LineStop[] stopsToChooseFrom, int currentTramLocation) {
        int lowerRange;
        int upperRange;
        int lastStopNumber = stopsToChooseFrom.length - 1;
        if (currentTramLocation == lastStopNumber) {  // means this is last stop on the line
            lowerRange = 0;
            upperRange = lastStopNumber - 1;  // we assume that passenger wants to travel to some other stop
        } else {
            lowerRange = currentTramLocation + 1;
            upperRange = lastStopNumber;
        }
        return new int[]{lowerRange, upperRange};
    }

    private int[] rangeOfPossibleStopsForBackwardDirection(LineStop[] stopsToChooseFrom, int currentTramLocation) {
        int lowerRange;
        int upperRange;
        int lastStopNumber = stopsToChooseFrom.length - 1;
        if (currentTramLocation == 0) {  // means this is last stop on the line
            lowerRange = 1;  // we assume that passenger wants to travel to some other stop
            upperRange = lastStopNumber;
        } else {
            lowerRange = 0;
            upperRange = currentTramLocation - 1;
        }
        return new int[]{lowerRange, upperRange};
    }

    public void setTimeOfGettingToCurrentStop(Time timeOfGettingToCurrentStop) {
        this.timeOfGettingToCurrentStop = timeOfGettingToCurrentStop;
    }

    public int getNumberOfTravels() {
        return numberOfTravels;
    }

    public void setNumberOfTravels(int numberOfTravels) {
        this.numberOfTravels = numberOfTravels;
    }
}
