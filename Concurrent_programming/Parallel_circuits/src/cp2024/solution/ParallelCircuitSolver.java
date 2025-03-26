package cp2024.solution;

import cp2024.circuit.Circuit;
import cp2024.circuit.CircuitSolver;
import cp2024.circuit.CircuitValue;

import java.util.ArrayList;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Semaphore;

public class ParallelCircuitSolver implements CircuitSolver {
    private static final int N_COUNTING_WORKERS = Integer.MAX_VALUE;
    private final ExecutorService workers;
    private final ArrayList<CircuitValue> tasks = new ArrayList<>();
    private boolean stopped;

    public ParallelCircuitSolver() {
        this.workers = Executors.newFixedThreadPool(N_COUNTING_WORKERS);
        this.stopped = false;
    }

    @Override
    public CircuitValue solve(Circuit c) {
        if (stopped) {
            return new StoppedCircuitValue();
        }
        ParallelCircuitValue newCircuitComputationTask = ParallelCircuitValue.factory(c.getRoot(), 0,
                workers, new Semaphore(0), new ArrayBlockingQueue<>(1));
        tasks.add(newCircuitComputationTask);
        return newCircuitComputationTask;
    }

    @Override
    public void stop() {
        stopped = true;
        Thread stopManager = new Thread(new stopManager(this));
        stopManager.start();
    }

    private static class stopManager implements Runnable {
        private final ParallelCircuitSolver solver;

        public stopManager(ParallelCircuitSolver solver) {
            this.solver = solver;
        }

        @Override
        public void run() {
            for (CircuitValue circuitValue : solver.tasks) {
                ParallelCircuitValue parallelCircuitValue = (ParallelCircuitValue) circuitValue;
                parallelCircuitValue.taskManager.cancel(true);
            }

            solver.workers.shutdownNow();
        }

    }
}
