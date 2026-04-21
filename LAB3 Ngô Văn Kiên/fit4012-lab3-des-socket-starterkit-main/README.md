# FIT4012 - Lab 3 - Hệ thống gửi và nhận dữ liệu mã hoá DES qua Socket

Dự án này xây dựng một hệ thống nhỏ gồm **Sender** và **Receiver** giao tiếp với nhau bằng **TCP socket**. Sender sinh **DES key 8 byte** và **IV 8 byte**, mã hoá bản tin bằng **DES-CBC + PKCS#7**, sau đó gửi tuần tự **key + IV + header độ dài 4 byte + ciphertext**. Receiver lắng nghe kết nối, nhận đúng thứ tự các thành phần này, giải mã và in ra bản rõ.

Thiết kế này phù hợp cho mục đích học tập vì giúp quan sát rõ luồng dữ liệu, cấu trúc gói tin và các lỗi giao thức thường gặp. Nó **không an toàn để triển khai ngoài thực tế** vì DES đã lỗi thời, key được truyền dạng plaintext và chưa có cơ chế xác thực toàn vẹn.

## Team members
- **Thành viên 1**: [Điền tên thành viên 1] - MSSV: [Điền MSSV thành viên 1]
- **Thành viên 2**: [Điền tên thành viên 2] - MSSV: [Điền MSSV thành viên 2]

## Task division
- **Thành viên 1 phụ trách chính**: xây dựng `sender.py`, đóng gói packet, log phía gửi.
- **Thành viên 2 phụ trách chính**: xây dựng `receiver.py`, xử lý lỗi, log phía nhận.
- **Phần làm chung**: kiểm thử, hoàn thiện README, report, threat model và demo.

## Demo roles
- **Bạn demo Sender / gói tin / log gửi**: thành viên 1.
- **Bạn demo Receiver / giải mã / log nhận**: thành viên 2.
- **Cả hai cùng trả lời threat model và ethics**: cả hai thành viên.

## Mục tiêu học tập
- Hiểu vai trò của Sender, Receiver, TCP socket, DES key, IV, length header và ciphertext.
- Chạy và kiểm thử được mô hình truyền dữ liệu mã hoá DES-CBC qua socket.
- Quan sát được tác dụng của PKCS#7 padding trong mã hoá khối.
- Phân tích được một số rủi ro bảo mật của thiết kế hiện tại.

## Cấu trúc repo
- `sender.py`: phía gửi.
- `receiver.py`: phía nhận.
- `des_socket_utils.py`: pad/unpad, encrypt/decrypt, build/parse packet, nhận đủ dữ liệu.
- `tests/`: kiểm thử đơn vị và kiểm thử tích hợp local.
- `logs/`: log minh chứng chạy thành công và ca lỗi.
- `report-1page.md`: báo cáo ngắn.
- `threat-model-1page.md`: threat model 1 trang.
- `peer-review-response.md`: phản hồi review chéo.

## How to run
### 1. Cài môi trường
```bash
python -m venv .venv
. .venv/bin/activate
pip install -r requirements.txt
```

Trên Windows PowerShell:
```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
```

### 2. Chạy Receiver
```bash
python receiver.py
```

### 3. Chạy Sender
```bash
python sender.py
```
Sau đó nhập bản tin khi chương trình hỏi.

### 4. Demo local bằng biến môi trường
Terminal 1:
```bash
RECEIVER_HOST=127.0.0.1 RECEIVER_PORT=6001 SOCKET_TIMEOUT=30 python receiver.py
```

Terminal 2:
```bash
SERVER_IP=127.0.0.1 SERVER_PORT=6001 MESSAGE="Xin chao FIT4012" python sender.py
```

## Input / Output
### Input
- Sender nhận bản tin từ bàn phím hoặc từ biến môi trường `MESSAGE`.
- Receiver nhận packet qua TCP socket.

### Output
- Sender in ra trạng thái gửi thành công, `Key`, `IV`, `Ciphertext`.
- Receiver in ra trạng thái lắng nghe, thông tin kết nối, và bản tin gốc sau giải mã.
- Log có thể được ghi vào file bằng biến môi trường `SENDER_LOG_FILE` và `RECEIVER_LOG_FILE`.

## Các ca kiểm thử chính
- Happy path.
- Header độ dài sai.
- Ciphertext bị cắt cụt.
- Padding không hợp lệ.
- Kết nối bị đóng đột ngột.
- Tamper và wrong key để đáp ứng CI starter kit.

## Deliverables bắt buộc
- `README.md`
- `report-1page.md`
- `threat-model-1page.md`
- `peer-review-response.md`
- thư mục `tests/`
- thư mục `logs/`
- mã nguồn chạy được
- ít nhất 1 log minh chứng chạy thành công và ít nhất 2 log ca lỗi

## Threat-model awareness
Thiết kế hiện tại có một số điểm yếu bảo mật rõ ràng:
- DES là thuật toán cũ, độ dài khoá ngắn.
- DES key được gửi cùng kênh ở dạng plaintext nên có thể bị lộ nếu bị nghe lén.
- Chưa có cơ chế xác thực toàn vẹn nên ciphertext và trường length có thể bị sửa đổi.
- Nếu không đặt timeout và xử lý exception, receiver có thể treo khi nhận thiếu dữ liệu.

## Ethics & Safe use
- Chỉ thử nghiệm trên localhost, VM, hoặc mạng học tập được cho phép.
- Chỉ dùng dữ liệu giả lập, không dùng dữ liệu cá nhân thật hoặc dữ liệu nhạy cảm.
- Không quét cổng, không gửi dữ liệu tới máy lạ và không dùng bài này để tấn công hệ thống thật.
- Không trình bày hệ thống này như một giải pháp an toàn sẵn sàng triển khai ngoài đời.
- Nếu tham khảo thư viện hoặc mã nguồn bên ngoài, phải ghi nguồn rõ ràng.
- Tôn trọng trung thực học thuật khi làm việc nhóm.
