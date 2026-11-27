#include "analyzer.hpp"
// <sstream>: fornisce std::ostringstream per comporre stringhe formattate con numeri e testo (usato per l'alert message)
#include <sstream>
// <utility>: fornisce std::move per abilitare la move semantics (trasferimento di proprietà senza copia inutile)
#include <utility>

using namespace std;

namespace sentinel {

// ============================================================================
// CONCETTO DI FUNZIONE MEMBRO (Metodo) E RISOLUZIONE DI AMBITO:
// Tutte le funzioni seguenti sono "funzioni membro" della classe LogAnalyzer.
// La sintassi `LogAnalyzer::nomeFunzione` sfrutta l'operatore di risoluzione di
// visibilità/ambito (scope resolution operator `::`): serve a indicare al compilatore
// che non stiamo definendo una funzione libera globale, bensì l'implementazione del metodo
// dichiarato all'interno della classe `LogAnalyzer` nel file header analyzer.hpp.
// Essendo funzioni membro, operano sempre su una specifica istanza (oggetto) della classe
// tramite il puntatore implicito `this`, avendo così accesso diretto a tutte le variabili
// e attributi privati dell'oggetto (come report_, window_seconds_, ecc.).
// ============================================================================

// Costruttore:
// Utilizza la "Member Initializer List" (lista di inizializzazione dei membri, preceduta da `:`)
// per impostare direttamente i campi prima dell'esecuzione del corpo del costruttore.
// Include controlli di fallback tramite operatore ternario: se i parametri forniti non sono validi (<= 0),
// vengono assegnati valori di default ragionevoli (finestra di 60 secondi, soglia di 3 errori).
LogAnalyzer::LogAnalyzer(int64_t window_seconds, size_t error_threshold)
    : window_seconds_(window_seconds > 0 ? window_seconds : 60),
      error_threshold_(error_threshold > 0 ? error_threshold : 3) {}

// Funzione membro per la registrazione delle righe non parsabili/corrotte:
// Modifica lo stato interno del report incrementando sia il totale delle righe
// esaminate sia il contatore specifico delle righe malformate.
void LogAnalyzer::recordMalformedLine() {
    ++report_.total_lines;
    ++report_.malformed_lines;
}

// Funzione membro per l'elaborazione di una riga di log valida (LogEntry):
// Prende l'entry per riferimento costante (const LogEntry&) per evitare copie costose.
// Aggiorna le statistiche globali (linee, byte) e le tabelle hash di frequenza per IP ed endpoint.
// In caso di errore server (HTTP 5xx, codice tra 500 e 599 compresi), delega la verifica
// della finestra temporale al metodo privato check5xxWindow.
void LogAnalyzer::processEntry(const LogEntry& entry) {
    ++report_.total_lines;
    ++report_.parsed_entries;
    report_.total_bytes += entry.response_bytes;

    // operator[] su unordered_map: se la chiave (IP o endpoint) esiste già ne incrementa il valore,
    // altrimenti la inserisce automaticamente inizializzandola a 0 e poi incrementandola a 1.
    ++report_.ip_counts[entry.ip];
    ++report_.endpoint_counts[entry.endpoint];

    // Controllo errori server (status code 500-599)
    if (entry.status_code >= 500 && entry.status_code < 600) {
        check5xxWindow(entry);
    }
}

// Funzione membro privata per il rilevamento anomalie con finestra scorrevole (Sliding Window):
// Traccia i timestamp degli errori 5xx dentro una coda a due estremità (std::deque).
void LogAnalyzer::check5xxWindow(const LogEntry& entry) {
    // 1. Aggiunge in coda il timestamp UNIX (epoch) dell'errore appena rilevato
    recent_5xx_epochs_.push_back(entry.epoch_seconds);

    // 2. Sliding window (scorrimento): rimuove dal fronte della coda tutti i timestamp
    // che sono ormai più vecchi rispetto alla finestra temporale impostata (window_seconds_)
    while (!recent_5xx_epochs_.empty() &&
           (entry.epoch_seconds - recent_5xx_epochs_.front()) > window_seconds_) {
        recent_5xx_epochs_.pop_front();
    }

    // 3. Se il numero di errori rimasti nella finestra è inferiore alla soglia:
    // reimposta il flag di burst a false (il picco è terminato o non ancora critico) ed esce.
    if (recent_5xx_epochs_.size() < error_threshold_) {
        alert_emitted_in_current_burst_ = false;
        return;
    }

    // 4. Se la soglia è raggiunta o superata e non abbiamo ancora notificato questo picco (burst):
    // viene generato un nuovo allarme AnomalyAlert.
    if (!alert_emitted_in_current_burst_) {
        AnomalyAlert alert;
        alert.window_start_epoch = recent_5xx_epochs_.front(); // Timestamp del primo errore nella finestra
        alert.window_end_epoch = recent_5xx_epochs_.back();     // Timestamp dell'ultimo errore nella finestra
        alert.error_count = recent_5xx_epochs_.size();          // Totale errori accumulati nella finestra

        // Composizione del messaggio descrittivo tramite stream di stringa
        std::ostringstream oss;
        oss << "5xx spike detected: " << alert.error_count
            << " errors within " << window_seconds_
            << "s window (around " << entry.timestamp << " on " << entry.endpoint << ")";
        alert.message = oss.str();

        // std::move evita la copia dell'oggetto 'alert', trasferendone direttamente le risorse
        // nel vettore report_.alerts (move semantics).
        report_.alerts.push_back(std::move(alert));

        // Imposta il flag a true per evitare la generazione di allarmi ripetuti a ogni successivo
        // errore appartenente alla medesima sequenza ininterrotta di anomalie.
        alert_emitted_in_current_burst_ = true;
    }
}

// Funzione membro per la chiusura dell'analisi:
// Restituisce l'oggetto AnalysisReport consolidato con tutte le metriche e gli allarmi accumulati.
AnalysisReport LogAnalyzer::finalize() {
    return report_;
}

} // namespace sentinel
