# BÀI TẬP: CHƯƠNG TRÌNH MÃ HÓA VÀ GIẢI MÃ FILE SỬ DỤNG RSA
## Ngô Văn Kiên
## Nguyễn Hoàng Sơn

## 1. Giới thiệu đề tài

Đề tài yêu cầu xây dựng một chương trình mã hóa và giải mã sử dụng thuật toán RSA với hai mục tiêu chính:

1. **Gửi file an toàn**: người gửi mã hóa file trước khi gửi, người nhận chỉ có thể giải mã nếu có đúng khóa riêng tư.
2. **Xác thực người gửi**: người nhận có thể kiểm tra file có đúng do người gửi ký hay không, đồng thời phát hiện file có bị chỉnh sửa trong quá trình truyền hay không.

Project này được xây dựng dưới dạng **trang web React** chạy trực tiếp trên trình duyệt. Giao diện hỗ trợ tạo khóa, chọn file, mã hóa, ký số, xác thực chữ ký và giải mã file.

---

## 2. Công nghệ sử dụng

- **React + Vite**: xây dựng giao diện web.
- **JavaScript Web Crypto API**: thực hiện mã hóa, giải mã, tạo khóa và ký số trong trình duyệt.
- **RSA-OAEP 4096-bit SHA-256**: mã hóa khóa bí mật dùng để bảo vệ file.
- **RSA-PSS 4096-bit SHA-256**: tạo và kiểm tra chữ ký số để xác thực người gửi.
- **AES-256-GCM**: mã hóa nội dung file.

---

## 3. Lý do kết hợp RSA và AES

RSA không phù hợp để mã hóa trực tiếp file lớn vì tốc độ chậm và giới hạn kích thước dữ liệu. Vì vậy chương trình sử dụng mô hình **mã hóa lai**:

- File được mã hóa bằng **AES-256-GCM** vì AES nhanh và phù hợp với dữ liệu lớn.
- Khóa AES được mã hóa bằng **RSA-OAEP** bằng public key của người nhận.
- Gói dữ liệu được ký bằng **RSA-PSS** bằng private key ký số của người gửi.

Cách này vừa đảm bảo tốc độ, vừa đảm bảo an toàn khi gửi file.

---

## 4. Chức năng chính

### 4.1. Tạo khóa RSA

Chương trình tạo 2 cặp khóa RSA:

- Cặp khóa mã hóa/giải mã:
  - **Public key nhận file**: dùng để mã hóa khóa AES.
  - **Private key giải mã**: dùng để giải mã khóa AES.

- Cặp khóa ký số/xác thực:
  - **Private key ký số**: người gửi dùng để ký gói dữ liệu.
  - **Public key xác thực người gửi**: người nhận dùng để kiểm tra chữ ký.

### 4.2. Mã hóa file an toàn

Khi người gửi chọn file và bấm mã hóa:

1. Chương trình tạo một khóa AES-256 ngẫu nhiên.
2. File gốc được mã hóa bằng AES-GCM.
3. Khóa AES được mã hóa bằng public key RSA-OAEP của người nhận.
4. Chương trình tạo chữ ký số RSA-PSS cho gói dữ liệu.
5. Kết quả được tải xuống dưới dạng file `.rsa-safe.json`.

### 4.3. Xác thực và giải mã file

Khi người nhận nhận được file `.rsa-safe.json`:

1. Chọn file mã hóa.
2. Dán private key giải mã của người nhận.
3. Dán public key xác thực của người gửi.
4. Chương trình kiểm tra chữ ký số.
5. Nếu chữ ký hợp lệ, chương trình giải mã khóa AES bằng RSA.
6. Dùng khóa AES để giải mã file gốc và tải file xuống.

Nếu chữ ký không hợp lệ, chương trình sẽ báo lỗi và không giải mã file.

---

## 5. Cấu trúc project

```text
rsa-secure-file-web/
├── index.html
├── package.json
├── README.md
└── src/
    ├── main.jsx
    └── style.css
```

Ý nghĩa các file chính:

- `index.html`: file HTML gốc của ứng dụng.
- `package.json`: khai báo script chạy project và thư viện cần dùng.
- `src/main.jsx`: toàn bộ logic giao diện, tạo khóa, mã hóa, ký số, xác thực và giải mã.
- `src/style.css`: định dạng giao diện.

---

## 6. Yêu cầu cài đặt

Máy cần cài **Node.js bản LTS**.

Kiểm tra bằng lệnh:

```bash
node -v
npm -v
```

Nếu chưa có Node.js, tải và cài bản LTS tại trang chính thức của Node.js.

---

## 7. Cách chạy project

### Cách chạy trên Windows PowerShell

Giải nén file ZIP:

```powershell
Expand-Archive .\rsa-secure-file-web-compact-keys-cleaned.zip -DestinationPath . -Force
```

Vào thư mục project:

```powershell
cd .\rsa-secure-file-web
```

Cài thư viện:

```powershell
npm install
```

Chạy project:

```powershell
npm run dev
```

Mở trình duyệt tại địa chỉ:

