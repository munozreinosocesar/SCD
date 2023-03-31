#include <iostream>
#include <mutex>
#include <cassert>
#include <thread>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"

using namespace std;
using namespace HM;

const int num_clientes = 3;

template< int min, int max > int aleatorio() {
	static default_random_engine generador( (random_device())() );
	static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
	return distribucion_uniforme( generador );
}

class Barberia : public HoareMonitor {
	private:
		CondVar condEspera, condSilla, condDormir;

	public:
		Barberia();
		void siguienteCliente();
		void cortarPeloCliente(int num_cliente);
		void finCliente();
};

Barberia::Barberia() {
	condEspera = newCondVar();
	condSilla = newCondVar();
	condDormir = newCondVar();
}

void Barberia::siguienteCliente() {
	if (condEspera.get_nwt() == 0) {
		cout << "Barbero se duerme." << endl;
		condDormir.wait();
	}

	condEspera.signal();
}

void Barberia::cortarPeloCliente(int num_cliente) {
	cout << "\tEntra el cliente " << num_cliente << "." << endl;

	while (condDormir.get_nwt() == 1) {
		cout << "Barbero se despierta." << endl;
		condDormir.signal();
	}

	while (condSilla.get_nwt() == 1) {
		condEspera.wait();
	}

	cout << "Empieza a cortar el pelo al cliente " << num_cliente << "." << endl;
	condSilla.wait();
}

void Barberia::finCliente() {
	cout << "Barbero termina de cortar." << endl;
	condSilla.signal();
}

void cortarPeloACliente() {
	chrono::milliseconds duracion(aleatorio<200,2000>());
	this_thread::sleep_for(duracion);
}

void esperarFueraBarberia(int num_cliente) {
	chrono::milliseconds duracion(aleatorio<200,2000>());
	this_thread::sleep_for(duracion);
}

void funcion_hebra_barbero(MRef<Barberia> barberia) {
	while(true) {
		barberia->siguienteCliente();
		cortarPeloACliente();
		barberia->finCliente();
	}
}

void funcion_hebra_cliente(MRef<Barberia> barberia, int num_cliente) {
	while(true) {
		barberia->cortarPeloCliente(num_cliente);
		esperarFueraBarberia(num_cliente);
	}
}

int main() {
	cout << "-------------------------------------" << endl
		 << "Problema de la barberia (Monitor SU)." << endl
		 << "-------------------------------------" << endl
		 << flush ;

	MRef<Barberia> barberia = Create<Barberia>() ;

	thread barbero, cliente[num_clientes];

	barbero = thread(funcion_hebra_barbero, barberia);

	for (int i=0; i<num_clientes; i++)
		cliente[i] = thread(funcion_hebra_cliente, barberia, i);

	barbero.join();

	for (int i=0; i<num_clientes; i++)
		cliente[i].join();
}
