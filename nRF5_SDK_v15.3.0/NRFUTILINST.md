# nRF Utilインストール手順

Nordic社から無償公開されているツール「[nRF Util](https://infocenter.nordicsemi.com/topic/ug_nrfutil/UG/nrfutil/nrfutil_intro.html)」を導入する手順を記載します。

## 前提
macOSに初期導入されているPython3環境を利用します。<br>
また、事前にXcodeコマンドラインツールの導入が必要となります。

#### 動作確認環境
macOS Catalina (Version 10.15.5)<br>
Command Line Tools for Xcode 11.5

## インストール

以下のコマンドを実行します。

```
sudo pip3 install nrfutil
```

下記は実行例になります。

```
makmorit@iMac-makmorit-jp ~ % sudo pip3 install nrfutil
Password:
The directory '/Users/makmorit/Library/Caches/pip/http' or its parent directory is not owned by the current user and the cache has been disabled. Please check the permissions and owner of that directory. If executing pip with sudo, you may want sudo's -H flag.
The directory '/Users/makmorit/Library/Caches/pip' or its parent directory is not owned by the current user and caching wheels has been disabled. check the permissions and owner of that directory. If executing pip with sudo, you may want sudo's -H flag.
Collecting nrfutil
Collecting protobuf (from nrfutil)
  Downloading https://files.pythonhosted.org/packages/12/b5/9347e30e11c040f5ca24f079d6f06485280c49b2a9f894b5400e27d4d6d1/protobuf-3.12.2-cp37-cp37m-macosx_10_9_x86_64.whl (1.3MB)
    100% |████████████████████████████████| 1.3MB 1.3MB/s
Collecting click (from nrfutil)
  Downloading https://files.pythonhosted.org/packages/d2/3d/fa76db83bf75c4f8d338c2fd15c8d33fdd7ad23a9b5e57eb6c5de26b430e/click-7.1.2-py2.py3-none-any.whl (82kB)
    100% |████████████████████████████████| 92kB 1.8MB/s
Collecting pyserial (from nrfutil)
  Downloading https://files.pythonhosted.org/packages/0d/e4/2a744dd9e3be04a0c0907414e2a01a7c88bb3915cbe3c8cc06e209f59c30/pyserial-3.4-py2.py3-none-any.whl (193kB)
    100% |████████████████████████████████| 194kB 1.3MB/s
Collecting pyspinel>=1.0.0a3 (from nrfutil)
  Downloading https://files.pythonhosted.org/packages/80/e2/12156b8f2d0b6e0bcd169e6df078cbeb432312a5b2da8250b200975e4bac/pyspinel-1.0.1-py3-none-any.whl (69kB)
    100% |████████████████████████████████| 71kB 1.3MB/s
Collecting crcmod (from nrfutil)
Collecting libusb1 (from nrfutil)
Collecting pyyaml (from nrfutil)
Collecting tqdm (from nrfutil)
  Downloading https://files.pythonhosted.org/packages/f3/76/4697ce203a3d42b2ead61127b35e5fcc26bba9a35c03b32a2bd342a4c869/tqdm-4.46.1-py2.py3-none-any.whl (63kB)
    100% |████████████████████████████████| 71kB 1.6MB/s
Collecting intelhex (from nrfutil)
  Downloading https://files.pythonhosted.org/packages/bf/77/bf670318b3db325c71e2ac6a90b7bcfdf9fc739b7cf6aebb31715721623e/intelhex-2.2.1-py2.py3-none-any.whl (50kB)
    100% |████████████████████████████████| 51kB 1.6MB/s
Collecting pc-ble-driver-py>=0.14.2 (from nrfutil)
  Downloading https://files.pythonhosted.org/packages/a2/6e/0aa68cb67d4e0873bf997abcfa9059158dc1a8806fb0350afb093e07fc34/pc_ble_driver_py-0.14.2-cp37-cp37m-macosx_10_9_x86_64.whl (2.2MB)
    100% |████████████████████████████████| 2.2MB 831kB/s
Collecting piccata (from nrfutil)
  Downloading https://files.pythonhosted.org/packages/19/4d/711f14ad4fd6de8c54143b404ddf8c203a52e2501227b137c9bb0e0ec55b/piccata-2.0.1-py3-none-any.whl
Collecting ecdsa (from nrfutil)
  Downloading https://files.pythonhosted.org/packages/b8/11/4b4d30e4746584684c758d8f1ddc1fa5ab1470b6bf70bce4d9b235965e99/ecdsa-0.15-py2.py3-none-any.whl (100kB)
    100% |████████████████████████████████| 102kB 1.6MB/s
Requirement already satisfied: setuptools in /Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/3.7/lib/python3.7/site-packages (from protobuf->nrfutil) (40.8.0)
Requirement already satisfied: six>=1.9 in /Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/3.7/lib/python3.7/site-packages (from protobuf->nrfutil) (1.12.0)
Collecting ipaddress (from pyspinel>=1.0.0a3->nrfutil)
  Downloading https://files.pythonhosted.org/packages/c2/f8/49697181b1651d8347d24c095ce46c7346c37335ddc7d255833e7cde674d/ipaddress-1.0.23-py2.py3-none-any.whl
Collecting wrapt (from pc-ble-driver-py>=0.14.2->nrfutil)
Collecting future (from pc-ble-driver-py>=0.14.2->nrfutil)
Installing collected packages: protobuf, click, pyserial, ipaddress, pyspinel, crcmod, libusb1, pyyaml, tqdm, intelhex, wrapt, future, pc-ble-driver-py, piccata, ecdsa, nrfutil
Successfully installed click-7.1.2 crcmod-1.7 ecdsa-0.15 future-0.18.2 intelhex-2.2.1 ipaddress-1.0.23 libusb1-1.8 nrfutil-6.1.0 pc-ble-driver-py-0.14.2 piccata-2.0.1 protobuf-3.12.2 pyserial-3.4 pyspinel-1.0.1 pyyaml-5.3.1 tqdm-4.46.1 wrapt-1.12.1
You are using pip version 19.0.3, however version 20.2b1 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
makmorit@iMac-makmorit-jp ~ % echo $?
0
makmorit@iMac-makmorit-jp ~ %
```

以上で、nRF Utilのインストールは完了です。
