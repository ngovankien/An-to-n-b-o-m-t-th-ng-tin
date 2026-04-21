import os
import socket
import threading
import tkinter as tk
from tkinter import ttk, messagebox
from des_socket_utils import HEADER_SIZE, parse_header, recv_exact, decrypt_des_cbc

DEFAULT_HOST = os.getenv("RECEIVER_HOST", "0.0.0.0")
DEFAULT_PORT = int(os.getenv("RECEIVER_PORT", "6000"))
DEFAULT_TIMEOUT = float(os.getenv("SOCKET_TIMEOUT", "60"))


class ReceiverGUI:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("DES Socket Receiver")
        self.root.geometry("640x520")

        self.server_socket = None
        self.is_listening = False

        frame = ttk.Frame(root, padding=10)
        frame.pack(fill="both", expand=True)

        ttk.Label(frame, text="Host:").pack(anchor="w")
        self.host_var = tk.StringVar(value=DEFAULT_HOST)
        ttk.Entry(frame, textvariable=self.host_var).pack(fill="x", pady=(0, 8))

        ttk.Label(frame, text="Port:").pack(anchor="w")
        self.port_var = tk.StringVar(value=str(DEFAULT_PORT))
        ttk.Entry(frame, textvariable=self.port_var).pack(fill="x", pady=(0, 8))

        ttk.Label(frame, text="Timeout (giây):").pack(anchor="w")
        self.timeout_var = tk.StringVar(value=str(int(DEFAULT_TIMEOUT)))
        ttk.Entry(frame, textvariable=self.timeout_var).pack(fill="x", pady=(0, 8))

        btns = ttk.Frame(frame)
        btns.pack(fill="x", pady=(0, 8))
        ttk.Button(btns, text="Bắt đầu lắng nghe", command=self.start_server).pack(side="left")
        ttk.Button(btns, text="Dừng", command=self.stop_server).pack(side="left", padx=8)

        ttk.Label(frame, text="Bản tin nhận được:").pack(anchor="w")
        self.message_text = tk.Text(frame, height=6)
        self.message_text.pack(fill="x", pady=(0, 8))

        ttk.Label(frame, text="Log:").pack(anchor="w")
        self.log_text = tk.Text(frame, height=14, state="disabled")
        self.log_text.pack(fill="both", expand=True)

    def write_log(self, line: str) -> None:
        self.log_text.config(state="normal")
        self.log_text.insert("end", line + "\n")
        self.log_text.see("end")
        self.log_text.config(state="disabled")

    def set_message(self, msg: str) -> None:
        self.message_text.delete("1.0", "end")
        self.message_text.insert("1.0", msg)

    def start_server(self) -> None:
        if self.is_listening:
            messagebox.showinfo("Thông báo", "Receiver đang lắng nghe rồi.")
            return
        try:
            host = self.host_var.get().strip()
            port = int(self.port_var.get().strip())
            timeout = float(self.timeout_var.get().strip())
        except ValueError:
            messagebox.showerror("Lỗi", "Port/Timeout không hợp lệ.")
            return
        threading.Thread(target=self.run_server, args=(host, port, timeout), daemon=True).start()

    def run_server(self, host: str, port: int, timeout: float) -> None:
        try:
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_socket.bind((host, port))
            self.server_socket.listen(1)
            self.server_socket.settimeout(timeout)
            self.is_listening = True
            self.root.after(0, lambda: self.write_log(f"[+] Đang lắng nghe {host}:{port} ..."))

            conn, addr = self.server_socket.accept()
            with conn:
                conn.settimeout(timeout)
                self.root.after(0, lambda: self.write_log(f"[+] Kết nối từ {addr}"))
                header = recv_exact(conn, HEADER_SIZE)
                key, iv, length = parse_header(header)
                cipher_bytes = recv_exact(conn, length)
                plaintext = decrypt_des_cbc(key, iv, cipher_bytes)
                message = plaintext.decode("utf-8", errors="ignore")
                self.root.after(0, lambda: self.set_message(message))
                self.root.after(0, lambda: self.write_log(f"[+] Bản tin gốc: {message}"))
        except TimeoutError:
            self.root.after(0, lambda: self.write_log("[!] Timeout: không có kết nối hoặc nhận dữ liệu quá lâu."))
        except Exception as e:
            self.root.after(0, lambda: self.write_log(f"[!] Lỗi: {e}"))
        finally:
            self.is_listening = False
            if self.server_socket:
                try:
                    self.server_socket.close()
                except Exception:
                    pass
                self.server_socket = None

    def stop_server(self) -> None:
        if self.server_socket:
            try:
                self.server_socket.close()
                self.write_log("[+] Đã dừng receiver.")
            except Exception as e:
                self.write_log(f"[!] Lỗi khi dừng receiver: {e}")
        self.is_listening = False


if __name__ == "__main__":
    root = tk.Tk()
    ReceiverGUI(root)
    root.mainloop()
