# ファームウェア更新機能

#### `pyocd`のインストール

```
(ncs) bash-3.2$ pip install pyocd
Collecting pyocd
  Downloading https://files.pythonhosted.org/packages/da/7a/f67482d46c954f6ecd22f0e45115c94ecb1992b7c3a3fedd2a89781062bf/pyocd-0.31.0-py3-none-any.whl (12.5MB)
    100% |████████████████████████████████| 12.5MB 2.7MB/s
Collecting intervaltree<4.0,>=3.0.2 (from pyocd)
  Downloading https://files.pythonhosted.org/packages/50/fb/396d568039d21344639db96d940d40eb62befe704ef849b27949ded5c3bb/intervaltree-3.1.0.tar.gz
Requirement already satisfied: pyyaml<6.0,>=5.1 in ./GitHub/onecard-fido/pyvenvs/ncs/lib/python3.7/site-packages (from pyocd) (5.4.1)
Collecting hidapi; platform_system != "Linux" (from pyocd)
  Downloading https://files.pythonhosted.org/packages/82/4a/f0634b2cea7ee5ed4c4dc1e9a86e49df9df8b7794723a7d728156e03993a/hidapi-0.10.1-cp37-cp37m-macosx_10_9_x86_64.whl (43kB)
    100% |████████████████████████████████| 51kB 4.7MB/s
Collecting pyusb<2.0,>=1.2.1 (from pyocd)
  Downloading https://files.pythonhosted.org/packages/15/a8/4982498b2ab44d1fcd5c49f07ea3795eab01601dc143b009d333fcace3b9/pyusb-1.2.1-py3-none-any.whl (58kB)
    100% |████████████████████████████████| 61kB 5.9MB/s
Collecting pyocd-pemicro>=1.0.0.post2 (from pyocd)
  Downloading https://files.pythonhosted.org/packages/e1/84/26abfc902ac2225d8a4478e190f4143973a6c9966949e54b8e711f078e36/pyocd_pemicro-1.0.6-py3-none-any.whl
Requirement already satisfied: pyelftools<1.0 in ./GitHub/onecard-fido/pyvenvs/ncs/lib/python3.7/site-packages (from pyocd) (0.27)
Collecting capstone<5.0,>=4.0 (from pyocd)
  Downloading https://files.pythonhosted.org/packages/f2/ae/21dbb3ccc30d5cc9e8cdd8febfbf5d16d93b8c10e595280d2aa4631a0d1f/capstone-4.0.2.tar.gz (2.0MB)
    100% |████████████████████████████████| 2.0MB 3.4MB/s
Collecting cmsis-pack-manager>=0.2.10 (from pyocd)
  Downloading https://files.pythonhosted.org/packages/d0/a4/79c0952953dfefa4e4527943151bdf014b00adb20df10e73e8f076b27144/cmsis_pack_manager-0.2.10-py2.py3-none-macosx_10_13_x86_64.whl (3.2MB)
    100% |████████████████████████████████| 3.2MB 21.7MB/s
Collecting prettytable<3.0,>=2.0 (from pyocd)
  Downloading https://files.pythonhosted.org/packages/26/1b/42b59a4038bc0442e3a0085bc0de385658131eef8a88946333f870559b09/prettytable-2.1.0-py3-none-any.whl
Collecting pylink-square<1.0,>=0.8.2 (from pyocd)
  Downloading https://files.pythonhosted.org/packages/fb/0f/ab60002217044bb48dc5b800f16ad8a5808e635358bd134e433196d929fe/pylink_square-0.10.0-py2.py3-none-any.whl (76kB)
    100% |████████████████████████████████| 81kB 4.1MB/s
Collecting naturalsort<2.0,>=1.5 (from pyocd)
  Downloading https://files.pythonhosted.org/packages/0c/84/ce1985c8c61d2ac21a4b3a5d586ed0794b855f925ecc47adca546f0c7022/naturalsort-1.5.1.tar.gz
Requirement already satisfied: intelhex<3.0,>=2.0 in ./GitHub/onecard-fido/pyvenvs/ncs/lib/python3.7/site-packages (from pyocd) (2.3.0)
Requirement already satisfied: six<2.0,>=1.15.0 in ./GitHub/onecard-fido/pyvenvs/ncs/lib/python3.7/site-packages (from pyocd) (1.15.0)
Requirement already satisfied: colorama<1.0 in ./GitHub/onecard-fido/pyvenvs/ncs/lib/python3.7/site-packages (from pyocd) (0.4.4)
Collecting sortedcontainers<3.0,>=2.0 (from intervaltree<4.0,>=3.0.2->pyocd)
  Downloading https://files.pythonhosted.org/packages/32/46/9cb0e58b2deb7f82b84065f37f3bffeb12413f947f9388e4cac22c4621ce/sortedcontainers-2.4.0-py2.py3-none-any.whl
Requirement already satisfied: setuptools>=19.0 in ./GitHub/onecard-fido/pyvenvs/ncs/lib/python3.7/site-packages (from hidapi; platform_system != "Linux"->pyocd) (40.8.0)
Collecting pypemicro==0.1.7 (from pyocd-pemicro>=1.0.0.post2->pyocd)
  Downloading https://files.pythonhosted.org/packages/d7/e4/3c0e16487212dc4f1bfb4b57245dccec35cb6a6914e09b08649d87d276c5/pypemicro-0.1.7-py3-none-any.whl (3.7MB)
    100% |████████████████████████████████| 3.7MB 2.3MB/s
Collecting milksnake>=0.1.2 (from cmsis-pack-manager>=0.2.10->pyocd)
  Downloading https://files.pythonhosted.org/packages/27/be/e10e73f857ac98ef43587fa8db37a3ef6de56e728037a7b9728de26711c7/milksnake-0.1.5-py2.py3-none-any.whl
Collecting appdirs>=1.4 (from cmsis-pack-manager>=0.2.10->pyocd)
  Using cached https://files.pythonhosted.org/packages/3b/00/2344469e2084fb287c2e0b57b72910309874c3245463acd6cf5e3db69324/appdirs-1.4.4-py2.py3-none-any.whl
Collecting importlib-metadata; python_version < "3.8" (from prettytable<3.0,>=2.0->pyocd)
  Downloading https://files.pythonhosted.org/packages/07/76/c4674c460f5ff4b5f7a962214e46295e20504dfde9fcba78fd728dfe2ac9/importlib_metadata-4.6.3-py3-none-any.whl
Collecting wcwidth (from prettytable<3.0,>=2.0->pyocd)
  Using cached https://files.pythonhosted.org/packages/59/7c/e39aca596badaf1b78e8f547c807b04dae603a433d3e7a7e04d67f2ef3e5/wcwidth-0.2.5-py2.py3-none-any.whl
Collecting psutil>=5.2.2 (from pylink-square<1.0,>=0.8.2->pyocd)
  Downloading https://files.pythonhosted.org/packages/fe/19/83ab423a7b69cafe4078dea751acdff7377e4b59c71e3718125ba3c341f9/psutil-5.8.0-cp37-cp37m-macosx_10_9_x86_64.whl (236kB)
    100% |████████████████████████████████| 245kB 4.9MB/s
Collecting future (from pylink-square<1.0,>=0.8.2->pyocd)
  Using cached https://files.pythonhosted.org/packages/45/0b/38b06fd9b92dc2b68d58b75f900e97884c45bedd2ff83203d933cf5851c9/future-0.18.2.tar.gz
Requirement already satisfied: cffi>=1.6.0 in ./GitHub/onecard-fido/pyvenvs/ncs/lib/python3.7/site-packages (from milksnake>=0.1.2->cmsis-pack-manager>=0.2.10->pyocd) (1.14.5)
Collecting typing-extensions>=3.6.4; python_version < "3.8" (from importlib-metadata; python_version < "3.8"->prettytable<3.0,>=2.0->pyocd)
  Using cached https://files.pythonhosted.org/packages/2e/35/6c4fff5ab443b57116cb1aad46421fb719bed2825664e8fe77d66d99bcbc/typing_extensions-3.10.0.0-py3-none-any.whl
Collecting zipp>=0.5 (from importlib-metadata; python_version < "3.8"->prettytable<3.0,>=2.0->pyocd)
  Downloading https://files.pythonhosted.org/packages/92/d9/89f433969fb8dc5b9cbdd4b4deb587720ec1aeb59a020cf15002b9593eef/zipp-3.5.0-py3-none-any.whl
Requirement already satisfied: pycparser in ./GitHub/onecard-fido/pyvenvs/ncs/lib/python3.7/site-packages (from cffi>=1.6.0->milksnake>=0.1.2->cmsis-pack-manager>=0.2.10->pyocd) (2.20)
Installing collected packages: sortedcontainers, intervaltree, hidapi, pyusb, pypemicro, pyocd-pemicro, capstone, milksnake, appdirs, cmsis-pack-manager, typing-extensions, zipp, importlib-metadata, wcwidth, prettytable, psutil, future, pylink-square, naturalsort, pyocd
  Running setup.py install for intervaltree ... done
  Running setup.py install for capstone ... done
  Running setup.py install for future ... done
  Running setup.py install for naturalsort ... done
Successfully installed appdirs-1.4.4 capstone-4.0.2 cmsis-pack-manager-0.2.10 future-0.18.2 hidapi-0.10.1 importlib-metadata-4.6.3 intervaltree-3.1.0 milksnake-0.1.5 naturalsort-1.5.1 prettytable-2.1.0 psutil-5.8.0 pylink-square-0.10.0 pyocd-0.31.0 pyocd-pemicro-1.0.6 pypemicro-0.1.7 pyusb-1.2.1 sortedcontainers-2.4.0 typing-extensions-3.10.0.0 wcwidth-0.2.5 zipp-3.5.0
You are using pip version 19.0.3, however version 21.2.2 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
(ncs) bash-3.2$
```

#### インストール後の動作確認

```
(ncs) bash-3.2$ pyocd list
  #   Probe                             Unique ID  
---------------------------------------------------
  0   Segger J-Link OB-K22-NordicSemi   960160943  
(ncs) bash-3.2$
```
