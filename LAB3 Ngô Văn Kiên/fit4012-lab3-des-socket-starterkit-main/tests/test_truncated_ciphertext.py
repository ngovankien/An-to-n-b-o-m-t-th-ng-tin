import pytest
from des_socket_utils import encrypt_des_cbc, decrypt_des_cbc


def test_truncated_ciphertext_raises_value_error():
    key, iv, cipher_bytes = encrypt_des_cbc(b"truncated ciphertext test")
    with pytest.raises(ValueError):
        decrypt_des_cbc(key, iv, cipher_bytes[:-1])
