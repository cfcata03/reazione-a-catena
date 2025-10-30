# Relazione Progetto: Reazione a Catena

## Scelte Progettuali

### Architettura Generale
Il progetto simula una reazione nucleare a catena utilizzando processi concorrenti che comunicano tramite meccanismi IPC (Inter-Process Communication):

- **Shared Memory**: per le statistiche globali della simulazione
- **Semafori**: per la sincronizzazione dell'accesso alle risorse condivise
- **Code di Messaggi**: per la comunicazione attivatore-atomi

### Processi Implementati

#### 1. Processo Master (master.c)
- Crea e inizializza tutte le risorse IPC
- Genera i processi iniziali (atomi, attivatore, alimentazione)
- Coordina l'avvio sincronizzato di tutti i processi
- Stampa le statistiche ogni secondo
- Consuma energia (ENERGY_DEMAND per secondo)
- Monitora le condizioni di terminazione
- Gestisce la pulizia delle risorse alla terminazione

#### 2. Processo Atomo (atomo.c)
- Ogni atomo ha un numero atomico casuale (1 - N_ATOM_MAX)
- Attende messaggi di attivazione dalla coda messaggi
- Alla scissione:
  - Fork di un nuovo processo atomo
  - Divisione del numero atomico (preferibilmente uguale per massima energia)
  - Calcolo energia: `energy(n1, n2) = n1*n2 - max(n1, n2)`
  - Aggiornamento statistiche
- Atomi con numero atomico ≤ MIN_N_ATOMICO diventano scorie

#### 3. Processo Attivatore (attivatore.c)
- Invia periodicamente (ogni 100ms) messaggi di scissione
- Attiva casualmente 1-3 atomi per iterazione
- Utilizza code di messaggi con target_pid=0 (broadcast)

#### 4. Processo Alimentazione (alimentazione.c)
- Ogni STEP nanosecondi crea N_NUOVI_ATOMI nuovi atomi
- Usa execl per lanciare processi atomo separati
- Mantiene la reazione in corso

### Sincronizzazione

#### Semafori
Tre semafori sono utilizzati per la sincronizzazione:
- `SEM_STATS`: protegge l'accesso alle statistiche
- `SEM_ATOMS`: protegge il contatore degli atomi attivi
- `SEM_BARRIER`: può essere usato per barriere di sincronizzazione

#### Avvio Coordinato
1. Master crea tutti i processi
2. Ogni processo segnala l'inizializzazione incrementando `init_count`
3. Tutti i processi attendono che `running` diventi true
4. Master imposta `running=1` quando tutti sono pronti
5. La simulazione parte sincronizzata

### Condizioni di Terminazione

1. **TIMEOUT**: Raggiunto SIM_DURATION secondi
2. **EXPLODE**: Energia netta (prodotta - consumata) > ENERGY_EXPLODE_THRESHOLD
3. **BLACKOUT**: Energia disponibile < 0
4. **MELTDOWN**: Fallimento di fork() in qualsiasi processo

### Gestione Risorse

#### Allocazione
- Shared memory: una struttura Statistics condivisa
- Semafori: 3 semafori per sincronizzazione
- Message queue: una coda per messaggi di attivazione

#### Deallocazione
- Cleanup automatico all'uscita tramite `atexit()`
- Segnale ai processi figli di terminare
- SIGTERM broadcast ai processi rimanenti
- Wait su tutti i processi figli
- Rimozione esplicita di tutte le risorse IPC

### Configurazione

Tutti i parametri sono caricati da variabili d'ambiente:
- `N_ATOMI_INIT`: numero atomi iniziali (default: 10)
- `N_ATOM_MAX`: numero atomico massimo (default: 100)
- `MIN_N_ATOMICO`: soglia per diventare scoria (default: 5)
- `ENERGY_DEMAND`: energia consumata al secondo (default: 50)
- `ENERGY_EXPLODE_THRESHOLD`: soglia esplosione (default: 10000)
- `SIM_DURATION`: durata simulazione in secondi (default: 30)
- `STEP`: nanosecondi tra aggiunte atomi (default: 1000000000)
- `N_NUOVI_ATOMI`: atomi aggiunti ogni STEP (default: 2)

Nessuna ricompilazione necessaria per cambiare parametri.

### Modularità

Il codice è diviso in moduli:
- `config.h/c`: gestione configurazione
- `shared.h/c`: funzioni IPC condivise
- `master.c`: processo principale
- `atomo.c`: processo atomo
- `attivatore.c`: processo attivatore
- `alimentazione.c`: processo alimentazione

Ogni processo è un eseguibile separato lanciato con `execl()`.

### Adattamenti per macOS

- Rimossa ridefinizione `union semun` (già presente in macOS)
- Usa flag `-D_GNU_SOURCE` per compatibilità POSIX estesa
- Testato su macOS Darwin kernel

### Evitare Busy Waiting

- Messaggi bloccanti con `msgrcv()`
- Semafori bloccanti
- `sleep()` e `nanosleep()` per attese temporizzate
- Nessun polling attivo

### Concorrenza

- Tutti i processi eseguono in parallelo
- Sincronizzazione tramite semafori solo quando necessario
- Aggiornamenti atomici delle statistiche
- Minimizzazione delle sezioni critiche

## Compilazione

```bash
make
```

Opzioni GCC utilizzate: `-Wvla -Wextra -Werror -D_GNU_SOURCE`

## Test

Forniti 3 script di test per verificare le diverse condizioni di terminazione:
- `run_timeout.sh`: verifica TIMEOUT
- `run_explode.sh`: verifica EXPLODE
- `run_blackout.sh`: verifica BLACKOUT

MELTDOWN si verifica automaticamente in caso di esaurimento risorse di sistema.

## Statistiche Visualizzate

Ogni secondo:
- Attivazioni (totali e ultimo secondo)
- Scissioni (totali e ultimo secondo)
- Energia prodotta (totali e ultimo secondo)
- Energia consumata (totali e ultimo secondo)
- Energia corrente disponibile
- Scorie prodotte (totali e ultimo secondo)
- Numero atomi attivi

## Note di Implementazione

- La funzione energia è massima quando n1 = n2 (scissione simmetrica)
- Gli atomi possono generare catene molto lunghe di scissioni
- Il processo alimentazione mantiene la reazione attiva
- Il processo attivatore può causare reazioni a catena esponenziali
- Le configurazioni possono essere bilanciate per ottenere diverse terminazioni
