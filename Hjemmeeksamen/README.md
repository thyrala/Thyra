# Midtveiseksamen IN2140 V21
I dette prosjektet skulle vi lage en server-klient-tjeneste der klienter kunne koble seg opp til en server for å hente/bli tilsendt filer. Vi opprettet et nytt transportlag, Reliable Datagram Protocol (RDP), som tilbyr multipleksing og pålitelighet.

Hele oppgaven er inspirert av filtjenesteprotokollen FSP, som overfører store filer (som en operativsystemkjerne) over UDP.

For å bevise at vi tilbyr pålitelighet ble det også lagt til en implementasjon som gjør at UDP mister noen av pakkene som blir sendt. Dette måtte vi derfor finne en løsning på, slik at alle pakkene kommer frem i riktig rekkefølge.

## Krav
- C
- gcc for kompilering

## Hvordan kjøre

Bruk Makefile for å kompilere
```
make all
``` 

Bruk også Makefile til å kjøre systemet.

For å kjøre server bruk
```
make runS
``` 

For å kjøre klient bruk
```
make runC
``` 
