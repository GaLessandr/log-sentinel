#pragma once

// Header del progetto per la definizione della struttura dati del singolo record di log
#include "log_entry.hpp"

// Librerie standard C++ utilizzate:
// <cstddef>: definisce tipi fondamentali come std::size_t (intero senza segno per conteggi e dimensioni)
#include <cstddef>
// <cstdint>: definisce tipi a dimensione intera garantita, come int64_t (necessario per epoch time a 64 bit)
#include <cstdint>
// <deque>: std::deque (double-ended queue), coda a doppia estremità usata per implementare efficientemente una sliding window (finestra temporale scorrevole con push sul retro e pop sul fronte)
#include <deque>
#include <string>
// <unordered_map>: tabella hash per associazioni chiave-valore con accesso medio O(1) (ideale per conteggi aggregati di frequenza)
#include <unordered_map>
#include <vector>

using namespace std;

namespace sentinel {

// AnomalyAlert: rappresenta una notifica/allarme generato quando viene rilevata un'anomalia
struct AnomalyAlert {
    int64_t window_start_epoch = 0;  // Timestamp UNIX (in secondi) di inizio della finestra di rilevazione
    int64_t window_end_epoch = 0;    
    size_t error_count = 0;          
    string message;             
};

// AnalysisReport: raccoglie i risultati aggregati e le statistiche complessive dell'analisi di un file di log.
struct AnalysisReport {
    size_t total_lines = 0;          // Numero totale di righe esaminate (sia valide che errate)
    size_t parsed_entries = 0;       // Numero di voci di log parsate ed elaborate con successo
    size_t malformed_lines = 0;      
    size_t total_bytes = 0;          
    
    // Mappe hash per conteggio frequenze (chiave: stringa, valore: conteggio occorrenze)
    unordered_map<string, size_t> ip_counts;        // Conteggio richieste per ciascun indirizzo IP
    unordered_map<string, size_t> endpoint_counts;  // Conteggio richieste per ciascun endpoint/URL
    
    // Lista sequenziale di tutti gli alert di anomalia scattati durante l'analisi
    vector<AnomalyAlert> alerts;
};

// LogAnalyzer: classe responsabile dell'elaborazione analitica sequenziale dei log,
// del calcolo delle metriche aggregate e del rilevamento delle anomalie tramite finestra temporale scorrevole.
class LogAnalyzer {
public:
    // Costruttore:
    // - window_seconds: ampiezza della finestra temporale di osservazione espressa in secondi
    // - error_threshold: soglia di errori oltre la quale deve essere generato un alert di anomalia
    LogAnalyzer(int64_t window_seconds, size_t error_threshold);

    // recordMalformedLine: incrementa i contatori relativi alle righe che non rispettano il formato atteso
    void recordMalformedLine();

    // processEntry: elabora un record di log valido aggiornando le statistiche complessive
    // (byte totali, conteggio IP, conteggio endpoint) e verificando eventuali picchi di errori
    void processEntry(const LogEntry& entry);

    // finalize: conclude la sessione di analisi e restituisce l'AnalysisReport consolidato con tutte le metriche
    AnalysisReport finalize();

private:
    int64_t window_seconds_;                          // Ampiezza della finestra scorrevole in secondi
    size_t error_threshold_;                          // Soglia limite di errori per generare un alert
    AnalysisReport report_;                           // Istanza interna del report che accumula i dati aggregati
    deque<int64_t> recent_5xx_epochs_;                // Coda circolare dei timestamp (epoch) degli errori 5xx recenti per gestire la finestra temporale
    bool alert_emitted_in_current_burst_ = false;     // Flag per evitare alert duplicati durante lo stesso picco (burst) consecutivo

    // check5xxWindow: metodo privato che gestisce lo scorrimento della deque degli errori HTTP 5xx,
    // eliminando gli eventi fuori finestra e scatenando un AnomalyAlert se viene superata la soglia
    void check5xxWindow(const LogEntry& entry);
};

} // namespace sentinel
