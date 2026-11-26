#pragma once
/*Dice al compilatore di includere questo header una sola volta durante la compilazione, evitando errori di ridefinizione su piu' file .cpp.*/

/*Permette a diversi tipi di dati a prescindere dall'OS usato di avere una lunghezza predefinita (invece di variare in base al sistema operativo usato)*/ 
#include <cstdint>
#include <string>

using namespace std;

/*Contenitore logico che serve a raggruppare tutte le strutture, classi e funzioni del progetto sotto un unica etichetta detta "sentinel". cosi' facendo se ci fosse un altra struct di nome LogEntry il compilatore saprebbe come comportarsi perche' ufficialmente quest'ultima e' vista sotto il nome di sentinel::LogEntry*/ 
namespace sentinel {
	struct LogEntry {
		string ip;
		//Conserva la data e l'ora originali in formato leggibile
		string timestamp;
		//Stessa cosa di timestamp ma convertito in secondi trascorsi dal 1 gennaio 1970.
		int64_t epoch_seconds = 0;
		//Indica il verbo HTTP della richiesta
		string method;
		/*Il percorso URL richiesto sul server. (Serve per capire quali pagine o API ricevono piu' traffico o generano piu' errori)*/
		string endpoint;
		//Codice di risposta HTTp del server
		int status_code = 0;
		//Soomandoli tutti calcoliamo la banda totale consumata.
		size_t response_bytes = 0;
	};
}
