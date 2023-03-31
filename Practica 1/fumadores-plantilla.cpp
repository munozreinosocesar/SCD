#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;


Semaphore mostr_vacio = 1, s[3] = {0,0,0};



//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------


template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
  while(true){
    sem_wait(mostr_vacio);
    int obj = aleatorio<0,2>();

    if (obj == 0){
      cout << "Ingrediente : cerillas en el mostrador" << endl;
    }else if(obj == 1){
      cout << "Ingrediente : tabaco en el mostrador" << endl;
    }else if(obj == 2){
      cout << "Ingrediente : papel en el mostrador" << endl;
    }

    sem_signal(s[obj]);
  }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
    sem_wait( s[num_fumador] ) ;
    if (num_fumador == 0){
      cout << "Ingrediente : cerillas retirado del mostrador" << endl;
    }else if(num_fumador == 1){
      cout << "Ingrediente : tabaco retirado del mostrador" << endl;
    }else if(num_fumador == 2){
      cout << "Ingrediente : papel retirado del mostrador" << endl;
    }
    sem_signal( mostr_vacio );
    fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
  thread  estanquero ( funcion_hebra_estanquero),
          fumador0 ( funcion_hebra_fumador,0 ),
          fumador1 ( funcion_hebra_fumador,1 ),
          fumador2 ( funcion_hebra_fumador,2 );

  fumador0.join();
  fumador1.join();
  fumador2.join();



}
