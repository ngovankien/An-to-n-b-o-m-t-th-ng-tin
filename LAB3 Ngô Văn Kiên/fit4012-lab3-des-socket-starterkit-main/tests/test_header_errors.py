import struct
from des_socket_utils import encrypt_des_cbc, parse_header


def test_wrong_header_length_field_is_detected_by_value_check():
    key, iv, cipher_bytes = encrypt_des_cbc(b"header test")
    fake_length = len(cipher_bytes) + 9
    header = key + iv + struct.pack('!I', fake_length)
    _, _, length = parse_header(header)
    assert length != len(cipher_bytes)
