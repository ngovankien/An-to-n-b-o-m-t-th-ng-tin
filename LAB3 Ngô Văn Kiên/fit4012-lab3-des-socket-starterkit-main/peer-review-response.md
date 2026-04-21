# Peer Review Response

## Thông tin nhóm
- Thành viên 1: [Điền tên thành viên 1]
- Thành viên 2: [Điền tên thành viên 2]

## Thành viên 1 góp ý cho thành viên 2
Phần receiver đã chạy đúng happy path nhưng ban đầu còn thiếu xử lý timeout và lỗi giao thức. Sau góp ý, nhóm đã bổ sung bắt `TimeoutError`, `ConnectionError`, `ValueError` và chuẩn hoá log để receiver không thoát bằng traceback khó đọc.

## Thành viên 2 góp ý cho thành viên 1
Phần sender ban đầu gửi được dữ liệu nhưng log chưa nhất quán và chưa thuận tiện cho việc lưu minh chứng. Sau góp ý, nhóm đã thêm hàm ghi log chung, chuẩn hoá định dạng output và giữ cơ chế nhập từ bàn phím hoặc biến môi trường để tiện demo và test tự động.

## Nhóm đã sửa gì sau góp ý
Nhóm đã cập nhật `receiver.py` để xử lý các tình huống timeout, header sai, thiếu dữ liệu và padding lỗi theo hướng rõ ràng hơn. Nhóm cũng mở rộng bộ test cho header sai, truncated ciphertext, bad padding và closed connection thay vì chỉ dựa vào happy path. Ngoài ra README, report và threat model đã được viết lại đầy đủ hơn để bám sát rubric của lab.
