# 1. wine
```bash
$ ./run-in-wine path/to/my.exe
```

# 2. GB stuff

docs [here](https://gbdk.org/docs/api/docs_getting_started.html)

## bgb
```bash
wget -q https://bgb.bircd.org/bgb.zip && mkdir bgb && unzip bgb.zip -d bgb && rm bgb.zip
```

## gbdk
```bash
wget -qO gbdk.tar.gz https://github.com/gbdk-2020/gbdk-2020/releases/download/4.5.0/gbdk-linux64.tar.gz && tar -xf gbdk.tar.gz && rm gbdk.tar.gz
```

## gbtd/gbmb
```bash
wget -qO gbtd_gbmb.zip https://github.com/gbdk-2020/GBTD_GBMB/releases/download/2.4.5/GBTD_GBMB_release.zip && unzip gbtd_gbmb.zip && rm gbtd_gbmb.zip
```

## build folder
```bash
# from host
cd 0_folder_name && ./build

# through docker
./build-folder 0_folder_name
```
