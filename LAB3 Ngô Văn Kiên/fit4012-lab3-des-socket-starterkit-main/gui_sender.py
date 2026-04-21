import os
import socket
import tkinter as tk
from tkinter import ttk, messagebox
from des_socket_utils import encrypt_des_cbc, build_packet

DEFAULT_SERVER_IP = os.getenv("SERVER_IP", "127.0.0.1")
DEFAULT_SERVER_PORT = int(os.getenv("SERVER_PORT", "6000"))


class SenderGUI:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("DES Socket Sender")
        self.root.geometry("560x440")

        frame = ttk.Frame(root, padding=10)
        frame.pack(fill="both", expand=True)

        ttk.Label(frame, text="Server IP:").pack(anchor="w")
        self.ip_var = tk.StringVar(value=DEFAULT_SERVER_IP)
        ttk.Entry(frame, textvariable=self.ip_var).pack(fill="x", pady=(0, 8))

        ttk.Label(frame, text="Server Port:").pack(anchor="w")
        self.port_var = tk.StringVar(value=str(DEFAULT_SERVER_PORT))
        ttk.Entry(frame, textvariable=self.port_var).pack(fill="x", pady=(0, 8))

        ttk.Label(frame, text="Bản tin cần gửi:").pack(anchor="w")
        self.message_text = tk.Text(frame, height=6)
        self.message_text.pack(fill="x", pady=(0, 8))

        ttk.Button(frame, text="Gửi bản tin", command=self.send_message).pack(anchor="w", pady=(0, 8))

        ttk.Label(frame, text="Log:").pack(anchor="w")
        self.log_text = tk.Text(frame, height=12, state="disabled")
        self.log_text.pack(fill="both", expand=True)

    def write_log(self, line: str) -> None:
        self.log_text.config(state="normal")
        self.log_text.insert("end", line + "\n")
        self.log_text.see("end")
        self.log_text.config(state="disabled")

    def send_message(self) -> None:
        server_ip = self.ip_var.get().strip()
        try:
            server_port = int(self.port_var.get().strip())
        except ValueError:
            messagebox.showerror("Lỗi", "Port phải là số nguyên.")
            return

        message = self.message_text.get("1.0", "end").strip()
        if not message:
            messagebox.showwarning("Cảnh báo", "Bạn chưa nhập bản tin.")
            return

        try:
            plain = message.encode("utf-8")
            key, iv, cipher_bytes = encrypt_des_cbc(plain)
            packet = build_packet(key, iv, cipher_bytes)

            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((server_ip, server_port))
                s.sendall(packet)

            self.write_log("[+] Đã gửi bản mã.")
            self.write_log(f"[+] Key: {key.hex()}")
            self.write_log(f"[+] IV: {iv.hex()}")
            self.write_log(f"[+] Ciphertext: {cipher_bytes.hex()}")
            messagebox.showinfo("Thành công", "Đã gửi bản tin.")
        except Exception as e:
            self.write_log(f"[!] Lỗi: {e}")
            messagebox.showerror("Lỗi gửi dữ liệu", str(e))


if __name__ == "__main__":
    root = tk.Tk()
    SenderGUI(root)
    root.mainloop()
