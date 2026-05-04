import hashlib
import shutil
import os


def tinh_sha512_file(duong_dan_file):
    """
    Hàm tính mã băm SHA-512 của một file.
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
        print("Lỗi: Không tìm thấy file.")
        return None


def gui_file(file_goc, file_nhan):
    """
    Mô phỏng quá trình gửi file bằng cách sao chép file gốc
    thành file người nhận.
    """
    try:
        shutil.copyfile(file_goc, file_nhan)
        print("\nĐã gửi file thành công.")
        print("File người nhận nhận được là:", file_nhan)

    except FileNotFoundError:
        print("Lỗi: Không tìm thấy file gốc.")


def lam_hong_file(file_nhan):
    """
    Mô phỏng việc file bị thay đổi trong quá trình truyền.
    Chương trình thêm dữ liệu vào cuối file.
    """
    try:
        with open(file_nhan, "ab") as file:
            file.write(b"\nDu lieu bi thay doi trong qua trinh truyen file")

        print("\nFile đã bị thay đổi trong quá trình truyền.")

    except FileNotFoundError:
        print("Lỗi: Không tìm thấy file nhận.")


def kiem_tra_toan_ven(hash_nguoi_gui, file_nhan):
    """
    Người nhận kiểm tra tính toàn vẹn của file.
    """
    hash_nguoi_nhan = tinh_sha512_file(file_nhan)

    if hash_nguoi_nhan is None:
        return

    print("\n===== MÃ BĂM SHA-512 =====")

    print("\nMã băm người gửi:")
    print(hash_nguoi_gui)

    print("\nMã băm người nhận:")
    print(hash_nguoi_nhan)

    print("\n===== KẾT QUẢ XÁC THỰC =====")

    if hash_nguoi_gui == hash_nguoi_nhan:
        print("File nhận được còn nguyên vẹn.")
        print("Quá trình gửi và nhận file thành công.")
    else:
        print("File nhận được đã bị thay đổi.")
        print("Quá trình gửi và nhận file không đảm bảo tính toàn vẹn.")


# Chương trình chính
print("CHƯƠNG TRÌNH MÔ PHỎNG GỬI VÀ NHẬN FILE")
print("XÁC THỰC TÍNH TOÀN VẸN FILE BẰNG SHA-512")

file_goc = input("\nNhập tên hoặc đường dẫn file cần gửi: ")

if os.path.exists(file_goc):
    file_nhan = "file_nguoi_nhan_nhan_duoc"

    # Lấy phần mở rộng của file gốc, ví dụ .jpg, .txt, .pdf
    ten_file, phan_mo_rong = os.path.splitext(file_goc)

    if phan_mo_rong != "":
        file_nhan = "file_nhan_duoc" + phan_mo_rong

    print("\nNgười gửi đang tính mã băm SHA-512 của file gốc...")
    hash_nguoi_gui = tinh_sha512_file(file_goc)

    print("\nMã băm SHA-512 của file gốc:")
    print(hash_nguoi_gui)

    print("\nĐang mô phỏng quá trình gửi file...")
    gui_file(file_goc, file_nhan)

    print("\nBạn muốn mô phỏng trường hợp nào?")
    print("1. File không bị thay đổi khi truyền")
    print("2. File bị thay đổi khi truyền")

    lua_chon = input("Nhập lựa chọn 1 hoặc 2: ")

    if lua_chon == "2":
        lam_hong_file(file_nhan)
    else:
        print("\nFile không bị thay đổi trong quá trình truyền.")

    print("\nNgười nhận đang kiểm tra tính toàn vẹn của file...")
    kiem_tra_toan_ven(hash_nguoi_gui, file_nhan)

else:
    print("File không tồn tại. Vui lòng kiểm tra lại tên file hoặc đường dẫn.")