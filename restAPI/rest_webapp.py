#!/usr/bin/python3
# -*- coding: UTF-8 -*-

plik_bazy = './osoby.sqlite'

import re, sqlite3
from urllib.parse import parse_qs

class OsobyApp:
    def __init__(self, environment, start_response):
        self.env = environment
        self.start_response = start_response
        self.status = '200 OK'
        self.headers = [ ('Content-Type', 'text/html; charset=UTF-8') ]
        self.content = b''

    def __iter__(self):
        try:
            self.route()
        except sqlite3.Error as e:
            self.failure('500 Internal Server Error', f'SQLite error: {e}')
        self.headers.append(('Content-Length', str(len(self.content))))
        self.start_response(self.status, self.headers)
        yield self.content

    def failure(self, status, detail=None):
        self.status = status
        s = f'<html><head><title>{status}</title></head><body><h1>{status}</h1>'
        if detail:
            s += f'<p>{detail}</p>'
        s += '</body></html>'
        self.content = s.encode('UTF-8')

    def route(self):
        path = self.env['PATH_INFO']

        if path == '/osoby':
            self.handle_table_persons()
            return
        if path == '/osoby/search':
            self.handle_search_persons()
            return
        m = re.match(r'^/osoby/(\d+)$', path)
        if m:
            self.handle_item_persons(int(m.group(1)))
            return

        if path == '/psy':
            self.handle_table_dogs()
            return
        if path == '/psy/search':
            self.handle_search_dogs()
            return
        m = re.match(r'^/psy/(\d+)$', path)
        if m:
            self.handle_item_dogs(int(m.group(1)))
            return

        self.failure('404 Not Found')

    def handle_table_persons(self):
        if self.env['REQUEST_METHOD'] == 'GET':
            colnames, rows = self.sql_select_persons()
            self.send_rows(colnames, rows)
        elif self.env['REQUEST_METHOD'] == 'POST':
            colnames, vals = self.read_tsv()
            q = f'INSERT INTO osoby ({", ".join(colnames)}) VALUES ({", ".join(["?"] * len(vals))})'
            id = self.sql_modify_persons(q, vals)
            colnames, rows = self.sql_select_persons(id)
            self.send_rows(colnames, rows)
        else:
            self.failure('501 Not Implemented')

    def handle_item_persons(self, id):
        if self.env['REQUEST_METHOD'] == 'GET':
            colnames, rows = self.sql_select_persons(id)
            self.send_rows(colnames, rows) if rows else self.failure('404 Not Found')

        elif self.env['REQUEST_METHOD'] == 'PUT':
            colnames, vals = self.read_tsv()
            q = f'UPDATE osoby SET {", ".join([f"{c}=?" for c in colnames])} WHERE id={id}'
            self.sql_modify_persons(q, vals)
            colnames, rows = self.sql_select_persons(id)
            self.send_rows(colnames, rows)

        elif self.env['REQUEST_METHOD'] == 'DELETE':
            try:
                q = 'DELETE FROM osoby WHERE id = ' + str(id)
                self.sql_modify_persons(q)
            except sqlite3.IntegrityError:
                self.failure('409 Conflict', 'Can not delete person who owns dog.')
                return

        else:
            self.failure('501 Not Implemented')


    def handle_search_persons(self):
        if self.env['REQUEST_METHOD'] != 'GET':
            self.failure('405 Method Not Allowed')
            return
        params = parse_qs(self.env.get('QUERY_STRING', ''))
        where = []
        if 'imie' in params:
            where.append(f'imie="{params["imie"][0]}"')
        if 'nazwisko' in params:
            where.append(f'nazwisko="{params["nazwisko"][0]}"')
        if not where:
            self.failure('400 Bad Request', 'Brak parametrów wyszukiwania')
            return
        where_clause = " AND ".join(where)
        colnames, rows = self.sql_search_persons(where_clause)
        self.send_rows(colnames, rows)

    def sql_select_persons(self, id=None):
        conn = sqlite3.connect(plik_bazy)
        conn.execute("PRAGMA foreign_keys = ON")
        crsr = conn.cursor()
        query = 'SELECT * FROM osoby' + (f' WHERE id={id}' if id else '')
        crsr.execute(query)
        result = crsr.fetchall()
        colnames = [d[0] for d in crsr.description]
        conn.close()
        return colnames, result

    def sql_modify_persons(self, query, params=None):
        conn = sqlite3.connect(plik_bazy)
        conn.execute("PRAGMA foreign_keys = ON")
        crsr = conn.cursor()
        crsr.execute(query, params or [])
        rowid = crsr.lastrowid
        conn.commit()
        conn.close()
        return rowid

    def sql_search_persons(self, where):
        conn = sqlite3.connect(plik_bazy)
        conn.execute("PRAGMA foreign_keys = ON")
        crsr = conn.cursor()
        crsr.execute(f'SELECT * FROM osoby WHERE {where}')
        result = crsr.fetchall()
        colnames = [d[0] for d in crsr.description]
        conn.close()
        return colnames, result

    def handle_table_dogs(self):
        if self.env['REQUEST_METHOD'] == 'GET':
            colnames, rows = self.sql_select_dogs()
            self.send_rows(colnames, rows)
        elif self.env['REQUEST_METHOD'] == 'POST':
            colnames, vals = self.read_tsv()
            q = f'INSERT INTO psy ({", ".join(colnames)}) VALUES ({", ".join(["?"] * len(vals))})'
            id = self.sql_modify_dogs(q, vals)
            colnames, rows = self.sql_select_dogs(id)
            self.send_rows(colnames, rows)
        else:
            self.failure('501 Not Implemented')

    def handle_item_dogs(self, id):
        if self.env['REQUEST_METHOD'] == 'GET':
            colnames, rows = self.sql_select_dogs(id)
            self.send_rows(colnames, rows) if rows else self.failure('404 Not Found')
        elif self.env['REQUEST_METHOD'] == 'PUT':
            colnames, vals = self.read_tsv()
            q = f'UPDATE psy SET {", ".join([f"{c}=?" for c in colnames])} WHERE id={id}'
            self.sql_modify_dogs(q, vals)
            colnames, rows = self.sql_select_dogs(id)
            self.send_rows(colnames, rows)
        elif self.env['REQUEST_METHOD'] == 'DELETE':
            self.sql_modify_dogs(f'DELETE FROM psy WHERE id={id}')
        else:
            self.failure('501 Not Implemented')

    def handle_search_dogs(self):
        if self.env['REQUEST_METHOD'] != 'GET':
            self.failure('405 Method Not Allowed')
            return
        params = parse_qs(self.env.get('QUERY_STRING', ''))
        where = []
        if 'imie' in params:
            where.append(f'imie="{params["imie"][0]}"')
        if 'rasa' in params:
            where.append(f'rasa="{params["rasa"][0]}"')
        if 'wlascicielID' in params:
            where.append(f'wlascicielID="{params["wlascicielID"][0]}"')
        if not where:
            self.failure('400 Bad Request', 'Brak parametrów wyszukiwania')
            return
        where_clause = " AND ".join(where)
        colnames, rows = self.sql_search_dogs(where_clause)
        self.send_rows(colnames, rows)

    def sql_select_dogs(self, id=None):
        conn = sqlite3.connect(plik_bazy)
        conn.execute("PRAGMA foreign_keys = ON")
        crsr = conn.cursor()
        query = 'SELECT * FROM psy' + (f' WHERE id={id}' if id else '')
        crsr.execute(query)
        result = crsr.fetchall()
        colnames = [d[0] for d in crsr.description]
        conn.close()
        return colnames, result

    def sql_modify_dogs(self, query, params=None):
        conn = sqlite3.connect(plik_bazy)
        conn.execute("PRAGMA foreign_keys = ON")
        crsr = conn.cursor()
        crsr.execute(query, params or [])
        rowid = crsr.lastrowid
        conn.commit()
        conn.close()
        return rowid

    def sql_search_dogs(self, where):
        conn = sqlite3.connect(plik_bazy)
        conn.execute("PRAGMA foreign_keys = ON")
        crsr = conn.cursor()
        crsr.execute(f'SELECT * FROM psy WHERE {where}')
        result = crsr.fetchall()
        colnames = [d[0] for d in crsr.description]
        conn.close()
        return colnames, result

    def read_tsv(self):
        f = self.env['wsgi.input']
        n = int(self.env.get('CONTENT_LENGTH', 0))
        raw_bytes = f.read(n)
        lines = raw_bytes.decode('UTF-8').splitlines()
        colnames = lines[0].split('\t')
        vals = lines[1].split('\t')
        return colnames, vals

    def send_rows(self, colnames, rows):
        s = '\t'.join(colnames) + '\n'
        for row in rows:
            s += '\t'.join([str(v) for v in row]) + '\n'
        self.content = s.encode('UTF-8')
        self.headers = [('Content-Type', 'text/tab-separated-values; charset=UTF-8')]

if __name__ == '__main__':
    from wsgiref.simple_server import make_server
    port = 8000
    print(f'Listening on port {port}...')
    httpd = make_server('', port, OsobyApp)
    httpd.serve_forever()
