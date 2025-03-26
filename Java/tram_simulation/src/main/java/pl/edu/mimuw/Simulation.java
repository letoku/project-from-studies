package pl.edu.mimuw;

import pl.edu.mimuw.events.Event;
import pl.edu.mimuw.datastructures.general.EventsQueue;
import pl.edu.mimuw.datastructures.general.HeapEventsQueue;
import pl.edu.mimuw.simulationobjects.Line;
import pl.edu.mimuw.simulationobjects.Passenger;
import pl.edu.mimuw.simulationobjects.Stop;
import pl.edu.mimuw.utils.Logger;

import java.util.Scanner;

public class Simulation {
    private final int daysToSimulate;
    private final int tramCapacity;
    private int currentDay;
    private final Passenger[] passengers;
    private final Stop[] stops;
    private final Line[] lines;
    private final EventsQueue eventsQueue;
    private final Logger logger;
    private int totalTravels;

    public Simulation(int daysToSimulate, int passengersNumber, Stop[] stops, Line[] lines, EventsQueue eventsQueue, Logger logger, int tramCapacity) {
        this.daysToSimulate = daysToSimulate;
        this.currentDay = 0;
        this.tramCapacity = tramCapacity;
        this.passengers = Passenger.initPassengers(passengersNumber, stops, eventsQueue, logger);
        this.stops = stops;
        this.lines = lines;
        this.eventsQueue = eventsQueue;
        this.logger = logger;
        this.totalTravels = 0;
        addReadParametersLog();
    }

    public static Simulation readSimulationFromInput(Scanner sc) {
        HeapEventsQueue eventsQueue = new HeapEventsQueue();
        Logger simulationLogger = new Logger();
        int simulationDays = sc.nextInt();
        int stopCapacity = sc.nextInt();
        int stopsNumber = sc.nextInt();
        Stop[] stops = Stop.readStopsFromInput(sc, stopsNumber, stopCapacity);

        int passengersNumber = sc.nextInt();
        int tramCapacity = sc.nextInt();
        int linesNumber = sc.nextInt();
        Line[] lines = Line.readLinesFromInput(sc, linesNumber, tramCapacity, stops, eventsQueue, simulationLogger);

        return new Simulation(simulationDays, passengersNumber, stops, lines, eventsQueue, simulationLogger, tramCapacity);
    }

    public void simulate() {
        for (int i = 0; i < daysToSimulate; i++) {
            simulateSingleDay();
            summarizeDay();
            currentDay += 1;
        }
    }

    ;

    private void simulateSingleDay() {
        initDay();
        while (!eventsQueue.isEmpty()) {
            Event event = eventsQueue.pop();
            event.executeEvent();
        }
    }

    private void initDay() {
        logger.addLog("\n------------ DZIEŃ " + currentDay + " ------------");

        for (Passenger passenger : passengers) {
            passenger.addGoToStopEvent(currentDay);
        }

        for (Line line : lines) {
            line.startTrams(currentDay);
        }
    }

    private void addReadParametersLog() {
        logger.addLog("\n------------  WCZYTANE PARAMETRY ------------");
        logger.addLog("Liczba dni symulacji: " + daysToSimulate);
        logger.addLog("Pojemność przystanku: " + stops[0].getPassengersCapacity());
        logger.addLog("Liczba przystanków: " + stops.length);
        for (int i = 0; i < stops.length; i++) {
            logger.addLog("Przystanek " + i + ": " + stops[i].getName());
        }
        logger.addLog("Liczba pasażerów: " + passengers.length);
        logger.addLog("Pojemność tramwaju: " + tramCapacity);
        logger.addLog("Liczba linii tramwajowych: " + lines.length);

        for (Line line : lines) {
            logger.addLog("\nLINIA NUMER: " + line.getLineNumber());
            logger.addLog("Liczba przystanków: " + line.getLineLength());
            logger.addLog(line.lineStopsDescriptionLog());
        }
    }

    public void summarizeSimulation() {
        logger.addLog("\n------------ PODSUMOWANIE CAŁEJ SYMULACJI ------------");
        logger.addLog("Liczba wszystkich podróży: " + totalTravels);
        for (Stop stop : stops) {
            logger.addLog(stop.averageWaitTimeLog());
        }

        logger.printSimulationLogs();
    }

    private void summarizeDay() {
        int currentDayNumberOfTravels = currentDayNumberOfTravels();
        totalTravels += currentDayNumberOfTravels;

        logger.addLog("\n------------ PODSUMOWANIE DNIA " + currentDay + " ------------");
        logger.addLog("------------ Wszystkie podróże tego dnia:  " + currentDayNumberOfTravels + " ------------");
        logger.addLog("------------ Liczba minut przeczekanych na przystankach tego dnia:  " + currentDayNumberOfMinutesWaitedOnStops() + " ------------");
        resetStateOfSimulation();
    }

    private void resetStateOfSimulation() {
        eventsQueue.clear();
        for (Passenger passenger : passengers) {
            passenger.setNumberOfTravels(0);
        }

        for (Stop stop : stops) {
            stop.resetStateAfterDay();
        }
    }

    private int currentDayNumberOfTravels() {
        int totalTravels = 0;
        for (Passenger passenger : passengers) {
            totalTravels += passenger.getNumberOfTravels();
        }
        return totalTravels;
    }

    private int currentDayNumberOfMinutesWaitedOnStops() {
        int totalMinutesWaited = 0;
        for (Stop stop : stops) {
            totalMinutesWaited += stop.totalMinutesWaitedOnCurrentDay();
        }

        return totalMinutesWaited;
    }
}
