package cp2024.solution;

import cp2024.circuit.CircuitNode;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Semaphore;

public class ParallelCircuitIfValue extends ParallelCircuitValue {
    private int indexOfChosenChild;
    private final int[] childrenValues;

    public ParallelCircuitIfValue(CircuitNode circuitNode, int subTaskParentIndex, ExecutorService workers,
                                  Semaphore parentNotProcessedSubTasksSemaphore,
                                  ArrayBlockingQueue<Integer> parentDoneTasksQueue) {
        super(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue);
        indexOfChosenChild = -1;
        childrenValues = new int[3];
        childrenValues[0] = -1;
        childrenValues[1] = -1;
        childrenValues[2] = -1;
        this.taskManager = workers.submit(new IfTaskManager(this));
    }

    private static class IfTaskManager extends TaskManager<ParallelCircuitIfValue> {
        private boolean done;

        public IfTaskManager(ParallelCircuitIfValue circuitValue) {
            super(circuitValue);
            done = false;
        }

        @Override
        public void run() {
            circuitValue.initComputations();
            while (!done) {
                try {
                    circuitValue.notProcessedTasksSempahore.acquire();
                    int indexOfDoneTask = circuitValue.doneTasks.take();
                    boolean computedValue = circuitValue.tasks.get(indexOfDoneTask).getValue();
                    handleDoneTask(computedValue, indexOfDoneTask);

                } catch (InterruptedException e) {
                    handleInterruption();
                }
            }
        }

        private void handleDoneTask(boolean computedValue, int indexOfDoneTask) {
            if (indexOfDoneTask == 0) {
                circuitValue.indexOfChosenChild = (computedValue) ? 1 : 2;
            } else {
                circuitValue.childrenValues[indexOfDoneTask] = (computedValue) ? 1 : 0;
            }

            if (circuitValue.indexOfChosenChild != -1) {
                if (circuitValue.childrenValues[circuitValue.indexOfChosenChild] != -1) {
                    circuitValue.circuitBooleanValue = circuitValue.childrenValues[circuitValue.indexOfChosenChild] == 1;
                    done = true;
                    circuitValue.endTask();
                }
            } else {
                if (circuitValue.childrenValues[1] == 0 && circuitValue.childrenValues[2] == 0) {
                    circuitValue.circuitBooleanValue = false;
                    done = true;
                    circuitValue.endTask();
                }

                if (circuitValue.childrenValues[1] == 1 && circuitValue.childrenValues[2] == 1) {
                    circuitValue.circuitBooleanValue = true;
                    done = true;
                    circuitValue.endTask();
                }
            }
        }
    }
}
