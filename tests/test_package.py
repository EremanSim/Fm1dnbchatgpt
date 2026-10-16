import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
import fm1_install as ota
assert ota.known_loader("ota-FM-1_9174010")
assert ota.known_loader("ota-FM-1_990")
assert not ota.known_loader("ota-FM-1_800")
assert not ota.known_loader("ota-FM-1_9174")
if len(sys.argv)>1:
    product,image=ota.load_package(sys.argv[1],False)
    assert product=="FM-1_9174010"
    assert image
    print("package: identity, loader marker, structure and CRC checks passed")
else:
    print("installer: loader allowlist checks passed")
