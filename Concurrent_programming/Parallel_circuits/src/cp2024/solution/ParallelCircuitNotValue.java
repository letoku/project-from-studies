package cp2024.solution;

import cp2024.circuit.CircuitNode;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Semaphore;

public class ParallelCircuitNotValue extends ParallelCircuitValue {
    public ParallelCircuitNotValue(CircuitNode circuitNode, int subTaskParentIndex, ExecutorService workers,
                                   Semaphore parentNotProcessedSubTasksSemaphore,
                                   ArrayBlockingQueue<Integer> parentDoneTasksQueue) {
        super(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue);
        this.taskManager = workers.submit(new NotTaskManager(this));
    }

    private static class NotTaskManager extends TaskManager<ParallelCircuitNotValue> {

        public NotTaskManager(ParallelCircuitNotValue circuitValue) {
            super(circuitValue);
        }

        @Override
        public void run() {
            circuitValue.initComputations();
            try {
                circuitValue.notProcessedTasksSempahore.acquire();
                int indexOfDoneTask = circuitValue.doneTasks.take();
                boolean computedValue = circuitValue.tasks.get(indexOfDoneTask).getValue();
                handleDoneTask(computedValue);

            } catch (InterruptedException e) {
                handleInterruption();
            }
        }

        private void handleDoneTask(boolean computedValue) {
            circuitValue.circuitBooleanValue = !computedValue;
            circuitValue.endTask();
        }
    }
}