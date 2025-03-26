package pl.edu.mimuw.simulationobjects;

import pl.edu.mimuw.datastructures.general.Vector;
import pl.edu.mimuw.datastructures.specific.Time;

import java.util.Scanner;

import static java.lang.Math.min;

public class Stop {
    private final String name;
    private final int passengersCapacity;
    private final Passenger[] passengers;
    private int currentPassengersNumber;
    private final Vector<Integer> singleDayWaitingTimesHistory;
    private final Vector<Integer> waitingTimesHistory;


    public Stop(String name, int passengersCapacity) {
        this.name = name;
        this.passengersCapacity = passengersCapacity;
        this.passengers = new Passenger[passengersCapacity];
        this.currentPassengersNumber = 0;
        this.singleDayWaitingTimesHistory = new Vector<>();
        this.waitingTimesHistory = new Vector<>();
    }

    public static Stop[] readStopsFromInput(Scanner sc, int stopsNumber, int stopCapacity) {
        Stop[] stops = new Stop[stopsNumber];
        for (int i = 0; i < stopsNumber; i++) {
            String stopName = sc.next();
            stops[i] = new Stop(stopName, stopCapacity);
        }
        return stops;
    }

    public void takePassenger(Passenger passenger, Time time, boolean fromTram) {
        if (currentPassengersNumber >= passengersCapacity) {
            return;
        }
        passenger.getOnStop(this, time, fromTram);
        passengers[currentPassengersNumber] = passenger;
        currentPassengersNumber++;
    }

    public int freeSeatsNumber() {
        return passengersCapacity - currentPassengersNumber;
    }

    public String getName() {
        return name;
    }

    public void releasePassengersToTram(Tram tram, int passengersLimit, Time time) {
        int releasingPassengersNumber = min(passengersLimit, currentPassengersNumber);
        for (int i = 0; i < releasingPassengersNumber; i++) {
            int minutesOfWaiting = passengers[i].getOnTram(tram, time);
            singleDayWaitingTimesHistory.add(minutesOfWaiting);
        }
        reallocateSeats(releasingPassengersNumber);
        currentPassengersNumber -= releasingPassengersNumber;
    }

    /**
     * In our simulation, first n passengers get on the tram. After this there is an empty space of n elements at the
     * beginning of the passengers array. This function moves all remaining passengers to the beginning to ensure nice
     * contiguous arrangement of passengers in the array
     *
     * @param releasedPassengersNumber
     */
    private void reallocateSeats(int releasedPassengersNumber) {
        int indexOfFirstFreeSeat = 0;
        for (int i = releasedPassengersNumber; i < currentPassengersNumber; i++) {
            passengers[indexOfFirstFreeSeat] = passengers[i];
            indexOfFirstFreeSeat++;
        }
    }

    public int totalMinutesWaitedOnCurrentDay() {
        int totalMinutesWaitedOnCurrentDay = 0;
        for (int i = 0; i < singleDayWaitingTimesHistory.size(); i++) {
            totalMinutesWaitedOnCurrentDay += singleDayWaitingTimesHistory.getAtIndex(i);
        }
        return totalMinutesWaitedOnCurrentDay;
    }

    public void resetStateAfterDay() {
        clear();
        archiveSingleDayWaitingHistory();
    }

    private void clear() {
        currentPassengersNumber = 0;
    }

    private void archiveSingleDayWaitingHistory() {
        waitingTimesHistory.concatenate(singleDayWaitingTimesHistory);
        singleDayWaitingTimesHistory.clear();
    }

    public String averageWaitTimeLog() {
        return name + " - średni czas czekania: " + String.format("%.1f", averageWaitingTime());
    }

    public double averageWaitingTime() {
        int sum = 0;
        for (int i = 0; i < waitingTimesHistory.size(); i++) {
            sum += waitingTimesHistory.getAtIndex(i);
        }
        double totalWaits = waitingTimesHistory.size();
        return sum / totalWaits;
    }

    public int getPassengersCapacity() {
        return passengersCapacity;
    }
}
