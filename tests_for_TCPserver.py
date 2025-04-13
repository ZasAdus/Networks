import socket
import unittest
import time

SERVER_IP = '127.0.0.1'
SERVER_PORT = 2020

def send_message(message: str) -> str:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((SERVER_IP, SERVER_PORT))
        sock.sendall((message + "\r\n").encode("ascii"))
        response = sock.recv(1024)
        return response.decode("ascii").strip()

def send_fragmented_message(fragments):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((SERVER_IP, SERVER_PORT))
        for fragment in fragments:
            sock.sendall(fragment.encode('ascii'))
            time.sleep(0.1)
        response = sock.recv(1024)
        return response.decode('ascii').strip()
    
def send_raw_message(raw_bytes):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((SERVER_IP, SERVER_PORT))
        sock.sendall(raw_bytes)
        response = sock.recv(1024)
        return response.decode("ascii").strip()

class TestTCPServer(unittest.TestCase):

    def test_simple_valid(self):
        res = send_message("kajak ala mak")
        self.assertEqual(res, "2/3")  

    def test_spaces(self):
        for msg in [" ala", "ala ", "ala i  kot"]:
            with self.subTest(msg=msg):
                self.assertEqual(send_message(msg), "ERROR")

    def test_nonalpha_characters(self):
        for msg in ["ala i2i", "1ala", "ala i kot2"]:
            with self.subTest(msg=msg):
                self.assertEqual(send_message(msg), "ERROR")

    def test_valid_with_one_palindrome(self):
        res = send_message("kot ala pies")
        self.assertEqual(res, "1/3")  

    def test_empty_string(self):
        res = send_message("")
        self.assertEqual(res, "0/0")

    def test_uppercase_palindromes(self):
        res = send_message("Ala KAJAK")
        self.assertEqual(res, "2/2")

    def test_invalid_null_terminator(self):
        res = send_raw_message(b"kajak a\x00la\r\n")
        self.assertEqual(res, "ERROR")

    def test_invalid_newline_in_message(self):
        res = send_message("kajak a\r\nla")
        self.assertEqual(res, "2/2\r\n0/1")
    
    def test_fragmented_message(self):
        res = send_fragmented_message(['a', 'l', 'a', '\r\n'])
        self.assertEqual(res, "1/1")
        
        res = send_fragmented_message(['k', 'aj', 'ak ', 'a', 'l', 'a', '\r\n'])
        self.assertEqual(res, "2/2")
        
        res = send_fragmented_message(['ala ', ' ', 'kot', '\r\n'])
        self.assertEqual(res, "ERROR")

if __name__ == '__main__':
    unittest.main()