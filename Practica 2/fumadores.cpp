#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"

using namespace std;
using namespace HM;

const int num_est = 1, num_fum = 3;

template< int min, int max > int aleatorio() {
	static default_random_engine generador( (random_device())() );
	static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
	return distribucion_uniforme( generador );
}

class Estanco : public HoareMonitor {
	private:
		static const int tam = 1;
		int mostrador[tam];
		int puntero;		
		CondVar condEstanquero, condFumador[num_fum];

	public:
		Estanco();
		void obtenerIngrediente(int num_fumador);
		void ponerIngrediente(int ingrediente);
} ;

Estanco::Estanco() {
	puntero = 0;
	for (int i=0; i<tam; i++)
		mostrador[i] = -1;

	condEstanquero = newCondVar();
	for (int i=0; i<num_fum; i++)
		condFumador[i] = newCondVar();
}

void Estanco::obtenerIngrediente(int num_fumador) {
	while (puntero == 0 || mostrador[puntero-1] != num_fumador)
		condFumador[num_fumador].wait();

	cout << "\tFumador " << num_fumador << " toma ingrediente del mostrador." << endl;
	puntero--;
	mostrador[puntero] = -1;
	condEstanquero.signal();
}

void Estanco::ponerIngrediente(int ingrediente) {
	while(puntero == tam)
		condEstanquero.wait();
	
	cout << "Estanquero pone ingrediente " << ingrediente << " en el mostrador." << endl;
	mostrador[puntero] = ingrediente;
	puntero++;
	condFumador[ingrediente].signal();
}

int producirIngrediente() {
	chrono::milliseconds duracion(aleatorio<20,200>());
	this_thread::sleep_for(duracion);
	
	int ingrediente = aleatorio<0,2>();
	cout << "Estanquero produce ingrediente " << ingrediente << endl;
	return ingrediente;
}

void funcion_hebra_estanquero(MRef<Estanco> estanco) {
	while(true) {
		int ingrediente = producirIngrediente();
		estanco->ponerIngrediente(ingrediente);
	}
}

void fumar(int num_fumador) {
	chrono::milliseconds duracion(aleatorio<200,2000>());
	cout << "\tFumador " << num_fumador << " empieza a fumar (" << duracion.count() << ")." << endl;
	this_thread::sleep_for(duracion);
	cout << "\tFumador " << num_fumador << " termina de fumar, espera ingredientes." << endl;
}

void funcion_hebra_fumador(MRef<Estanco> estanco, int num_fumador) {
	while(true) {
		estanco->obtenerIngrediente(num_fumador);
		fumar(num_fumador);
	}
}

int main() {
	cout << "-----------------------------------------------------"	<< endl
		 << "Problema de los fumadores (Monitor SU, buffer LIFO). "	<< endl
		 << "-----------------------------------------------------"	<< endl
		 << flush ;

	MRef<Estanco> estanco = Create<Estanco>() ;

	thread estanquero[num_est], fumador[num_fum];

	for (int i=0; i<num_est; i++)
		estanquero[i] = thread(funcion_hebra_estanquero, estanco);

	for (int i=0; i<num_fum; i++)
		fumador[i] = thread(funcion_hebra_fumador, estanco, i);	

	for (int i=0; i<num_est; i++)
		estanquero[i].join();

	for (int i=0; i<num_fum; i++)
		fumador[i].join();
}
