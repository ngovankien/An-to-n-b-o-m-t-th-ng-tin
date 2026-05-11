import os
import sys
import json
import shutil
import subprocess
import time
import uuid
from http.server import HTTPServer, SimpleHTTPRequestHandler
from urllib.parse import urlparse
import cgi

ROOT = os.path.dirname(os.path.abspath(__file__))
PUBLIC = os.path.join(ROOT, "public")
CPP = os.path.join(ROOT, "cpp", "secure_rsa_file.cpp")
WORK = os.path.join(ROOT, "workspace")
BIN = os.path.join(ROOT, "secure_rsa_file.exe" if os.name == "nt" else "secure_rsa_file")
os.makedirs(WORK, exist_ok=True)

last_files = {}

def json_bytes(obj, status=200):
    data = json.dumps(obj, ensure_ascii=False).encode("utf-8")
    return status, data, "application/json; charset=utf-8"

def tool_missing_message():
    return (
        "Máy chưa cài g++ hoặc chưa thêm g++ vào PATH.\n\n"
        "Cách sửa trên Windows:\n"
        "1) Cài MSYS2 từ https://www.msys2.org/\n"
        "2) Mở MSYS2 UCRT64 và chạy:\n"
        "   pacman -Syu\n"
        "   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-openssl\n"
        "3) Thêm vào PATH:\n"
        "   C:\\msys64\\ucrt64\\bin\n"
        "4) Mở PowerShell mới, kiểm tra:\n"
        "   g++ --version\n"
        "   openssl version\n"
    )

def compile_binary():
    if os.path.exists(BIN):
        return True, "Đã có file chạy C++."
    if shutil.which("g++") is None:
        return False, tool_missing_message()

    cmd = ["g++", "-std=c++17", CPP, "-o", BIN, "-lssl", "-lcrypto"]
    try:
        p = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, timeout=60)
    except FileNotFoundError:
        return False, tool_missing_message()
    except Exception as e:
        return False, f"Không biên dịch được C++: {e}"
    if p.returncode != 0:
        return False, "Biên dịch C++ lỗi:\n" + (p.stderr or p.stdout)
    return True, "Biên dịch C++ thành công."

def run_cpp(args, timeout=120):
    ok, msg = compile_binary()
    if not ok:
        return False, msg
    try:
        p = subprocess.run([BIN] + args, cwd=WORK, capture_output=True, text=True, timeout=timeout)
    except FileNotFoundError:
        return False, tool_missing_message()
    except Exception as e:
        return False, f"Lỗi chạy chương trình C++: {e}"
    out = (p.stdout or "") + (p.stderr or "")
    return p.returncode == 0, out.strip()

def read_text(path):
    with open(path, "r", encoding="utf-8", errors="ignore") as f:
        return f.read()

def save_upload(field, path):
    data = field.file.read()
    with open(path, "wb") as f:
        f.write(data)
    return path

def write_text(path, text):
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)

