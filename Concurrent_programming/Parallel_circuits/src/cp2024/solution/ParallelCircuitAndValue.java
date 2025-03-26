package cp2024.solution;

import cp2024.circuit.CircuitNode;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Semaphore;

public class ParallelCircuitAndValue extends ParallelCircuitValue {

    public ParallelCircuitAndValue(CircuitNode circuitNode, int subTaskParentIndex, ExecutorService workers, Semaphore parentNotProcessedSubTasksSemaphore, ArrayBlockingQueue<Integer> parentDoneTasksQueue) {
        super(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue);
        this.taskManager = workers.submit(new AndTaskManager(this));
    }

    private static class AndTaskManager extends TaskManager<ParallelCircuitAndValue> {
        private int tasksDone;
        private boolean done;

        public AndTaskManager(ParallelCircuitAndValue circuitAndValue) {
            super(circuitAndValue);
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
                circuitValue.circuitBooleanValue = true;
                circuitValue.endTask();
            }
        }

        private void handleDoneTask(boolean computedValue) {
            if (computedValue) {
                tasksDone++;
            } else {
                circuitValue.circuitBooleanValue = false;
                done = true;
                circuitValue.endTask();
            }
        }
    }
}
