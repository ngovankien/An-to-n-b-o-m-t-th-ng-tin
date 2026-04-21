# threat-model-1page.md

## Thông tin nhóm
- Thành viên 1: [Ngô Văn Kiên]
- Thành viên 2: [Nguyễn Hoàng Sơn]

## Assets
- Plaintext do người dùng nhập
- DES key và IV
- Ciphertext truyền trên mạng
- Log hệ thống và file output
- Địa chỉ IP và cổng dịch vụ

## Attacker model
Kẻ tấn công được giả định có thể nghe lén lưu lượng trong cùng mạng LAN, sửa đổi dữ liệu trên đường truyền, gửi gói tin giả hoặc gói tin lỗi tới receiver, và đóng kết nối bất thường để gây gián đoạn. Kẻ tấn công không cần chiếm toàn quyền máy nạn nhân nhưng có khả năng quan sát hoặc can thiệp vào kênh truyền TCP.

## Threats
1. **Lộ khóa DES**: khóa được gửi cùng kênh dưới dạng plaintext nên người nghe lén có thể lấy khóa và giải mã toàn bộ dữ liệu.
2. **Sửa đổi ciphertext**: attacker có thể chỉnh sửa ciphertext gây lỗi giải mã hoặc làm sai lệch bản rõ được khôi phục.
3. **Giả mạo trường length**: attacker sửa header làm receiver đọc sai số byte cần nhận, từ đó gây lỗi hoặc làm chương trình chờ dữ liệu không đúng.
4. **Từ chối dịch vụ mức đơn giản**: attacker mở kết nối rồi ngắt giữa chừng hoặc gửi dữ liệu không đầy đủ khiến receiver timeout hay ghi log lỗi liên tục.

## Mitigations
- Không truyền khóa ở dạng plaintext; dùng cơ chế trao đổi khóa an toàn hơn
- Thay DES bằng AES hoặc thuật toán hiện đại hơn
- Thêm cơ chế xác thực toàn vẹn như HMAC hoặc dùng AEAD
- Kiểm tra chặt chẽ giá trị length và giới hạn kích thước packet
- Đặt timeout cho socket và xử lý exception rõ ràng để tránh treo vô thời hạn
- Hạn chế ghi log dữ liệu nhạy cảm ngoài môi trường học tập

## Residual risks
Ngay cả khi cải tiến, hệ thống vẫn còn một số rủi ro như máy người dùng bị compromise, log vô tình làm lộ thông tin nhạy cảm, hoặc môi trường mạng nội bộ vẫn có thể bị quan sát trái phép. Vì vậy, hệ thống này chỉ phù hợp cho mục đích học tập và minh họa trong phòng lab hoặc môi trường được phép, không phù hợp để triển khai thực tế.