import pytest
from des_socket_utils import encrypt_des_cbc, decrypt_des_cbc


def test_bad_padding_raises_value_error():
    plain = b"padding test"
    key, iv, cipher_bytes = encrypt_des_cbc(plain, key=b"12345678", iv=b"abcdefgh")
    tampered = bytearray(cipher_bytes)
    tampered[-1] ^= 0x01

    with pytest.raises(ValueError):
        decrypt_des_cbc(key, iv, bytes(tampered))
