#include "log_parser.hpp"
// Ci danno gli strumenti per leggere una data testuale e convertirla in secondi unix (ctime, iomanip)
#include <ctime>
#include <iomanip>
/* Ci fornisce istringstream che tratta una stringa come se fosse un flusso di input (cin), 
   comodo per estrarre numeri e parole separati da spazi */
#include <sstream>

using namespace std;

namespace sentinel {

    // parseLine: analizza una singola riga di log e restituisce un oggetto LogEntry opzionale.
    // L'uso di std::string_view permette di scorrere e analizzare la riga senza copiare memoria (zero-copy parsing).
    optional<LogEntry> LogParser::parseLine(string_view line) {
        if (line.empty()) return nullopt;

        LogEntry entry;

        // 1. Estrazione IP (dall'inizio fino al primo spazio)
        // .find(' '): cerca la prima occorrenza del carattere spazio partendo dall'inizio (indice 0).
        size_t ip_end = line.find(' ');
        // string_view::npos: costante sentinella che indica che il carattere cercato non è stato trovato.
        if (ip_end == string_view::npos) return nullopt;
        
        // .substr(pos, count): estrae una sottostringa a partire da indice 0 per la lunghezza ip_end.
        // Convertiamo la view in std::string poiché entry.ip richiede la memorizzazione del valore.
        entry.ip = string(line.substr(0, ip_end));

        // 2. Estrazione Timestamp (tra '[' e ']')
        // .find('[', ip_end): cerca la parentesi quadra aperta partendo dalla fine dell'IP per ottimizzare la ricerca.
        size_t ts_start = line.find('[', ip_end);
        // .find(']', ts_start): cerca la parentesi quadra chiusa a partire dalla posizione di '['.
        size_t ts_end = line.find(']', ts_start);
        
        // Se uno dei due delimitatori non è presente (valore restituito pari a string_view::npos), il log è malformato.
        if (ts_start == string_view::npos || ts_end == string_view::npos) {
            return nullopt;
        }
        
        // .substr(ts_start + 1, ts_end - ts_start - 1):
        // ts_start + 1 esclude la quadra aperta, mentre (ts_end - ts_start - 1) calcola la lunghezza esatta del testo racchiuso.
        entry.timestamp = string(line.substr(ts_start + 1, ts_end - ts_start - 1));
        entry.epoch_seconds = parseTimestampToEpoch(entry.timestamp);

        // 3. Estrazione Metodo ed Endpoint (tra '"', ts_end)
        size_t req_start = line.find('"', ts_end);
        size_t req_end = line.find('"', req_start + 1);
        if (req_start == string_view::npos || req_end == string_view::npos) {
          return nullopt;
        }

        string_view request = line.substr(req_start + 1, req_end - req_start - 1);
        size_t method_end = request.find(' ');
        size_t path_end = request.find(' ', method_end + 1);
        if (method_end == string_view::npos || path_end == string_view::npos) {
          return nullopt;
        }
        entry.method = string(request.substr(0, method_end));
        entry.endpoint = string(request.substr(method_end + 1, path_end - method_end - 1));

        // 4. Estrazione Status Code e Response Bytes.
        // Viene convertita in std::string perché std::istringstream necessita di una stringa con memoria gestita o compatibile con gli stream.
        string rest(line.substr(req_end + 1));

        // std::istringstream iss(rest): crea uno stream di input a partire dalla sottostringa rimanente.
        // Questo permette di trattare la stringa come uno stream (simile a cin), delegando il parsing dei tipi numerici.
        istringstream iss(rest);

        // L'operatore di estrazione '>>' salta automaticamente gli spazi bianchi iniziali e intermedi,
        // convertendo la prima sequenza di cifre in un intero per status_code e la successiva in un intero/size_t per response_bytes.
        // Se la conversione fallisce (es. formato errato, caratteri non numerici o dati mancanti),
        // il flusso imposta il failbit e l'espressione booleana !(iss >> ...) risulta vera, scartando la riga.
        if (!(iss >> entry.status_code >> entry.response_bytes)) {
          return nullopt;
        }

        return entry;
    }


    // parseTimestampToEpoch: converte una data/ora in formato testuale Apache/Nginx (es. "10/Oct/2000:13:55:36 -0700")
    // nel corrispettivo timestamp UNIX in secondi (epoch time), utile per ordinamenti e calcoli temporali.
    int64_t LogParser::parseTimestampToEpoch(string_view ts) {
        // Controllo di guardia sulla lunghezza: "DD/Mon/YYYY:HH:MM:SS" richiede almeno 20 caratteri.
        if (ts.size() < 20) return 0;

        string date_part(ts.substr(0, 20));

        // Inizializza la struttura C `tm` con tutti i campi azzerati ({}) per evitare valori indefiniti.
        tm tm{};
        istringstream ss(date_part);

        // std::get_time analizza lo stream secondo il pattern specificato:
        // %d = giorno del mese (01-31)
        // %b = nome abbreviato del mese (es. Jan, Feb, Oct)
        // %Y = anno a 4 cifre
        // %H:%M:%S = ore, minuti e secondi
        // e popola i campi corrispondenti della struttura `tm`.
        ss >> get_time(&tm, "%d/%b/%Y:%H:%M:%S");

        // Se l'analisi della data fallisce (formato non valido o testo errato), ss.fail() ritorna true.
        if (ss.fail()) return 0;

        // timegm(&tm): interpreta la struttura `tm` come orario UTC e calcola i secondi trascorsi dal 1° gennaio 1970 (Epoch).
        // static_cast<int64_t>(...): converte il risultato (time_t) in un intero a 64 bit con segno.
        return static_cast<int64_t>(timegm(&tm));
    } 

} // namespace sentinel
