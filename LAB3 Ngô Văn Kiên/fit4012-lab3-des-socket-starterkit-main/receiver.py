import os
import socket
from des_socket_utils import HEADER_SIZE, parse_header, recv_exact, decrypt_des_cbc

HOST = os.getenv('RECEIVER_HOST', '0.0.0.0')
PORT = int(os.getenv('RECEIVER_PORT', '6000'))
TIMEOUT = float(os.getenv('SOCKET_TIMEOUT', '30'))
OUTPUT_FILE = os.getenv('RECEIVER_OUTPUT_FILE', '')
LOG_FILE = os.getenv('RECEIVER_LOG_FILE', '')


def write_log(line: str) -> None:
    print(line)
    if LOG_FILE:
        with open(LOG_FILE, 'a', encoding='utf-8') as f:
            f.write(line + '\n')


def main() -> None:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen(1)
        s.settimeout(TIMEOUT)
        write_log(f"[+] Đang lắng nghe {HOST}:{PORT} ...")

        try:
            conn, addr = s.accept()
        except TimeoutError:
            write_log(f"[!] Timeout: không có sender kết nối trong {TIMEOUT} giây.")
            return

        with conn:
            conn.settimeout(TIMEOUT)
            write_log(f"[+] Kết nối từ {addr}")
            try:
                header = recv_exact(conn, HEADER_SIZE)
                key, iv, length = parse_header(header)
                if length <= 0:
                    raise ValueError("Độ dài ciphertext không hợp lệ.")
                cipher_bytes = recv_exact(conn, length)
                plaintext = decrypt_des_cbc(key, iv, cipher_bytes)
                message = plaintext.decode('utf-8', errors='ignore')
                line = f"[+] Bản tin gốc: {message}"
                write_log(line)

                if OUTPUT_FILE:
                    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
                        f.write(message)
            except TimeoutError:
                write_log("[!] Timeout khi đang nhận dữ liệu từ sender.")
            except ConnectionError as e:
                write_log(f"[!] Lỗi kết nối: {e}")
            except ValueError as e:
                write_log(f"[!] Lỗi dữ liệu/giao thức: {e}")
            except Exception as e:
                write_log(f"[!] Lỗi không mong muốn: {e}")


if __name__ == '__main__':
    main()
