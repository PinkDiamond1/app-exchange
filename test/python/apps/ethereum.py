from ragger.utils import pack_APDU, RAPDU
from ragger.error import ExceptionRAPDU


class Command:
    GET_PUBLIC_KEY = 0x02
    SIGN = 0x04
    GET_APP_CONFIGURATION = 0x06
    SIGN_PERSONAL_MESSAGE = 0x08
    PROVIDE_ERC20_TOKEN_INFORMATION = 0x0A
    SIGN_EIP_712_MESSAGE = 0x0C
    GET_ETH2_PUBLIC_KEY = 0x0E
    SET_ETH2_WITHDRAWAL_INDEX = 0x10
    SET_EXTERNAL_PLUGIN = 0x12
    PROVIDE_NFT_INFORMATION = 0x14
    SET_PLUGIN = 0x16
    PERFORM_PRIVACY_OPERATION = 0x18


class P1:
    NON_CONFIRM = 0x00
    FIRST = 0x00
    CONFIRM = 0x01
    MORE = 0x80


class P2:
    NO_CHAINCODE = 0x00
    CHAINCODE = 0x01


class TxType:
    MIN = 0x00
    EIP2930 = 0x01
    EIP1559 = 0x02
    LEGACY = 0xc0
    MAX =  0x7f


ETH_CONF = bytes([
    0x03, 0x45, 0x54, 0x48, 0x08, 0x45, 0x74, 0x68, 0x65, 0x72, 0x65, 0x75,
    0x6D, 0x05, 0x03, 0x45, 0x54, 0x48, 0x12
])

ETH_CONF_DER_SIGNATURE = bytes([
    0x30, 0x44, 0x02, 0x20, 0x65, 0xD7, 0x93, 0x1A, 0xB3, 0x14, 0x43, 0x62,
    0xD5, 0x7E, 0x3F, 0xDC, 0xC5, 0xDE, 0x92, 0x1F, 0xB6, 0x50, 0x24, 0x73,
    0x7D, 0x91, 0x7F, 0x0A, 0xB1, 0xF8, 0xB1, 0x73, 0xD1, 0xED, 0x3C, 0x2E,
    0x02, 0x20, 0x27, 0x49, 0x35, 0x68, 0xD1, 0x12, 0xDC, 0x53, 0xC7, 0x17,
    0x7F, 0x8E, 0x5F, 0xC9, 0x15, 0xD9, 0x1A, 0x90, 0x37, 0x80, 0xA0, 0x67,
    0xBA, 0xDF, 0x10, 0x90, 0x85, 0xA7, 0x3D, 0x36, 0x03, 0x23
])

# length (5) + 44'/60'/0'/0/0
ETH_PACKED_DERIVATION_PATH = bytes([0x05,
                                    0x80, 0x00, 0x00, 0x2c,
                                    0x80, 0x00, 0x00, 0x3c,
                                    0x80, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00])

ERR_SILENT_MODE_CHECK_FAILED = ExceptionRAPDU(0x6001, "ERR_SILENT_MODE_CHECK_FAILED")


class EthereumClient:
    CLA = 0xE0
    def __init__(self, client, derivation_path=b''):
        self._client = client
        self._derivation_path = derivation_path or ETH_PACKED_DERIVATION_PATH

    @property
    def client(self):
        return self._client

    @property
    def derivation_path(self):
        return self._derivation_path

    def _forge_signature_payload(self, additional_payload: bytes):
        return pack_APDU(self.CLA, Command.SIGN, data=(self.derivation_path + additional_payload))

    def _exchange(self,
                  ins: int,
                  p1: int = P1.NON_CONFIRM,
                  p2: int = P2.NO_CHAINCODE,
                  payload: bytes = b''):
        return self.client.exchange(self.CLA, ins=ins, p1=p1, p2=p2, data=payload)

    def get_public_key(self):
        return self._exchange(Command.GET_PUBLIC_KEY, payload=self.derivation_path)

    def sign(self, extra_payload: bytes = bytes.fromhex('eb')):
        # TODO: finish ETH signature with proper payload
        payload = self.derivation_path + extra_payload
        return self._exchange(Command.SIGN, payload=payload)
