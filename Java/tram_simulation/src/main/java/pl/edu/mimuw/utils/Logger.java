package pl.edu.mimuw.utils;

import pl.edu.mimuw.datastructures.general.Vector;

public class Logger {
    private final Vector<String> simulationLogs;

    public Logger() {
        this.simulationLogs = new Vector<String>();
    }

    public void addLog(String log) {
        simulationLogs.add(log);
    }

    ;

    public void print(String s) {
        System.out.println(s);
    }

    public void printSimulationLogs() {
        for (int i = 0; i < simulationLogs.size(); i++) {
            print(simulationLogs.getAtIndex(i));
        }
    }

}
