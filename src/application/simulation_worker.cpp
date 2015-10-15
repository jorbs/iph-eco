#include "include/application/simulation_worker.h"
#include "include/application/simulation_progress_listener.h"

extern "C" {
    void startSimulation(SimulationDataType::Simulation *simulation);
}

SimulationWorker::SimulationWorker(Simulation *simulation) : simulation(simulation) {
    this->moveToThread(&thread);
    thread.start();
}

SimulationWorker::~SimulationWorker() {
    thread.wait();
}

Simulation* SimulationWorker::getSimulation() const {
    return simulation;
}

void SimulationWorker::pause() {
    simulation->setStatus(SimulationStatus::PAUSED);
}

void SimulationWorker::resume() {
    simulation->setStatus(SimulationStatus::RUNNING);
}

void SimulationWorker::simulate() {
    if (simulation->getStatus() == SimulationStatus::IDLE || simulation->getStatus() == SimulationStatus::PAUSED) {
        SimulationDataType::Simulation *simulationStruct = simulation->toSimulationDataType();
        SimulationProgressListener progressListener;
        
        SimulationRepository::updateSimulationStatus(simulation, SimulationStatus::RUNNING);
        
        connect(this, SIGNAL(listenProgress(Simulation*)), &progressListener, SLOT(listen(Simulation*)));
        emit listenProgress(simulation);
        startSimulation(simulationStruct);
    }
}