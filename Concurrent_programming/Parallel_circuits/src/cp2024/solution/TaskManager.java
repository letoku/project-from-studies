package cp2024.solution;


public abstract class TaskManager<T extends ParallelCircuitValue> implements Runnable {
    protected final T circuitValue;

    public TaskManager(T circuitValue) {
        this.circuitValue = circuitValue;
    }

    @Override
    public abstract void run();

    protected void handleInterruption() {
        circuitValue.interrupted = true;
        circuitValue.endTask();
    }
}