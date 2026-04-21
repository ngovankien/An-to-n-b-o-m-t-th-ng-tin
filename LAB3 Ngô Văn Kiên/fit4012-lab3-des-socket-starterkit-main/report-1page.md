# report-1page.md

## Thông tin nhóm
- Thành viên 1: [Ngô Văn Kiên]
- Thành viên 2: [Nguyễn Hoàng Sơn]

## Mục tiêu
Bài lab xây dựng hệ thống Sender/Receiver truyền dữ liệu mã hóa bằng DES-CBC qua TCP socket. Nhóm cần hiểu vai trò của Sender, Receiver, DES key, IV, header độ dài, ciphertext và PKCS#7 padding; đồng thời chạy đúng hệ thống, bổ sung kiểm thử lỗi, ghi log minh chứng, viết threat model và trình bày việc sử dụng trong phạm vi học tập an toàn. :contentReference[oaicite:1]{index=1}

## Phân công thực hiện
Thành viên 1 phụ trách phía gửi: đọc input, sinh key và IV, mã hóa DES-CBC, đóng gói packet và ghi log phía sender. Thành viên 2 phụ trách phía nhận: bind, listen, accept, nhận header và ciphertext, giải mã, kiểm tra lỗi và ghi log phía receiver. Cả hai cùng xây dựng test, chạy demo local, viết report, threat model và hoàn thiện README.

## Cách làm
Sender nhận bản tin từ bàn phím hoặc biến môi trường `MESSAGE`, sinh DES key và IV ngẫu nhiên, mã hóa bản rõ bằng DES-CBC, sau đó gửi packet theo thứ tự: key, IV, header 4 byte mô tả độ dài ciphertext, rồi ciphertext. Receiver mở cổng TCP, chờ kết nối, nhận lần lượt các thành phần trên, đọc đúng số byte cần thiết, giải mã và bỏ padding PKCS#7 để khôi phục bản rõ. Hệ thống được bổ sung timeout, bắt exception và log để xử lý các tình huống như sai header độ dài, thiếu ciphertext, padding không hợp lệ hoặc kết nối đóng đột ngột. :contentReference[oaicite:2]{index=2}

## Kết quả
Hệ thống chạy thành công trên localhost. Receiver hiển thị trạng thái đang lắng nghe, ghi nhận kết nối thành công và in lại đúng bản tin gốc sau giải mã. Nhóm đã bổ sung kiểm thử cho happy path và các lỗi giao thức quan trọng như header sai, ciphertext bị cắt cụt, bad padding và closed connection. Các ca lỗi cho thấy chương trình phát hiện bất thường và trả về log hoặc exception rõ ràng thay vì tiếp tục xử lý dữ liệu sai. Điều này phù hợp với yêu cầu phải có minh chứng chạy đúng và có kiểm thử lỗi cơ bản cho giao thức/socket/padding. :contentReference[oaicite:3]{index=3}

## Kết luận
Lab 3 giúp nhóm kết nối ba mảng kiến thức là lập trình mạng, mật mã và tư duy an toàn hệ thống. Thiết kế hiện tại phù hợp cho mục đích học tập vì cho phép quan sát rõ luồng key, IV, header và ciphertext, nhưng không an toàn để triển khai thực tế do DES đã yếu, key được truyền plaintext và chưa có cơ chế xác thực toàn vẹn. Qua bài lab, nhóm rút ra rằng một hệ thống truyền dữ liệu không chỉ cần chạy được mà còn phải xử lý lỗi tốt, có kiểm thử, có log minh chứng và được đánh giá rủi ro bảo mật trước khi sử dụng. :contentReference[oaicite:4]{index=4}