package pl.edu.mimuw;

import java.util.Scanner;

public class Main {

    public static void main(String[] args) {

        Scanner sc = new Scanner(System.in);

        Simulation simulation = Simulation.readSimulationFromInput(sc);
        simulation.simulate();
        simulation.summarizeSimulation();
    }
}
