import hashlib

def bam_sha256(du_lieu):
    """
    Hàm băm dữ liệu bằng thuật toán SHA-256.
    """
    du_lieu_byte = du_lieu.encode("utf-8")
    ma_bam = hashlib.sha256(du_lieu_byte).hexdigest()
    return ma_bam


def bam_sha512(du_lieu):
    """
    Hàm băm dữ liệu bằng thuật toán SHA-512.
    """
    du_lieu_byte = du_lieu.encode("utf-8")
    ma_bam = hashlib.sha512(du_lieu_byte).hexdigest()
    return ma_bam


def kiem_tra_du_lieu(du_lieu_goc, du_lieu_kiem_tra):
    """
    Hàm kiểm tra dữ liệu có bị thay đổi hay không.
    """
    sha256_goc = bam_sha256(du_lieu_goc)
    sha512_goc = bam_sha512(du_lieu_goc)

    sha256_kiem_tra = bam_sha256(du_lieu_kiem_tra)
    sha512_kiem_tra = bam_sha512(du_lieu_kiem_tra)

    print("\n===== MÃ BĂM DỮ LIỆU BAN ĐẦU =====")
    print("SHA-256:", sha256_goc)
    print("SHA-512:", sha512_goc)

    print("\n===== MÃ BĂM DỮ LIỆU KIỂM TRA =====")
    print("SHA-256:", sha256_kiem_tra)
    print("SHA-512:", sha512_kiem_tra)

    if sha256_goc == sha256_kiem_tra and sha512_goc == sha512_kiem_tra:
        print("\nKẾT QUẢ: Dữ liệu không bị sửa đổi.")
    else:
        print("\nKẾT QUẢ: Dữ liệu đã bị sửa đổi.")


# Chương trình chính
print("CHƯƠNG TRÌNH BĂM DỮ LIỆU BẰNG SHA-256 VÀ SHA-512")

du_lieu_ban_dau = input("\nNhập dữ liệu ban đầu: ")

print("\nĐang tạo mã băm cho dữ liệu ban đầu...")
print("SHA-256:", bam_sha256(du_lieu_ban_dau))
print("SHA-512:", bam_sha512(du_lieu_ban_dau))

du_lieu_kiem_tra = input("\nNhập lại dữ liệu để kiểm tra: ")

kiem_tra_du_lieu(du_lieu_ban_dau, du_lieu_kiem_tra)