import pytest
from des_socket_utils import recv_exact


class DummyConn:
    def __init__(self, chunks):
        self.chunks = list(chunks)

    def recv(self, _n):
        if self.chunks:
            return self.chunks.pop(0)
        return b''


def test_closed_connection_before_enough_data():
    conn = DummyConn([b'1234'])
    with pytest.raises(ConnectionError):
        recv_exact(conn, 10)
