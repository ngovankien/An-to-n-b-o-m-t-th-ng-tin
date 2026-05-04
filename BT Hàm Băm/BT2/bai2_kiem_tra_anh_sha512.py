import hashlib
import os

def tinh_sha512_file(duong_dan_file):
    """
    Hàm tính mã băm SHA-512 của file ảnh.
    """
    sha512 = hashlib.sha512()

    try:
        with open(duong_dan_file, "rb") as file:
            while True:
                du_lieu = file.read(4096)
                if not du_lieu:
                    break
                sha512.update(du_lieu)

        return sha512.hexdigest()

    except FileNotFoundError:
        print("Lỗi: Không tìm thấy file ảnh.")
        return None


def tao_anh_bi_sua(file_goc, file_moi):
    """
    Tạo một file ảnh mới nhìn giống ảnh gốc,
    nhưng có thêm dữ liệu ở cuối file nên mã băm sẽ khác.
    """
    try:
        with open(file_goc, "rb") as f:
            du_lieu = f.read()

        with open(file_moi, "wb") as f:
            f.write(du_lieu)
            f.write(b"Du lieu an them vao cuoi file anh")

        print(f"\nĐã tạo file ảnh bị sửa: {file_moi}")
        print("Ảnh này nhìn bằng mắt thường vẫn giống ảnh gốc.")
        print("Nhưng dữ liệu bên trong file đã thay đổi.")

    except FileNotFoundError:
        print("Lỗi: Không tìm thấy file ảnh gốc.")


def so_sanh_hash(file_goc, file_kiem_tra):
    """
    So sánh mã băm SHA-512 của hai file ảnh.
    """
    hash_goc = tinh_sha512_file(file_goc)
    hash_kiem_tra = tinh_sha512_file(file_kiem_tra)

    if hash_goc is None or hash_kiem_tra is None:
        return

    print("\n===== MÃ BĂM SHA-512 =====")
    print("File gốc:")
    print(hash_goc)

    print("\nFile kiểm tra:")
    print(hash_kiem_tra)

    print("\n===== KẾT QUẢ KIỂM TRA =====")
    if hash_goc == hash_kiem_tra:
        print("File ảnh còn nguyên vẹn, không bị thay đổi.")
    else:
        print("File ảnh đã bị thay đổi hoặc không còn nguyên vẹn.")


# Chương trình chính
print("CHƯƠNG TRÌNH KIỂM TRA TÍNH TOÀN VẸN FILE ẢNH BẰNG SHA-512")

file_goc = input("\nNhập tên hoặc đường dẫn file ảnh gốc: ")

if os.path.exists(file_goc):
    file_bi_sua = "anh_da_sua.jpg"

    print("\nĐang tính mã băm file ảnh gốc...")
    hash_ban_dau = tinh_sha512_file(file_goc)

    print("\nMã băm SHA-512 của file ảnh gốc:")
    print(hash_ban_dau)

    print("\nĐang tạo một file ảnh bị sửa để kiểm tra...")
    tao_anh_bi_sua(file_goc, file_bi_sua)

    print("\nSo sánh file ảnh gốc và file ảnh bị sửa:")
    so_sanh_hash(file_goc, file_bi_sua)
else:
    print("File không tồn tại. Vui lòng kiểm tra lại tên file hoặc đường dẫn.")