#!/bin/sh

# Skrypt uruchamiający i demonstrujący działanie aplikacji z pliku
# rest_webapp.py. Skrypt przetestowano pod Debianem 7, czyli dystrybucją
# zainstalowaną w pracowniach studenckich.
#
# Przy ręcznym testowaniu  webaplikacji możesz chcieć użyć "curl -v" aby
# zobaczyć nagłówki zapytań i odpowiedzi HTTP.


# Zainicjuj bazę z danymi osób.

rm -f osoby.sqlite

sqlite3 osoby.sqlite "
CREATE TABLE osoby (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    imie VARCHAR,
    nazwisko TEXT,
    telefon TEXT,
    adres TEXT
);
CREATE TABLE psy (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    imie TEXT,
    rasa TEXT,
    wlascicielID INTEGER,
    FOREIGN KEY (wlascicielID) REFERENCES osoby(id) ON DELETE RESTRICT
);
INSERT INTO osoby VALUES (1, 'Anna', 'Nowak', '+48124569988',
    'Rynek Główny 2, 30-001 Kraków');
INSERT INTO osoby VALUES (2, 'Jan', 'Kowalski', '+48127770022',
    'ul. Podzamcze 1, 31-001 Kraków');

INSERT INTO psy (imie, wlascicielID, rasa)
    VALUES ('Reksio', '1', 'Beagle');

INSERT INTO psy (imie, wlascicielID, rasa)
    VALUES ('Azor', '2', 'Owczarek niemiecki');

INSERT INTO psy (imie, wlascicielID, rasa)
VALUES ('Burek', NULL, 'Kundel');

INSERT INTO psy (imie, wlascicielID, rasa)
VALUES ('Maks', NULL, 'Husky');

INSERT INTO psy (imie, wlascicielID, rasa)
VALUES ('Luna', '1', 'Golden Retriever');
"

# Dla pewności wypisz na ekran jej zawartość.

echo "Początkowa zawartość bazy:"
sqlite3 --header osoby.sqlite "SELECT * FROM osoby"

# Uruchom w tle serwer z webaplikacją.

env -i python3 -u rest_webapp.py > stdout.txt 2> stderr.txt &
server_pid=$!
sleep 1

# Testy aplikacji:

echo
echo "Test 1: pobieranie rekordu"
curl http://127.0.0.1:8000/osoby/1

echo
echo "Test 2: uaktualnianie rekordu"
printf "nazwisko\tadres\n" > dane.tsv
printf "Kowalska\tul. Podzamcze 1, 31-001 Kraków\n" >> dane.tsv
curl --upload-file dane.tsv \
        --header "Content-Type: text/tab-separated-values; charset=UTF-8" \
        http://127.0.0.1:8000/osoby/1
# użycie opcji --upload-file zmienia domyślną metodę na PUT

echo
echo "Test 3: usuwanie rekordu"
curl --request DELETE http://127.0.0.1:8000/osoby/2

echo
echo "Test 4: dodawanie nowego rekordu"
printf "imie\tnazwisko\ttelefon\tadres\n" > dane.tsv
printf "Adam\tWiśniewski\t+48120124433\tul. Reymonta 4\n" >> dane.tsv
curl --request POST --upload-file dane.tsv \
        --header "Content-Type: text/tab-separated-values; charset=UTF-8" \
        http://127.0.0.1:8000/osoby

echo
echo "Test 5: pobieranie całej bazy"
curl http://127.0.0.1:8000/osoby

# I jeszcze upewnienie się co do zawartości pliku z bazą.

echo
echo "Zawartość bazy po zmianach:"
sqlite3 --header osoby.sqlite "SELECT * FROM osoby"

# Koniec testów, można wyłączyć serwer aplikacyjny.

kill $server_pid