```text
http://localhost:5173/
```

Lưu ý: nên mở bằng `localhost` hoặc `127.0.0.1`, không mở bằng địa chỉ IP mạng LAN như `172.x.x.x` hoặc `192.168.x.x`, vì Web Crypto API có thể bị trình duyệt chặn khi không phải môi trường an toàn.

---

## 8. Hướng dẫn sử dụng

### 8.1. Tạo khóa

1. Mở web.
2. Bấm nút **Tạo khóa**.
3. Chương trình sẽ tạo các khóa cần thiết.
4. Có thể bấm **Copy** hoặc **Tải** để lưu khóa.

### 8.2. Mã hóa và ký file

1. Chọn tab **Mã hóa & ký file**.
2. Nhập tên người gửi.
3. Chọn file cần gửi.
4. Dán **Public key RSA-OAEP của người nhận**.
5. Dán **Private key RSA-PSS của người gửi để ký số**.
6. Bấm **Mã hóa, ký số và tải gói bảo mật**.
7. Chương trình tải xuống file có đuôi `.rsa-safe.json`.

File này là file đã được mã hóa và có chữ ký số.

### 8.3. Xác thực và giải mã file

1. Chọn tab **Xác thực & giải mã**.
2. Chọn file `.rsa-safe.json` đã nhận.
3. Dán **Private key RSA-OAEP của người nhận để giải mã**.
4. Dán **Public key RSA-PSS của người gửi để xác thực chữ ký**.
5. Bấm **Kiểm tra chữ ký và giải mã file**.
6. Nếu chữ ký hợp lệ, file gốc sẽ được tải xuống.

---

## 9. Mô tả thuật toán

### 9.1. Quá trình mã hóa

```text
Input: File gốc, public key người nhận, private signing key người gửi

Bước 1: Sinh khóa AES ngẫu nhiên.
Bước 2: Mã hóa file gốc bằng AES-GCM.
Bước 3: Mã hóa khóa AES bằng RSA-OAEP public key của người nhận.
Bước 4: Ghép dữ liệu gồm khóa AES đã mã hóa, IV, file đã mã hóa và tên file.
Bước 5: Ký dữ liệu bằng RSA-PSS private key của người gửi.
Bước 6: Xuất gói JSON chứa dữ liệu mã hóa và chữ ký.
```

### 9.2. Quá trình giải mã

```text
Input: File .rsa-safe.json, private key người nhận, public signing key người gửi

Bước 1: Đọc gói JSON.
Bước 2: Kiểm tra chữ ký bằng RSA-PSS public key của người gửi.
Bước 3: Nếu chữ ký sai, dừng chương trình và báo lỗi.
Bước 4: Nếu chữ ký đúng, giải mã khóa AES bằng RSA-OAEP private key của người nhận.
Bước 5: Dùng khóa AES để giải mã file gốc.
Bước 6: Tải file gốc xuống máy người dùng.
```

---

## 10. Định dạng file mã hóa

File kết quả có dạng JSON, ví dụ:

```json
{
  "version": "RSA-SAFE-FILE-v1",
  "algorithm": {
    "fileEncryption": "AES-256-GCM",
    "keyWrapping": "RSA-OAEP-4096-SHA256",
    "authentication": "RSA-PSS-4096-SHA256"
  },
  "senderName": "Nguyen Van A",
  "originalFileName": "document.pdf",
  "originalFileType": "application/pdf",
  "encryptedAesKey": "...",
  "iv": "...",
  "encryptedFile": "...",
  "signature": "...",
  "createdAt": "2026-05-11T00:00:00.000Z"
}
```

---

## 11. Kết quả đạt được

Project đã hoàn thành các yêu cầu:

- Có giao diện web để thao tác dễ dàng.
- Tạo được khóa RSA.
- Mã hóa được file để gửi an toàn.
- Giải mã được file khi có đúng private key.
- Ký số được gói dữ liệu.
- Xác thực được người gửi bằng public key.
- Phát hiện được trường hợp file bị chỉnh sửa hoặc chữ ký không hợp lệ.

---

## 12. Lưu ý bảo mật

Đây là project phục vụ học tập và demo. Khi triển khai thực tế cần lưu ý:

- Không gửi private key cho người khác.
- Không lưu private key dạng text thô trên máy công cộng.
- Nên bảo vệ private key bằng mật khẩu hoặc keystore.
- Public key cần được phân phối qua kênh đáng tin cậy để tránh giả mạo.
- Với hệ thống lớn, nên có backend hoặc hệ thống quản lý khóa chuyên dụng.

---

## 13. Kết luận

Chương trình đã minh họa đầy đủ cách sử dụng RSA trong bài toán gửi file an toàn và xác thực người gửi. Do RSA không phù hợp để mã hóa trực tiếp file lớn, chương trình sử dụng mô hình mã hóa lai RSA + AES. Bên cạnh đó, chữ ký số RSA-PSS giúp người nhận xác minh nguồn gốc file và kiểm tra tính toàn vẹn của dữ liệu.
