"""Regressões do pacote compartilhado pelo instalador local e pelo CI."""
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import sys

spec = importlib.util.spec_from_file_location('builder', Path(__file__).resolve().parents[1] / 'tools/build_web_installer.py')
builder = importlib.util.module_from_spec(spec)
spec.loader.exec_module(builder)


class PackageTests(unittest.TestCase):
    def test_package_matches_binary(self):
        with tempfile.TemporaryDirectory() as tmp:
            binary = Path(tmp) / 'firmware/PixelGotchi-esp32s3-test.bin'
            binary.parent.mkdir()
            data = b'\xe9' + bytes(65535)
            binary.write_bytes(data)
            info = builder.write_package(tmp, str(binary), 'test')
            manifest = json.loads((Path(tmp) / 'manifest.json').read_text(encoding='utf8'))
            self.assertEqual(manifest['builds'][0]['parts'], [{'path': info['path'], 'offset': 0}])
            self.assertEqual(manifest['builds'][0]['chipFamily'], 'ESP32-S3')
            self.assertEqual(info['sha256'], hashlib.sha256(data).hexdigest())
            self.assertEqual(info['size'], len(data))
            self.assertEqual(info, json.loads((Path(tmp) / 'firmware-info.json').read_text()))
            binary.write_bytes(b'invalid')
            with self.assertRaises(ValueError): builder.write_package(tmp, str(binary), 'test')

    def test_pip_runtime_fallback(self):
        with patch.object(builder.os.path, 'isfile', side_effect=lambda p: p.endswith('esptool.py')):
            self.assertEqual(builder.esptool_cmd()[0], sys.executable)


if __name__ == '__main__': unittest.main()