class Handler(SimpleHTTPRequestHandler):
    def translate_path(self, path):
        path = urlparse(path).path
        if path == "/":
            return os.path.join(PUBLIC, "index.html")
        return os.path.join(PUBLIC, path.lstrip("/"))

    def send_payload(self, status, data, ctype):
        self.send_response(status)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        parsed = urlparse(self.path)
        if parsed.path == "/api/check":
            ok, msg = compile_binary()
            status, data, ctype = json_bytes({"ok": ok, "message": msg})
            self.send_payload(status, data, ctype)
            return
        if parsed.path.startswith("/download/"):
            key = parsed.path.split("/download/", 1)[1]
            path = last_files.get(key)
            if not path or not os.path.exists(path):
                status, data, ctype = json_bytes({"ok": False, "message": "File không tồn tại hoặc chưa tạo."}, 404)
                self.send_payload(status, data, ctype)
                return
            with open(path, "rb") as f:
                data = f.read()
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Disposition", f"attachment; filename=\"{os.path.basename(path)}\"")
            self.send_header("Content-Length", str(len(data)))
            self.end_headers()
            self.wfile.write(data)
            return
        super().do_GET()

    def do_POST(self):
        parsed = urlparse(self.path)
        try:
            if parsed.path == "/api/genkey":
                length = int(self.headers.get("Content-Length", "0"))
                body = json.loads(self.rfile.read(length).decode("utf-8"))
                prefix = body.get("prefix", "receiver")
                if prefix not in ("receiver", "sender"):
                    raise ValueError("prefix không hợp lệ")
                ok, out = run_cpp(["gen", prefix])
                priv = os.path.join(WORK, f"{prefix}_private.pem")
                pub = os.path.join(WORK, f"{prefix}_public.pem")
                if ok:
                    last_files[f"{prefix}_private"] = priv
                    last_files[f"{prefix}_public"] = pub
                    resp = {"ok": True, "message": out, "privateKey": read_text(priv), "publicKey": read_text(pub),
                            "privateDownload": f"/download/{prefix}_private", "publicDownload": f"/download/{prefix}_public"}
                else:
                    resp = {"ok": False, "message": out}
                status, data, ctype = json_bytes(resp, 200 if ok else 500)
                self.send_payload(status, data, ctype)
                return

            if parsed.path == "/api/pack":
                form = cgi.FieldStorage(fp=self.rfile, headers=self.headers, environ={"REQUEST_METHOD": "POST"})
                job = str(int(time.time())) + "_" + uuid.uuid4().hex[:8]
                jobdir = os.path.join(WORK, "job_" + job)
                os.makedirs(jobdir, exist_ok=True)

                if "file" not in form:
                    raise ValueError("Chưa chọn file cần gửi")
                input_name = os.path.basename(form["file"].filename or "input.bin")
                input_path = os.path.join(jobdir, input_name)
                save_upload(form["file"], input_path)

                receiver_pub = os.path.join(jobdir, "receiver_public.pem")
                sender_priv = os.path.join(jobdir, "sender_private.pem")
                if "receiverPublicFile" in form and form["receiverPublicFile"].filename:
                    save_upload(form["receiverPublicFile"], receiver_pub)
                else:
                    txt = form.getfirst("receiverPublicText", "")
                    if not txt.strip():
                        default = os.path.join(WORK, "receiver_public.pem")
                        if os.path.exists(default): shutil.copy(default, receiver_pub)
                        else: raise ValueError("Thiếu receiver_public.pem")
                    else: write_text(receiver_pub, txt)

                if "senderPrivateFile" in form and form["senderPrivateFile"].filename:
                    save_upload(form["senderPrivateFile"], sender_priv)
                else:
                    txt = form.getfirst("senderPrivateText", "")
                    if not txt.strip():
                        default = os.path.join(WORK, "sender_private.pem")
                        if os.path.exists(default): shutil.copy(default, sender_priv)
                        else: raise ValueError("Thiếu sender_private.pem")
                    else: write_text(sender_priv, txt)

                outpkg = os.path.join(jobdir, "package.bt3")
                ok, out = run_cpp(["pack", input_path, receiver_pub, sender_priv, outpkg])
                if ok:
                    last_files["package"] = outpkg
                status, data, ctype = json_bytes({"ok": ok, "message": out, "download": "/download/package" if ok else None}, 200 if ok else 500)
                self.send_payload(status, data, ctype)
                return

            if parsed.path == "/api/unpack":
                form = cgi.FieldStorage(fp=self.rfile, headers=self.headers, environ={"REQUEST_METHOD": "POST"})
                job = str(int(time.time())) + "_" + uuid.uuid4().hex[:8]
                jobdir = os.path.join(WORK, "recv_" + job)
                os.makedirs(jobdir, exist_ok=True)

                if "packageFile" not in form:
                    raise ValueError("Chưa chọn file package.bt3")
                pkg = os.path.join(jobdir, "package.bt3")
                save_upload(form["packageFile"], pkg)

                receiver_priv = os.path.join(jobdir, "receiver_private.pem")
                sender_pub = os.path.join(jobdir, "sender_public.pem")
                if "receiverPrivateFile" in form and form["receiverPrivateFile"].filename:
                    save_upload(form["receiverPrivateFile"], receiver_priv)
                else:
                    txt = form.getfirst("receiverPrivateText", "")
                    if not txt.strip():
                        default = os.path.join(WORK, "receiver_private.pem")
                        if os.path.exists(default): shutil.copy(default, receiver_priv)
                        else: raise ValueError("Thiếu receiver_private.pem")
                    else: write_text(receiver_priv, txt)

                if "senderPublicFile" in form and form["senderPublicFile"].filename:
                    save_upload(form["senderPublicFile"], sender_pub)
                else:
                    txt = form.getfirst("senderPublicText", "")
                    if not txt.strip():
                        default = os.path.join(WORK, "sender_public.pem")
                        if os.path.exists(default): shutil.copy(default, sender_pub)
                        else: raise ValueError("Thiếu sender_public.pem")
                    else: write_text(sender_pub, txt)

                outdir = os.path.join(jobdir, "output")
                ok, out = run_cpp(["unpack", pkg, receiver_priv, sender_pub, outdir])
                download = None
                if ok:
                    files = [os.path.join(outdir, f) for f in os.listdir(outdir)]
                    if files:
                        last_files["decrypted"] = files[0]
                        download = "/download/decrypted"
                status, data, ctype = json_bytes({"ok": ok, "message": out, "download": download}, 200 if ok else 500)
                self.send_payload(status, data, ctype)
                return

            status, data, ctype = json_bytes({"ok": False, "message": "Endpoint không tồn tại"}, 404)
            self.send_payload(status, data, ctype)
        except Exception as e:
            status, data, ctype = json_bytes({"ok": False, "message": str(e)}, 500)
            self.send_payload(status, data, ctype)


def get_lan_ip():
    import socket
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    except Exception:
        return "127.0.0.1"
    finally:
        s.close()

if __name__ == "__main__":
    port = 8000
    print("BT3 RSA - Gửi file an toàn + xác thực người gửi")
    print(f"Mở trên máy này : http://localhost:{port}")
    print(f"Máy khác mở     : http://{get_lan_ip()}:{port}")
    print("Server lắng nghe 0.0.0.0 để các máy cùng LAN kết nối được.")
    HTTPServer(("0.0.0.0", port), Handler).serve_forever()
