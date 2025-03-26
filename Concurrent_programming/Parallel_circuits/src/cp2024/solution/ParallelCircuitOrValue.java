package cp2024.solution;

import cp2024.circuit.CircuitNode;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Semaphore;

public class ParallelCircuitOrValue extends ParallelCircuitValue {

    public ParallelCircuitOrValue(CircuitNode circuitNode, int subTaskParentIndex, ExecutorService workers, Semaphore parentNotProcessedSubTasksSemaphore, ArrayBlockingQueue<Integer> parentDoneTasksQueue) {
        super(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue);
        this.taskManager = workers.submit(new OrTaskManager(this));
    }

    private static class OrTaskManager extends TaskManager<ParallelCircuitOrValue> {
        private int tasksDone;
        private boolean done;

        public OrTaskManager(ParallelCircuitOrValue circuitOrValue) {
            super(circuitOrValue);
            tasksDone = 0;
            done = false;
        }

        @Override
        public void run() {
            circuitValue.initComputations();

            while (tasksDone < circuitValue.numOfChildren && !done) {
                if (Thread.currentThread().isInterrupted()) {
                    handleInterruption();
                    return;
                }
                try {
                    circuitValue.notProcessedTasksSempahore.acquire();
                    int indexOfDoneTask = circuitValue.doneTasks.take();
                    boolean computedValue = circuitValue.tasks.get(indexOfDoneTask).getValue();
                    handleDoneTask(computedValue);

                } catch (InterruptedException e) {
                    handleInterruption();
                    return;
                }
            }

            if (!done) {
                circuitValue.circuitBooleanValue = false;
                circuitValue.endTask();
            }
        }

        private void handleDoneTask(boolean computedValue) {
            if (!computedValue) {
                tasksDone++;
            } else {
                circuitValue.circuitBooleanValue = true;
                done = true;
                circuitValue.endTask();
            }
        }
    }
}
