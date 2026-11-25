#include "log_parser.hpp"
//Ci danno gli strumento per leggere una data testuale e convertirla in secondi unix, ctime, iomanip
#include <ctime>
#include <iomanip>
/*Ci fornisce istringstream che tratta una stringa come se fosse un flusso di input (cin), comodo per estarre numeri e parole separati da spazi*/
#include <sstream>

using namespace std;

namespace sentinel {
	optional<LogEntry> LogParser parseline::(string_view line){
		if(line.empty()) return nullopt;
	}
}
